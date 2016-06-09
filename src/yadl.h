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
	float *value;
} yadl_result;

typedef struct float_node_tag float_node;

struct float_node_tag {
	float value;
	float_node *next;
};

typedef struct yadl_config_tag yadl_config;

typedef struct adc_converter_tag {
	void (*adc_init)(yadl_config *config);
	int (*adc_read)(yadl_config *config);
	int adc_resolution;
} adc_converter;

typedef struct output_metadata_tag {
	FILE *fd;
	char *outfile;
} output_metadata;

typedef struct outputter_tag {
	output_metadata *(*open)(yadl_config *config, char *outfile);
	void (*write_header)(output_metadata *meta, yadl_config *config);
	void (*write_result)(output_metadata *meta, int reading_number, yadl_result *result, yadl_config *config);
	void (*write_footer)(output_metadata *meta);
	void (*close)(output_metadata *meta, yadl_config *config);
} outputter;

typedef struct sensor_tag {
	void (*init)(yadl_config *config);
	yadl_result * (*read)(yadl_config *config);
	char ** (*get_value_header_names)(yadl_config *config);
} sensor;

typedef float (*filter)(float_node *list);

typedef float (*temperature_unit_converter)(float input);

struct yadl_config_tag {
	sensor *sens;
	int gpio_pin;
	int min_valid_reading;
	int max_valid_reading;
	logger logger;
	int spi_channel;
	int i2c_address;
	int analog_channel;
	adc_converter *adc;
	int sleep_millis_between_retries;
	int sleep_millis_between_results;
	int sleep_millis_between_samples;
	int max_retries;
	filter filter_func;
	int num_results;
	int num_samples_per_result;
	int remove_n_samples_from_ends;
	int only_log_value_changes;
	float *last_values;
	float counter_multiplier;
	char *interrupt_edge;
	int adc_millivolts;
	char *temperature_unit;
	temperature_unit_converter temperature_converter;
	char *w1_slave;
	int analog_scaling_factor;
};

filter get_filter(char *name);

outputter *get_outputter(char *name);

sensor analog_sensor_funcs;

sensor wind_direction_sensor_funcs;

sensor digital_sensor_funcs;

sensor digital_counter_sensor_funcs;

sensor dht11_sensor_funcs;

sensor dht22_sensor_funcs;

sensor ds18b20_sensor_funcs;

sensor tmp36_sensor_funcs;

sensor *get_sensor(char *name);

adc_converter mcp3002_funcs;

adc_converter mcp3004_funcs;

adc_converter pcf8591_funcs;

adc_converter *get_adc(char *name);

void usage(void);

int get_num_values(yadl_config *config);

temperature_unit_converter get_temperature_converter(char *name);
