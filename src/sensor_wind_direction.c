/*
 * sensor_wind_direction.c
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

static void _wind_direction_init(yadl_config *config)
{
	if (config->adc == NULL) {
		fprintf(stderr, "You must specify the --adc argument\n");
		usage();
	}
	config->adc->adc_init(config);
}

static void _check_direction(int read_millivolts, int compare_millivolts,
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
static float _get_direction(int millivolts)
{
	float direction = -1.0;
	int distance = 0;

	/* Find the closest reading to the values listed in the datasheet */
	_check_direction(millivolts, 3840, 0.0, &direction, &distance);
	_check_direction(millivolts, 1980, 22.5, &direction, &distance);
	_check_direction(millivolts, 2250, 45.0, &direction, &distance);
	_check_direction(millivolts, 410, 67.5, &direction, &distance);
	_check_direction(millivolts, 450, 90.0, &direction, &distance);
	_check_direction(millivolts, 320, 112.5, &direction, &distance);
	_check_direction(millivolts, 900, 135.0, &direction, &distance);
	_check_direction(millivolts, 620, 157.5, &direction, &distance);
	_check_direction(millivolts, 1400, 180.0, &direction, &distance);
	_check_direction(millivolts, 1190, 202.5, &direction, &distance);
	_check_direction(millivolts, 3080, 225.0, &direction, &distance);
	_check_direction(millivolts, 2930, 247.5, &direction, &distance);
	_check_direction(millivolts, 4620, 270.0, &direction, &distance);
	_check_direction(millivolts, 4040, 292.5, &direction, &distance);
	_check_direction(millivolts, 4780, 315.0, &direction, &distance);
	_check_direction(millivolts, 3430, 337.5, &direction, &distance);

	return direction;
}

static yadl_result *_wind_direction_read_data(yadl_config *config)
{
	config->logger("Beginning to perform analog read. adc_millivolts=%d, adc_resolution=%d, adc_show_millivolts=%d\n",
			config->adc_millivolts, config->adc->adc_resolution, config->adc_show_millivolts);

	int reading = config->adc->adc_read(config);
	int read_millivolts = (float) reading * ((float) config->adc_millivolts / (float) config->adc->adc_resolution);
	float direction = _get_direction(read_millivolts);
	config->logger("analog reading=%d; %d millivolts; wind direction=%.1f\n",
			reading, read_millivolts, direction);

	yadl_result *result;
	result = malloc(sizeof(*result));
	result->value = direction;
	return result;
}

sensor wind_direction_sensor_funcs = {
	.init = &_wind_direction_init,
	.read = _wind_direction_read_data
};
