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
#include "yadl.h"

static volatile int _interrupt_counter = 0;

void _interrupt_handler(void)
{
	_interrupt_counter++;
}

static void _digital_counter_init(__attribute__((__unused__)) yadl_config *config)
{
	if (config->gpio_pin == -1) {
		fprintf(stderr, "You must specify the --gpio_pin argument\n");
		usage();
	}

	if (wiringPiSetup() == -1)
		exit(1);

	wiringPiISR (config->gpio_pin, INT_EDGE_FALLING, &_interrupt_handler);
}

static yadl_result *_digital_counter_read_data(yadl_config *config)
{
	config->logger("Counting the number of times the pin %d goes from high to low for the next %d seconds.\n",
			config->gpio_pin, config->counter_poll_secs);

	int start_counter = _interrupt_counter;

	/* Sleep and wait for the interupt handler to do the counting */	
	sleep(config->counter_poll_secs);

	int stop_counter = _interrupt_counter;

	int num_seen;
	/* Check to see if the number wrapped */
	if (stop_counter < start_counter)
		num_seen = (INT_MAX - start_counter) + stop_counter;
	else
		num_seen = stop_counter - start_counter;

	float counts_per_sec = (float) num_seen / config->counter_poll_secs;
	float value = counts_per_sec * config->counter_multiplier;

	config->logger("counter_poll_secs=%d, start_counter=%d, stop_counter=%d, num_seen=%d, counts_per_sec=%.2f\n",
			config->counter_poll_secs, start_counter, stop_counter, num_seen, counts_per_sec);
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
