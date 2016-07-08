/*
 * sensor_temperature_tmp36.c - Support for the TMP 36 temperature sensor.
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yadl.h"

static void _tmp36_init(yadl_config *config)
{
	if (config->temperature_converter == NULL) {
		fprintf(stderr, "You must specify the --temperature_unit flag\n");
		usage();
	}

	if (config->adc == NULL) {
		fprintf(stderr, "You must specify the --adc argument\n");
		usage();
	}
	config->adc->adc_init(config);
}

static yadl_result *_tmp36_read_data(yadl_config *config)
{
	yadl_result *result;

	config->logger("tmp36: Beginning to perform analog read. adc_millivolts=%d, adc_resolution=%d, analog_scaling_factor=%d\n",
			config->adc_millivolts, config->adc->adc_resolution, config->analog_scaling_factor);

	int reading = config->adc->adc_read(config);
	int read_milli_volts = (float) reading * ((float) config->adc_millivolts / (float) config->adc->adc_resolution);
	float temperature = (read_milli_volts - config->analog_scaling_factor) / 10.0;

	config->logger("tmp36: Reading %d was converted to %d millivolts.\n",
			reading, read_milli_volts, temperature);

	config->logger("tmp36: temperature=%.2fC, humidity=unsupported\n", temperature);

	result = malloc(sizeof(*result));
	result->value = malloc(sizeof(float) * 1);
	result->value[0] = config->temperature_converter(temperature);
	return result;
}

static char * _tmp36_value_header_names[] = { "temperature", NULL };

static char ** _tmp36_get_value_header_names(__attribute__((__unused__)) yadl_config *config)
{
	return _tmp36_value_header_names;
}

sensor tmp36_sensor_funcs = {
	.init = &_tmp36_init,
	.get_value_header_names = &_tmp36_get_value_header_names,
	.read = _tmp36_read_data
};
