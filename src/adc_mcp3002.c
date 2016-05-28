/*
 * adc_mcp3002.c - Support for the MCP3002 analog to digital converter
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

#include <stdlib.h>
#include <mcp3002.h>
#include <wiringPi.h>
#include "yadl.h"

#define PIN_BASE 64

static void mcp3002_analog_init(yadl_config *config)
{
	if (config->spi_channel == -1) {
		fprintf(stderr, "You must specify the --spi_channel argument\n");
		usage();
	}
	else if (config->analog_channel == -1) {
		fprintf(stderr, "You must specify the --analog_channel argument\n");
		usage();
	}

	config->logger("mcp3002: Initializing pin base %d for SPI channel %d\n",
			PIN_BASE, config->spi_channel);

	if (mcp3002Setup(PIN_BASE, config->spi_channel) == -1)
		exit(1);
}

static int mcp3002_analog_read(yadl_config *config)
{
	int chan = PIN_BASE + config->analog_channel;

	config->logger("mcp3002: Reading from pin base %d and analog channel %d\n",
			PIN_BASE, config->analog_channel);

	int ret = analogRead(chan);

	config->logger("mcp3002: Read value %d\n", ret);
	return ret;
}

adc_converter mcp3002_funcs = {
	.adc_init = &mcp3002_analog_init,
	.adc_read = &mcp3002_analog_read,
	.adc_resolution = 1024
};

