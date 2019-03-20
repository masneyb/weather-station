/*
 * adc_pcf8591.c - Support for the MCP3002 analog to digital converter
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

#include <stdlib.h>
#include <pcf8591.h>
#include <wiringPi.h>
#include "yadl.h"

#define PIN_BASE 64

static void pcf8591_analog_init(yadl_config *config)
{
	if (config->analog_channel == -1) {
		fprintf(stderr,
			"You must specify the --analog_channel argument\n");
		usage();
	} else if (config->i2c_address == -1) {
		fprintf(stderr,
			"You must specify the --i2c_address argument\n");
		usage();
	}

	config->logger("pcf8591: Initializing pin base %d for I2C address %d\n",
			PIN_BASE, config->i2c_address);

	if (pcf8591Setup(PIN_BASE, config->i2c_address) == -1)
		exit(1);
}

static int pcf8591_analog_read(yadl_config *config)
{
	int chan = PIN_BASE + config->analog_channel;

	int ret = analogRead(chan);

	config->logger("pcf8591: Read value %d from pin base %d and analog channel %d\n",
			ret, PIN_BASE, config->analog_channel);

	return ret;
}

adc_converter pcf8591_funcs = {
	.adc_init = &pcf8591_analog_init,
	.adc_read = &pcf8591_analog_read,
	.adc_resolution = 255
};

