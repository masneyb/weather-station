/*
 * sensors.c
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

#include <stdio.h>
#include <string.h>
#include "yadl.h"

sensor *get_sensor(char *name)
{
	if (name == NULL)
		return NULL;
	else if (strcmp(name, "digital") == 0)
		return &digital_sensor_funcs;
	else if (strcmp(name, "counter") == 0)
		return &digital_counter_sensor_funcs;
	else if (strcmp(name, "analog") == 0)
		return &analog_sensor_funcs;
	else if (strcmp(name, "argent_80422") == 0)
		return &argent_80422_sensor_funcs;
	else if (strcmp(name, "dht11") == 0)
		return &dht11_sensor_funcs;
	else if (strcmp(name, "dht22") == 0)
		return &dht22_sensor_funcs;
	else if (strcmp(name, "ds18b20") == 0)
		return &ds18b20_sensor_funcs;
	else if (strcmp(name, "tmp36") == 0)
		return &tmp36_sensor_funcs;

	fprintf(stderr, "Unknown sensor type '%s'\n", name);
	return NULL;
}

