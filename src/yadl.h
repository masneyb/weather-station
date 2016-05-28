/*
 * yadl.h
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
#include "loggers.h"

typedef struct yadl_result_tag {
	float value;
} yadl_result;

typedef struct sample_node_tag sample_node;

struct sample_node_tag {
	float sample;
	sample_node *next;
};

typedef struct yadl_config_tag yadl_config;

typedef struct adc_converter_tag {
	void (*adc_init)(yadl_config *config);
	int (*adc_read)(yadl_config *config);
	int adc_resolution;
} adc_converter;

typedef struct outputter_tag {
	FILE *(*open)(yadl_config *config);
	void (*write_header)(FILE *fd);
	void (*write_result)(FILE *fd, int reading_number, yadl_result *result, yadl_config *config);
	void (*write_footer)(FILE *fd);
	void (*close)(FILE *fd, yadl_config *config);
} outputter;

typedef struct sensor_tag {
	void (*init)(yadl_config *config);
	yadl_result * (*read)(yadl_config *config);
} sensor;

typedef float (*filter)(sample_node *list);

struct yadl_config_tag {
	sensor *sens;
	int gpio_pin;
	int min_valid_reading;
	int max_valid_reading;
	logger logger;
	char *outfile;
	int spi_channel;
	int i2c_address;
	int analog_channel;
	adc_converter *adc;
	int sleep_usecs_between_retries;
	int sleep_usecs_between_results;
	int sleep_usecs_between_samples;
	int max_retries;
	filter filter_func;
	int num_results;
	int num_samples_per_result;
	int remove_n_samples_from_ends;
};

filter get_filter(char *name);

outputter *get_outputter(char *name);

sensor analog_sensor_funcs;

sensor digital_sensor_funcs;

sensor *get_sensor(char *name);

adc_converter mcp3002_funcs;

adc_converter mcp3004_funcs;

adc_converter pcf8591_funcs;

adc_converter *get_adc(char *name);

logger get_logger(int debug);

void usage(void);
