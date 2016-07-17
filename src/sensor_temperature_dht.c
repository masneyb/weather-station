/*
 * dht_temperature_sensor.c - Support for the DHT11/DHT22 sensors
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "yadl.h"

#define BIT_ENABLED_THRESHOLD	      20
#define TIMEOUT_USECS                255

static void _dht11_parse_data(int data[5], yadl_result *result)
{
	result->value[0] = ((data[2] << 8) | data[3]) / 256.0;
	result->value[1] = ((data[0] << 8) | data[1]) / 256.0;
}

static void _dht22_parse_data(int data[5], yadl_result *result)
{
	result->value[0] = ((data[2] & 0x7F) * 256.0 + data[3]) / 10.0;
	if (data[2] & 0x80)
		result->value[0] *= -1.0;

	result->value[1] = (data[0] * 256.0 + data[1]) / 10.0;
}

static int _get_usecs_pin_is_in_state(yadl_config *config, int state)
{
	int microsecs_in_current_state = 0;

	while (digitalRead(config->gpio_pin) == state) {
		microsecs_in_current_state++;
		delayMicroseconds(1);

		/* check for a timeout */
		if (microsecs_in_current_state == TIMEOUT_USECS)
			return -1;
	}
	return microsecs_in_current_state;
}

float _calculate_dew_point(float temperature, float humidity)
{
	// This uses the August-Roche-Magnus approximation to calulate the dew point
	// based on the temperature and humidity.
	// See http://andrew.rsmas.miami.edu/bmcnoldy/Humidity.html
	return 243.04*(log(humidity/100)+((17.625*temperature)/(243.04+temperature)))/(17.625-log(humidity/100)-((17.625*temperature)/(243.04+temperature)));
}

/*
 * See the DHT11 data sheet that is linked in the README for a good
 * description of the overall process. */
static yadl_result *_dht_read_data(char *sensor_descr,
	yadl_config *config,
	void (*dht_parser)(int data[5], yadl_result *result))
{
	config->logger("%s: Polling sensor on GPIO pin %d. Note: usecs times shown below are approximate.\n",
			sensor_descr, config->gpio_pin);

	/* Signal to the DHT sensor that we want data. */
	pinMode(config->gpio_pin, OUTPUT);
	digitalWrite(config->gpio_pin, LOW);
	delay(18);

	/* Now set pin state to high */
	digitalWrite(config->gpio_pin, HIGH);

	/* Read response from sensor */
	pinMode(config->gpio_pin, INPUT);

	/* Wait while the pin is high */
	if (_get_usecs_pin_is_in_state(config, 1) < 0) {
		config->logger("%s: Sensor is not ready during phase 1 of the handshake.\n", sensor_descr);
		return NULL;
	}

	/* Sensor signals it is ready for data by setting pin to low for 80us */
	if (_get_usecs_pin_is_in_state(config, 0) < 0) {
		config->logger("%s: Sensor is not ready during phase 2 of the handshake.\n", sensor_descr);
		return NULL;
	}

	/* Sensor signals it is ready for data by setting pin to high for 80us */
	if (_get_usecs_pin_is_in_state(config, 1) < 0) {
		config->logger("%s: Sensor is not ready during phase 3 of the handshake.\n", sensor_descr);
		return NULL;
	}

	/* Now we are ready to start reading the 40 bits from the sensor */
	int data[5] = { 0 };
	int bit_pos = 0;

	for (; bit_pos < 40; bit_pos++) {
		/*
		 * Get the amount of time that the pin is low and high. The length
		 * of time that it is high determines the bit state. */
		int usecs_low = _get_usecs_pin_is_in_state(config, 0);
		if (usecs_low < 0) {
			config->logger("%s: Timeout reading bit position %d.\n",
					sensor_descr, bit_pos);
			break;
		}

		int usecs_high = _get_usecs_pin_is_in_state(config, 1);
		if (usecs_high < 0) {
			config->logger("%s: Timeout reading bit position %d.\n",
					sensor_descr, bit_pos);
			break;
		}

		/*
		 * If the pin was high for 26-28 us, then the bit is 0. If the pin
		 * was high for 70 us, then the bit is 1. Due to the scheduling
		 * policy, and lack of realtime scheduling, the times here are
		 * approximate. */
		int byte_pos = bit_pos / 8;
		data[byte_pos] <<= 1;
		if (usecs_high > BIT_ENABLED_THRESHOLD)
			data[byte_pos] |= 1;
	}

	if (bit_pos < 40) {
		config->logger("%s: Only read %d of bits from the sensor. Needed 40. Retrying.\n",
				sensor_descr, bit_pos);
		return NULL;
	}

	/* Wait for the sensor to go back to low power mode. */
	_get_usecs_pin_is_in_state(config, 0);

	config->logger("%s: Read data 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x from sensor\n",
			sensor_descr, data[0], data[1], data[2], data[3], data[4]);

	int checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
	if (data[4] != checksum) {
		config->logger("%s: Computed checksum 0x%02x does not match checksum 0x%02x read from the sensor. Retrying.\n",
				sensor_descr, checksum, data[4]);
		return NULL;
	}

	config->logger("%s: Computed checksum 0x%02x matches checksum received from sensor.\n",
			sensor_descr, checksum, data[4]);

	yadl_result *result = malloc(sizeof(*result));

	result->value = malloc(sizeof(float) * 3);
	dht_parser(data, result);
	result->value[2] = _calculate_dew_point(result->value[0], result->value[1]);
	config->logger("%s: temperature=%.2fC, humidity=%.2f%%, dew_point=%.2fC\n",
			sensor_descr, result->value[0], result->value[1], result->value[2]);
	result->value[0] = config->temperature_converter(result->value[0]);
	result->value[2] = config->temperature_converter(result->value[2]);

	result->unit = malloc(sizeof(char *) * 1);
	result->unit[0] = config->temperature_unit;

	return result;
}

static yadl_result *_dht11_read_data(yadl_config *config)
{
	return _dht_read_data("DHT11", config, &_dht11_parse_data);
}

static yadl_result *_dht22_read_data(yadl_config *config)
{
	return _dht_read_data("DHT22", config, &_dht22_parse_data);
}

static void _dht_init(__attribute__((__unused__)) yadl_config *config)
{
	if (config->gpio_pin == -1) {
		fprintf(stderr, "You must specify the --gpio_pin argument\n");
		usage();
	}

        if (config->temperature_converter == NULL) {
		fprintf(stderr, "You must specify the --temperature_unit flag\n");
		usage();
	}
}

static char * _dht_value_header_names[] = { "temperature", "humidity", "dew_point", NULL };

static char ** _dht_get_value_header_names(__attribute__((__unused__)) yadl_config *config)
{
        return _dht_value_header_names;
}

static char * _dht_unit_header_names[] = { "temperature_unit", NULL };

static char ** _dht_get_unit_header_names(__attribute__((__unused__)) yadl_config *config)
{
        return _dht_unit_header_names;
}

sensor dht11_sensor_funcs = {
	.init = &_dht_init,
	.get_value_header_names = &_dht_get_value_header_names,
	.get_unit_header_names = &_dht_get_unit_header_names,
	.read = &_dht11_read_data
};

sensor dht22_sensor_funcs = {
	.init = &_dht_init,
	.get_value_header_names = &_dht_get_value_header_names,
	.get_unit_header_names = &_dht_get_unit_header_names,
	.read = &_dht22_read_data
};

