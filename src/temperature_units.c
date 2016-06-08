/*
 * temperature_units.c
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

static float _celsius_to_fahrenheit(float input)
{
	return input * 9.0 / 5.0 + 32;
}

static float _celsius_to_celsius(float input)
{
	return input;
}

static float _celsius_to_kelvin(float input)
{
	return input + 273.15;
}

static float _celsius_to_rankine(float input)
{
	return (input + 273.15) * (9.0 / 5.0);
}

temperature_unit_converter get_temperature_converter(char *name)
{
	if (name == NULL)
		return NULL;
	else if (strcmp(name, "celsius") == 0)
		return &_celsius_to_celsius;
	else if (strcmp(name, "fahrenheit") == 0)
		return &_celsius_to_fahrenheit;
	else if (strcmp(name, "kelvin") == 0)
		return &_celsius_to_kelvin;
	else if (strcmp(name, "rankine") == 0)
		return &_celsius_to_rankine;

	fprintf(stderr, "Unknown tempearture unit '%s'\n", name);
	return NULL;
}

