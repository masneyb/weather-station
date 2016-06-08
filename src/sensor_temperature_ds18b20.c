/*
 * sensor_tempeature_ds18b20.c - Support for the DS18B20 temperature sensor
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

/* Example output from /sys/bus/w1/devices/28-031554ae2eff/w1_slave:
 * 58 01 80 80 1f ff 80 80 f8 : crc=f8 YES
 * 58 01 80 80 1f ff 80 80 f8 t=21500
 */

static char *_read_line(char *buf, int size, FILE *stream)
{
	char *ret = fgets(buf, size, stream);

	if (ret == NULL)
		return NULL;

	size_t len = strlen(buf);

	if (buf[len - 1] == '\n') {
		buf[len - 1] = '\0';
		len--;
	}
	if (buf[len - 1] == '\r')
		buf[len - 1] = '\0';
	return ret;
}

static yadl_result *_ds18b20_read_data(yadl_config *config)
{
	yadl_result *result;
	float temperature;
	char buf[255];
	char *pos;
	FILE *fd;

	config->logger("ds18b20: Opening w1 slave %s\n", config->w1_slave);

	fd = fopen(config->w1_slave, "r");
	if (fd == NULL) {
		fprintf(stderr, "ds18b20: Error opening file %s: %s\n", config->w1_slave,
			strerror(errno));
		exit(1);
	}

	if (_read_line(buf, sizeof(buf), fd) == NULL) {
		fprintf(stderr, "ds18b20: Error reading file %s: %s\n", config->w1_slave,
			errno == 0 ? "Premature end of file" : strerror(errno));
		fclose(fd);
		exit(1);
	}
	config->logger("ds18b20: Skipping first line '%s' from w1 slave %s\n", buf, config->w1_slave);

	if (_read_line(buf, sizeof(buf), fd) == NULL) {
		fprintf(stderr, "ds18b20: Error reading file %s: %s\n", config->w1_slave,
			errno == 0 ? "Premature end of file" : strerror(errno));
		fclose(fd);
		exit(1);
	}

	config->logger("ds18b20: Processing line '%s' from w1 slave %s\n", buf, config->w1_slave);

	fclose(fd);

	for (pos = buf; *pos != '\0'; pos++) {
		if (*pos == 't' && *(pos+1) == '=') {
			pos = pos + 2;
			break;
		}
	}

	if (*pos == '\0') {
		fprintf(stderr, "ds18b20: Could not parse line '%s' from w1 slave %s\n", buf, config->w1_slave);
		exit(1);
	}

	temperature = strtol(pos, NULL, 10) / 1000.0;
	config->logger("ds18b20: temperature=%.2fC, humidity=unsupported\n", temperature);

	result = malloc(sizeof(*result));
	result->value = malloc(sizeof(float) * 1);
	result->value[0] = config->temperature_converter(temperature);
	return result;
}

static void _ds18b20_init(__attribute__((__unused__)) yadl_config *config)
{
	if (config->w1_slave == NULL) {
		fprintf(stderr, "You must specify the --w1_slave argument\n");
		usage();
	}

	if (config->temperature_converter == NULL) {
		fprintf(stderr, "You must specify the --temperature_unit flag\n");
		usage();
	}
}

static char * _ds18b20_value_header_names[] = { "temperature", NULL };

static char ** _ds18b20_get_value_header_names(__attribute__((__unused__)) yadl_config *config)
{
        return _ds18b20_value_header_names;
}

sensor ds18b20_sensor_funcs = {
        .init = &_ds18b20_init,
	.get_value_header_names = &_ds18b20_get_value_header_names,
        .read = &_ds18b20_read_data
};

