/*
 * adcs.c
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
#include <stdlib.h>
#include <string.h>
#include "yadl.h"

adc_converter *get_adc(char *name)
{
	if (name == NULL)
		return NULL;
	else if (strcmp(name, "mcp3002") == 0)
		return &mcp3002_funcs;
	else if (strcmp(name, "mcp3004") == 0 || strcmp(name, "mcp3008") == 0)
		return &mcp3004_funcs;
	else if (strcmp(name, "pcf8591") == 0)
		return &pcf8591_funcs;

	fprintf(stderr, "Unknown ADC type '%s'\n", name);
	exit(1);
}

