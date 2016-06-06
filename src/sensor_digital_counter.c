/*
 * sensor_digital_counter.c
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

static volatile int _interrupt_counter = 0;
static int _last_stop_counter = 0;
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

	if (wiringPiSetup() == -1)
		exit(1);

	config->logger("Using interrupt edge %s on GPIO pin %d; counter_show_speed=%d.\n",
			config->interrupt_edge, config->gpio_pin,
			config->counter_show_speed);

	if (strcmp(config->interrupt_edge, "rising") == 0)
		wiringPiISR(config->gpio_pin, INT_EDGE_RISING, &_interrupt_handler);
	else if (strcmp(config->interrupt_edge, "falling") == 0)
		wiringPiISR(config->gpio_pin, INT_EDGE_FALLING, &_interrupt_handler);
	else if (strcmp(config->interrupt_edge, "both") == 0)
		wiringPiISR(config->gpio_pin, INT_EDGE_BOTH, &_interrupt_handler);
	else {
		fprintf(stderr, "Invalid --interrupt_edge paramter %s\n", config->interrupt_edge);
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

	float value;
	if (config->counter_show_speed) {
		static struct timeval _current_time;
		gettimeofday(&_current_time, NULL);
		double elapsed_secs = ((_current_time.tv_sec - _last_time.tv_sec) * 1000000L + _current_time.tv_usec - _last_time.tv_usec) / 1000000L;
		_last_time.tv_sec = _current_time.tv_sec;
		_last_time.tv_usec = _current_time.tv_usec;

		float counts_per_sec = num_seen / elapsed_secs;
		value = counts_per_sec * config->counter_multiplier;

		config->logger("elapsed_secs=%.2f, counts_per_sec=%.2f\n",
				elapsed_secs, counts_per_sec);
	}
	else {
		value = num_seen * config->counter_multiplier;
	}

	config->logger("counter_multiplier=%.2f, value=%.2f\n", config->counter_multiplier, value);

	yadl_result *result;
	result = malloc(sizeof(*result));
	result->value = value;
	return result;
}

sensor digital_counter_sensor_funcs = {
	.init = _digital_counter_init,
	.read = _digital_counter_read_data
};
