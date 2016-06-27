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
#include <unistd.h>
#include <sys/time.h>
#include "yadl.h"

#define RAIN_GAUGE_MULTIPLIER 0.011
#define WIND_SPEED_MULTIPLIER 1.492

static volatile int _wind_current_counter = 0;
static volatile int _rain_current_counter = 0;
static int _wind_last_counter = 0;
static int _rain_last_counter = 0;
static struct timeval _last_time;

static void _wind_speed_handler(void)
{
        _wind_current_counter++;
}

static void _rain_gauge_handler(void)
{
        _rain_current_counter++;
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

	/* Wait for the first sample */
	if (config->sleep_millis_between_results > 0)
		delay(config->sleep_millis_between_results);
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
static float _get_wind_direction(int millivolts)
{
	float direction = -1.0;
	int distance = 0;

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

	return direction;
}

static float _get_counts_per_sec(int start_counter, int stop_counter, double elapsed_secs)
{
	int num_seen;
	/* Check to see if the number wrapped */
	if (stop_counter < start_counter)
		num_seen = (INT_MAX - start_counter) + stop_counter;
	else
		num_seen = stop_counter - start_counter;

	return num_seen / elapsed_secs;
}

static yadl_result *_argent_80422_read_data(yadl_config *config)
{
	/* Poll wind direction */
	config->logger("Beginning to perform analog read. adc_millivolts=%d, adc_resolution=%d\n",
			config->adc_millivolts, config->adc->adc_resolution);

	int reading = config->adc->adc_read(config);
	int read_millivolts = (float) reading * ((float) config->adc_millivolts / (float) config->adc->adc_resolution);
	float wind_direction = _get_wind_direction(read_millivolts);
	config->logger("Wind direction: analog reading=%d; %d millivolts; direction (degrees)=%.1f\n",
			reading, read_millivolts, wind_direction);

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

	float avg_wind_cps = _get_counts_per_sec(start_wind_counter, stop_wind_counter, elapsed_secs);
	float avg_rain_cps = _get_counts_per_sec(start_rain_counter, stop_rain_counter, elapsed_secs);

	float wind_speed = avg_wind_cps * WIND_SPEED_MULTIPLIER;
	float rain_gauge = avg_rain_cps * RAIN_GAUGE_MULTIPLIER;

	config->logger("avg_wind_cps=%.1f, wind_speed=%.1f mph, avg_rain_cps=%.1f, rain_gauge=%.1f\n",
			avg_wind_cps, wind_speed, avg_rain_cps, rain_gauge);

	yadl_result *result;
	result = malloc(sizeof(*result));
	result->value = malloc(sizeof(float) * 3);
	result->value[0] = wind_direction;
	result->value[1] = wind_speed;
	result->value[2] = rain_gauge;
	return result;
}

static char * _argent_80422_value_header_names[] = { "wind_direction", "wind_speed", "rain_gauge", NULL };

static char ** _argent_80422_get_value_header_names(__attribute__((__unused__)) yadl_config *config)
{
	return _argent_80422_value_header_names;
}

sensor argent_80422_sensor_funcs = {
	.init = &_argent_80422_init,
	.get_value_header_names = &_argent_80422_get_value_header_names,
	.read = _argent_80422_read_data
};
