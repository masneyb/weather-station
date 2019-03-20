/*
 * sensor_digital_counter.c
 *
 * Copyright (C) 2016-2017 Brian Masney <masneyb@onstation.org>
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

static volatile int _interrupt_counter;
static int _last_stop_counter;
static struct timeval _last_time;

void _interrupt_handler(void)
{
	_interrupt_counter++;
}

static void _digital_counter_init(yadl_config *config)
{
	if (config->gpio_pin == -1) {
		fprintf(stderr, "You must specify the --gpio_pin argument\n");
		usage();
	}

	config->logger("Using interrupt edge %s on GPIO pin %d\n",
			config->interrupt_edge, config->gpio_pin);

	if (strcmp(config->interrupt_edge, "rising") == 0)
		wiringPiISR(config->gpio_pin, INT_EDGE_RISING,
			    &_interrupt_handler);
	else if (strcmp(config->interrupt_edge, "falling") == 0)
		wiringPiISR(config->gpio_pin, INT_EDGE_FALLING,
			    &_interrupt_handler);
	else if (strcmp(config->interrupt_edge, "both") == 0)
		wiringPiISR(config->gpio_pin, INT_EDGE_BOTH,
			    &_interrupt_handler);
	else {
		fprintf(stderr, "Invalid --interrupt_edge paramter %s\n",
			config->interrupt_edge);
		usage();
	}

	gettimeofday(&_last_time, NULL);

	/* Wait for the first sample */
	if (config->sleep_millis_between_results > 0)
		delay(config->sleep_millis_between_results);
}

static yadl_result *_digital_counter_read_data(yadl_config *config)
{
	int start_counter = _last_stop_counter;
	int stop_counter = _interrupt_counter;

	_last_stop_counter = stop_counter;

	int num_seen;
	/* Check to see if the number wrapped */
	if (stop_counter < start_counter)
		num_seen = (INT_MAX - start_counter) + stop_counter;
	else
		num_seen = stop_counter - start_counter;

	config->logger("start=%d, stop=%d, num_seen=%d\n",
			start_counter, stop_counter, num_seen);

	static struct timeval _current_time;

	gettimeofday(&_current_time, NULL);
	double elapsed_secs = ((_current_time.tv_sec - _last_time.tv_sec) *
			       1000000L + _current_time.tv_usec -
			       _last_time.tv_usec) / 1000000L;

	_last_time.tv_sec = _current_time.tv_sec;
	_last_time.tv_usec = _current_time.tv_usec;

	float counts_per_sec = num_seen / elapsed_secs;

	config->logger("num_seen=%d, elapsed_secs=%.2f, counts_per_sec=%.2f, counter_multiplier=%.2f\n",
			num_seen, elapsed_secs, counts_per_sec,
			config->counter_multiplier);

	yadl_result *result = malloc(sizeof(*result));

	result->value = malloc(sizeof(float) * 4);
	result->value[0] = num_seen;
	result->value[1] = num_seen * config->counter_multiplier;
	result->value[2] = counts_per_sec;
	result->value[3] = counts_per_sec * config->counter_multiplier;

	result->unit = NULL;

	return result;
}

static char *_digital_counter_value_header_names[] = {
	"num_seen", "num_seen_with_multiplier", "counts_per_sec",
	"counts_per_sec_with_multiplier", NULL
};

static char **_digital_counter_get_value_header_names(__attribute__((__unused__))
						      yadl_config *config)
{
	return _digital_counter_value_header_names;
}

sensor digital_counter_sensor_funcs = {
	.init = _digital_counter_init,
	.get_value_header_names = &_digital_counter_get_value_header_names,
	.read = _digital_counter_read_data
};
