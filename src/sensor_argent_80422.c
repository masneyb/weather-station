/*
 * sensor_argent_80422.c - Supports the wind direction, wind speed,
 *                         and rain gauge.
 *
 * Copyright (C) 2016 Brian Masney <masneyb@onstation.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <wiringPi.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "yadl.h"

static volatile unsigned int _wind_last_millis = 0;
static volatile int _wind_current_counter = 0;
static int _wind_last_counter = 0;

static volatile unsigned int _rain_last_millis = 0;
static volatile int _rain_current_counter = 0;
static int _rain_last_counter = 0;

static struct timeval _last_time;

static void _wind_speed_handler(void)
{
	unsigned int cur_millis = millis();
	if (cur_millis - _wind_last_millis > 10) {
		_wind_current_counter++;
		_wind_last_millis = cur_millis;
	}
}

static void _rain_gauge_handler(void)
{
	unsigned int cur_millis = millis();
	if (cur_millis - _rain_last_millis > 10) {
	        _rain_current_counter++;
		_rain_last_millis = cur_millis;
	}
}

static int _get_num_seen(int start_counter, int stop_counter)
{
	/* Check to see if the number wrapped */
	if (stop_counter < start_counter)
		return (INT_MAX - start_counter) + stop_counter;
	return stop_counter - start_counter;
}

static void _check_wind_direction(int read_millivolts, int compare_millivolts,
				float reading, float *direction, int *distance)
{
	int this_distance = abs(read_millivolts - compare_millivolts);
	if (*direction < 0 || this_distance < *distance) {
		*direction = reading;
		*distance = this_distance;
	}
}

/* These values came from the data sheet for the Sparkfun Weather
 * Station
 * https://www.sparkfun.com/products/8942
 * http://www.sparkfun.com/datasheets/Sensors/Weather/Weather%20Sensor%20Assembly..pdf
 */
static float _get_wind_direction(yadl_config *config)
{
	float direction = -1.0;
	int distance = 0;

	int reading = config->adc->adc_read(config);
	int millivolts = (float) reading * ((float) config->adc_millivolts / (float) config->adc->adc_resolution);

	/* Find the closest reading to the values listed in the datasheet */
	_check_wind_direction(millivolts, 3840, 0.0, &direction, &distance);
	_check_wind_direction(millivolts, 1980, 22.5, &direction, &distance);
	_check_wind_direction(millivolts, 2250, 45.0, &direction, &distance);
	_check_wind_direction(millivolts, 410, 67.5, &direction, &distance);
	_check_wind_direction(millivolts, 450, 90.0, &direction, &distance);
	_check_wind_direction(millivolts, 320, 112.5, &direction, &distance);
	_check_wind_direction(millivolts, 900, 135.0, &direction, &distance);
	_check_wind_direction(millivolts, 620, 157.5, &direction, &distance);
	_check_wind_direction(millivolts, 1400, 180.0, &direction, &distance);
	_check_wind_direction(millivolts, 1190, 202.5, &direction, &distance);
	_check_wind_direction(millivolts, 3080, 225.0, &direction, &distance);
	_check_wind_direction(millivolts, 2930, 247.5, &direction, &distance);
	_check_wind_direction(millivolts, 4620, 270.0, &direction, &distance);
	_check_wind_direction(millivolts, 4040, 292.5, &direction, &distance);
	_check_wind_direction(millivolts, 4780, 315.0, &direction, &distance);
	_check_wind_direction(millivolts, 3430, 337.5, &direction, &distance);

	config->logger("Wind direction: analog reading=%d; %d millivolts; direction (degrees)=%.1f\n",
			reading, millivolts, direction);

	return direction;
}

static pthread_mutex_t _wind_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *_argent_80422_wind_thread(void *arg)
{
	yadl_config *config = (yadl_config *) arg;

	int start_counter = _wind_current_counter;
	while(1) {
		sleep(1);

		int stop_counter = _wind_current_counter;
		int wind_num_seen = _get_num_seen(start_counter, stop_counter);
		float wind_speed = wind_num_seen * config->wind_speed_multiplier;

		float wind_direction = _get_wind_direction(config);

		config->logger("Wind thread: wind_direction=%.1f, wind_speed=%.1f mph\n",
				wind_direction, wind_speed);

		pthread_mutex_lock(&_wind_mutex);

		config->wind_directions_2m[config->wind_2m_idx] = wind_direction;
		config->wind_speeds_2m[config->wind_2m_idx] = wind_speed;
		config->wind_2m_idx = (config->wind_2m_idx + 1) % NUM_WIND_2_MIN_SAMPLES;

		config->wind_directions_10m[config->wind_10m_idx] = wind_direction;
		config->wind_speeds_10m[config->wind_10m_idx] = wind_speed;
		config->wind_10m_idx = (config->wind_10m_idx + 1) % NUM_WIND_10_MIN_SAMPLES;

		config->wind_directions_60m[config->wind_60m_idx] = wind_direction;
		config->wind_speeds_60m[config->wind_60m_idx] = wind_speed;
		config->wind_60m_idx = (config->wind_60m_idx + 1) % NUM_WIND_60_MIN_SAMPLES;

		pthread_mutex_unlock(&_wind_mutex);

		start_counter = stop_counter;
	}

	return NULL;
}

static void _create_wind_thread(yadl_config *config)
{
	pthread_t tid;

	for (int i = 0; i < NUM_WIND_2_MIN_SAMPLES; i++) {
		config->wind_directions_2m[i] = -1;
		config->wind_speeds_2m[i] = -1;
	}
	for (int i = 0; i < NUM_WIND_10_MIN_SAMPLES; i++) {
		config->wind_directions_10m[i] = -1;
		config->wind_speeds_10m[i] = -1;
	}
	for (int i = 0; i < NUM_WIND_60_MIN_SAMPLES; i++) {
		config->wind_directions_60m[i] = -1;
		config->wind_speeds_60m[i] = -1;
	}

	int ret = pthread_create(&tid, NULL, _argent_80422_wind_thread, config);
	if (ret < 0) {
		printf("Error creating wind thread: %s\n", strerror(errno));
		exit(1);
	}
}

static void _argent_80422_init(yadl_config *config)
{
	if (config->wind_speed_pin == -1) {
		fprintf(stderr, "You must specify the --wind_speed_pin argument\n");
		usage();
	}
	else if (config->rain_gauge_pin == -1) {
		fprintf(stderr, "You must specify the --rain_gauge_pin argument\n");
		usage();
	}

	if (config->wind_speed_unit == NULL) {
		fprintf(stderr, "You must specify the --wind_speed_unit argument\n");
		usage();
	}
	else if (strcmp(config->wind_speed_unit, "mph") == 0)
		config->wind_speed_multiplier = 1.492;
	else if (strcmp(config->wind_speed_unit, "kmh") == 0)
		config->wind_speed_multiplier = 2.4;
	else {
		fprintf(stderr, "Invalid --wind_speed_unit %s. Must be either mph or kmh.\n", config->wind_speed_unit);
		usage();
	}

	if (config->rain_gauge_unit == NULL) {
		fprintf(stderr, "You must specify the --rain_gauge_unit argument\n");
		usage();
	}
	else if (strcmp(config->rain_gauge_unit, "in") == 0)
		config->rain_gauge_multiplier = 0.011;
	else if (strcmp(config->rain_gauge_unit, "mm") == 0)
		config->rain_gauge_multiplier = 0.2794;
	else {
		fprintf(stderr, "Invalid --rain_gauge_unit %s. Must be either in or mm.\n", config->rain_gauge_unit);
		usage();
	}

	/* Initialize the ADC for the wind direction */
	if (config->adc == NULL) {
		fprintf(stderr, "You must specify the --adc argument\n");
		usage();
	}
	config->adc->adc_init(config);

	config->logger("wind_speed_pin=%d, rain_gauge_pin=%d\n",
			config->wind_speed_pin, config->rain_gauge_pin);

	wiringPiISR(config->wind_speed_pin, INT_EDGE_RISING, &_wind_speed_handler);
	wiringPiISR(config->rain_gauge_pin, INT_EDGE_RISING, &_rain_gauge_handler);

        gettimeofday(&_last_time, NULL);

	_create_wind_thread(config);

	/* Wait for the first sample */
	if (config->sleep_millis_between_results > 0)
		delay(config->sleep_millis_between_results);
}

static void _argent_80422_rain_gauge_total(float_node **list, int *num_samples, int interval_millis, float rain_gauge_cur, yadl_config *config)
{
	if (*num_samples == 0) {
		*list = new_list_node(rain_gauge_cur);
		*num_samples = 1;
		return;
	}
	else if (interval_millis < config->sleep_millis_between_results) {
		(*list)->value = rain_gauge_cur;
		return;
	}

	/* Add the new sample to the end of the list */
	float_node *last_list_node = list_last_node(*list);
	last_list_node->next = new_list_node(rain_gauge_cur);

	/* Remove the first sample if needed */
	int num_samples_to_keep = interval_millis / config->sleep_millis_between_results;
	if (*num_samples == num_samples_to_keep) {
		float_node *del_node = *list;
		*list = (*list)->next;
		free(del_node);
	}
	else {
		(*num_samples)++;
	}
	config->logger("rain gauge list: num_samples=%d, interval_millis=%d, num_samples_to_keep=%d\n",
			*num_samples, interval_millis, num_samples_to_keep);
}

static float _get_average_values(float values[], int total)
{
	float sum = 0.0;
	int num_items = 0;

	for (int i = 0; i < total; i++) {
		if (values[i] < 0)
			continue;

		sum += values[i];
		num_items++;
	}

	return sum / num_items;
}

static int _get_wind_gust_index(float values[], int total)
{
	int max_gust_idx = 0;

	for (int i = 0; i < total; i++) {
		if (values[i] > values[max_gust_idx])
			max_gust_idx = i;
	}

	return max_gust_idx;
}

static yadl_result *_argent_80422_read_data(yadl_config *config)
{
	float wind_direction = _get_wind_direction(config);

	/* Poll wind speed and rain gauge */
	int start_wind_counter = _wind_last_counter;
	int stop_wind_counter = _wind_current_counter;
	_wind_last_counter = stop_wind_counter;

	int start_rain_counter = _rain_last_counter;
	int stop_rain_counter = _rain_current_counter;
	_rain_last_counter = stop_rain_counter;

        static struct timeval _current_time;
        gettimeofday(&_current_time, NULL);
        double elapsed_secs = ((_current_time.tv_sec - _last_time.tv_sec) * 1000000L + _current_time.tv_usec - _last_time.tv_usec) / 1000000L;
        _last_time.tv_sec = _current_time.tv_sec;
        _last_time.tv_usec = _current_time.tv_usec;

	int wind_num_seen = _get_num_seen(start_wind_counter, stop_wind_counter);
	float avg_wind_cps = wind_num_seen / elapsed_secs;
	float wind_speed = avg_wind_cps * config->wind_speed_multiplier;

	int rain_num_seen = _get_num_seen(start_rain_counter, stop_rain_counter);
	float rain_gauge = rain_num_seen * config->rain_gauge_multiplier;

	_argent_80422_rain_gauge_total(&config->rain_gauge_1h, &config->num_rain_gauge_1h_samples,
					3600000, rain_gauge, config);
	_argent_80422_rain_gauge_total(&config->rain_gauge_6h, &config->num_rain_gauge_6h_samples,
					21600000, rain_gauge, config);
	_argent_80422_rain_gauge_total(&config->rain_gauge_24h, &config->num_rain_gauge_24h_samples,
					86400000, rain_gauge, config);

	yadl_result *result = malloc(sizeof(*result));

	result->value = malloc(sizeof(float) * 18);

	result->value[0] = wind_direction;
	result->value[1] = wind_speed;

	pthread_mutex_lock(&_wind_mutex);

	result->value[2] = _get_average_values(config->wind_directions_2m, NUM_WIND_2_MIN_SAMPLES);
	result->value[3] = _get_average_values(config->wind_speeds_2m, NUM_WIND_2_MIN_SAMPLES);

	int wind_gust_2m_idx = _get_wind_gust_index(config->wind_speeds_2m, NUM_WIND_2_MIN_SAMPLES);
	result->value[4] = config->wind_directions_2m[wind_gust_2m_idx];
	result->value[5] = config->wind_speeds_2m[wind_gust_2m_idx];

	result->value[6] = _get_average_values(config->wind_directions_10m, NUM_WIND_10_MIN_SAMPLES);
	result->value[7] = _get_average_values(config->wind_speeds_10m, NUM_WIND_10_MIN_SAMPLES);

	int wind_gust_10m_idx = _get_wind_gust_index(config->wind_speeds_10m, NUM_WIND_10_MIN_SAMPLES);
	result->value[8] = config->wind_directions_10m[wind_gust_10m_idx];
	result->value[9] = config->wind_speeds_10m[wind_gust_10m_idx];

	result->value[10] = _get_average_values(config->wind_directions_60m, NUM_WIND_60_MIN_SAMPLES);
	result->value[11] = _get_average_values(config->wind_speeds_60m, NUM_WIND_60_MIN_SAMPLES);

	int wind_gust_60m_idx = _get_wind_gust_index(config->wind_speeds_60m, NUM_WIND_60_MIN_SAMPLES);
	result->value[12] = config->wind_directions_60m[wind_gust_60m_idx];
	result->value[13] = config->wind_speeds_60m[wind_gust_60m_idx];

	pthread_mutex_unlock(&_wind_mutex);

	result->value[14] = rain_gauge;
	result->value[15] = list_sum(config->rain_gauge_1h);
	result->value[16] = list_sum(config->rain_gauge_6h);
	result->value[17] = list_sum(config->rain_gauge_24h);

	config->logger("current wind stats: wind_num_seen=%d, avg_wind_cps=%.1f, wind_speed=%.1f mph\n",
			wind_num_seen, avg_wind_cps, wind_speed);

	config->logger("2 minute wind stats: average direction=%.1f, average speed=%.1f mph, gust direction=%.1f, gust speed=%.1f\n",
			result->value[2], result->value[3], result->value[4], result->value[5]);

	config->logger("10 minute wind stats: average direction=%.1f, average speed=%.1f mph, gust direction=%.1f, gust speed=%.1f\n",
			result->value[6], result->value[7], result->value[8], result->value[9]);

	config->logger("60 minute wind stats: average direction=%.1f, average speed=%.1f mph, gust direction=%.1f, gust speed=%.1f\n",
			result->value[10], result->value[11], result->value[12], result->value[13]);

	config->logger("rain_num_seen=%d, rain_gauge (in): cur=%.1f, 1h=%.1f, 6h=%.1f, 24h=%.1f\n",
			rain_num_seen, rain_gauge, result->value[15], result->value[16],
			result->value[17]);

	result->unit = malloc(sizeof(char *) * 2);
	result->unit[0] = config->wind_speed_unit;
	result->unit[1] = config->rain_gauge_unit;

	return result;
}

static char * _argent_80422_value_header_names[] = { "wind_dir_cur", "wind_speed_cur",
							"wind_dir_avg_2m", "wind_speed_avg_2m",
							"wind_dir_gust_2m", "wind_speed_gust_2m",
							"wind_dir_avg_10m", "wind_speed_avg_10m",
							"wind_dir_gust_10m", "wind_speed_gust_10m",
							"wind_dir_avg_60m", "wind_speed_avg_60m",
							"wind_dir_gust_60m", "wind_speed_gust_60m", 
							"rain_gauge_cur", "rain_gauge_1h",
							"rain_gauge_6h", "rain_gauge_24h",
							NULL };

static char ** _argent_80422_get_value_header_names(__attribute__((__unused__)) yadl_config *config)
{
	return _argent_80422_value_header_names;
}

static char * _argent_80422_unit_header_names[] = { "wind_speed_unit", "rain_gauge_unit", NULL };

static char ** _argent_80422_get_unit_header_names(__attribute__((__unused__)) yadl_config *config)
{
        return _argent_80422_unit_header_names;
}

sensor argent_80422_sensor_funcs = {
	.init = &_argent_80422_init,
	.get_value_header_names = &_argent_80422_get_value_header_names,
	.get_unit_header_names = &_argent_80422_get_unit_header_names,
	.read = _argent_80422_read_data
};
