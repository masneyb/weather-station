/*
 * yadl.c
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include "yadl.h"

#define DEFAULT_SLEEP_USECS_BETWEEN_RETRIES 500000
#define DEFAULT_SLEEP_USECS_BETWEEN_SAMPLES 0
#define DEFAULT_SLEEP_USECS_BETWEEN_RESULTS 0
#define DEFAULT_MAX_RETRIES                 1
#define DEFAULT_NUM_SAMPLES_PER_RESULT      1
#define DEFAULT_NUM_RESULTS                 1
#define DEFAULT_REMOVE_N_SAMPLES_FROM_ENDS  0

void usage(void)
{
	printf("usage: yadl --sensor <analog|digital See sensor options below>\n");
	printf("\t\t--output <text|json|yaml|csv|xml|rrd>\n");
	printf("\t\t[ --outfile <optional output filename. Defaults to stdout> ]\n");
	printf("\t\t[ --num_results <# results returned (default %d)> ]\n", DEFAULT_NUM_RESULTS);
	printf("\t\t[ --sleep_usecs_between_results <usecs (default %d)> ]\n", DEFAULT_SLEEP_USECS_BETWEEN_RESULTS);
	printf("\t\t[ --num_samples_per_result <# samples (default %d). See --filter for aggregation.> ]\n", DEFAULT_NUM_SAMPLES_PER_RESULT);
	printf("\t\t[ --sleep_usecs_between_samples <usecs (default %d)> ]\n", DEFAULT_SLEEP_USECS_BETWEEN_SAMPLES);
	printf("\t\t[ --filter <median|mean|mode|min|max|range (default median)> ]\n");
	printf("\t\t[ --remove_n_samples_from_ends <# samples (default %d)> ]\n", DEFAULT_REMOVE_N_SAMPLES_FROM_ENDS);
	printf("\t\t[ --max_retries <# retries (default %d)> ]\n", DEFAULT_MAX_RETRIES);
	printf("\t\t[ --sleep_usecs_between_retries <usecs (default %d)> ]\n", DEFAULT_SLEEP_USECS_BETWEEN_RETRIES);
	printf("\t\t[ --min_valid_value <minimum allowable value> ]\n");
	printf("\t\t[ --max_valid_value <maximum allowable value> ]\n");
	printf("\t\t[ --debug ]\n");
	printf("\n");
	printf("Supported Sensors\n");
	printf("\n");
	printf("* digital\n");
	printf("  --gpio_pin <wiringPi pin #>\n");
	printf("  \tSee http://wiringpi.com/pins/ to lookup the pin number.\n");
	printf("\n");
	printf("* analog\n");
	printf("  --adc <see ADC list below>\n");
	printf("\n");
	printf("Supported Analog to Digital Converters (ADCs)\n");
	printf("\n");
	printf("* mcp3002 / mcp3004 / mcp3008 - 10-bit ADCs with a SPI interface.\n");
	printf("  --spi_channel <spi channel>\n");
	printf("  \tThe SPI channel is either 0 or 1 for the Raspberry Pi.\n");
	printf("\n");
	printf("  --analog_channel <analog channel>\n");
	printf("\n");
	printf("  You need to have the proper spi_bcmXXXX kernel module loaded on the Pi.\n");
	printf("\n");
	printf("* pcf8591 - 8-bit ADC with an I2C interface.\n");
	printf("  --i2c_address <I2C hex address>\n");
	printf("  \tUse i2cdetect to scan your bus. 48 is the default if you have a single board.\n");
	printf("\n");
	printf("  --analog_channel <analog channel>\n");
	printf("\n");
	printf("Examples\n");
	printf("\n");
	printf("* Polling an analog sensor hooked up to channel 0 of a MCP3008\n");
	printf("  $ yadl --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 0 --output csv --num_results 7 --sleep_usecs_between_results 50000\n");
	printf("  timestamp,value\n");
	printf("  1464465367,716.0\n");
	printf("  1464465367,712.0\n");
	printf("  1464465367,712.0\n");
	printf("  1464465367,708.0\n");
	printf("  1464465367,712.0\n");
	printf("  1464465367,712.0\n");
	printf("  1464465367,712.0\n");
	printf("\n");
	printf("* Polling BCM digital pin 17 (wiringPi pin 0) as JSON\n");
	printf("  $ yadl --sensor digital --gpio_pin 0 --output json --num_results 1 --num_samples_per_result 1\n");
	printf("  { \"result\": [ { \"value\": 0.0, \"timestamp\": 1464465651 } ] }\n");

	exit(1);
}

static yadl_result *_perform_reading(yadl_config *config)
{
	yadl_result *result;

	int success = 0;

	for (int i = 0; i < config->max_retries; i++) {
		if (i > 0 && config->sleep_usecs_between_retries > 0)
			delayMicroseconds(config->sleep_usecs_between_retries);

		result = config->sens->read(config);
		if (result == NULL) {
			/* bad reading */
			continue;
		}

		if (result->value < config->min_valid_reading ||
				result->value > config->max_valid_reading) {
			config->logger("Received reading outside of allowable ranges. value=%.2f. Retrying.\n",
					result->value);
			free(result);
			continue;
		}
		success = 1;
		break;
	}

	if (!success) {
		fprintf(stderr, "Error: Reached maximum retries.\n");
		exit(1);
	}

	return result;
}

static sample_node *_add_sample_to_sorted_list(float sample, sample_node *list)
{
	sample_node *newnode = malloc(sizeof(*newnode));

	newnode->sample = sample;
	newnode->next = NULL;

	if (list == NULL)
		return newnode;

	/* Add to head of list */
	if (list->sample > sample) {
		newnode->next = list;
		return newnode;
	}

	sample_node *lastnode = list;

	/* Add to middle of list */
	for (sample_node *cur = list->next; cur != NULL; cur = cur->next) {
		if (cur->sample > sample) {
			newnode->next = cur;
			lastnode->next = newnode;
			return list;
		}
		lastnode = cur;
	}

	/* Add to end of list */
	lastnode->next = newnode;
	return list;
}

static void _free_list(sample_node *list)
{
	sample_node *val = list;

	while (val != NULL) {
		sample_node *nextval = val->next;

		free(val);
		val = nextval;
	}
}

static void _dump_list(char *description, sample_node *list, yadl_config *config)
{
	config->logger("%s: Sorted samples are:", description);
	for (sample_node *val = list; val != NULL; val = val->next)
		config->logger(" %.2f", val->sample);
	config->logger("\n");
}

static sample_node *_remove_outliers(sample_node *list, yadl_config *config)
{
	if (config->remove_n_samples_from_ends <= 0)
		return list;

	sample_node *last_node_on_head_of_list = NULL;
	sample_node *cur = list;

	for (int i = 0; i < config->num_samples_per_result - config->remove_n_samples_from_ends - 1; i++) {
		if (i == (config->remove_n_samples_from_ends - 1))
			last_node_on_head_of_list = cur;
		cur = cur->next;
	}

	/* Chop off the end of the list */
	_free_list(cur->next);
	cur->next = NULL;

	/* Chop off the head of the list */
	sample_node *new_head_of_list = last_node_on_head_of_list->next;

	last_node_on_head_of_list->next = NULL;
	_free_list(list);

	return new_head_of_list;
}

static yadl_result *_perform_all_readings(yadl_config *config)
{
	sample_node *value_list = NULL;

	for (int i = 0; i < config->num_samples_per_result; i++) {
		if (i > 0 && config->sleep_usecs_between_samples > 0)
			delayMicroseconds(config->sleep_usecs_between_samples);

		yadl_result *sample = _perform_reading(config);

		config->logger("Sample #%d: value=%.2f\n", i, sample->value);

		value_list = _add_sample_to_sorted_list(sample->value, value_list);

		free(sample);
	}

	_dump_list("Values", value_list, config);
	value_list = _remove_outliers(value_list, config);
	_dump_list("Values(after removing outliers)", value_list, config);

	yadl_result *result = malloc(sizeof(*result));
	result->value = config->filter_func(value_list);
	_free_list(value_list);

	return result;
}

int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{"gpio_pin", required_argument, 0, 0 },
		{"min_valid_value", required_argument, 0, 0 },
		{"max_valid_value", required_argument, 0, 0 },
		{"output", required_argument, 0, 0 },
		{"sensor", required_argument, 0, 0 },
		{"debug", no_argument, 0, 0 },
		{"outfile", required_argument, 0, 0 },
		{"spi_channel", required_argument, 0, 0 },
		{"adc", required_argument, 0, 0 },
		{"max_retries", required_argument, 0, 0 },
		{"sleep_usecs_between_retries", required_argument, 0, 0 },
		{"num_samples_per_result", required_argument, 0, 0 },
		{"analog_channel", required_argument, 0, 0 },
		{"sleep_usecs_between_samples", required_argument, 0, 0 },
		{"filter", required_argument, 0, 0 },
		{"remove_n_samples_from_ends", required_argument, 0, 0 },
		{"num_results", required_argument, 0, 0 },
		{"sleep_usecs_between_results", required_argument, 0, 0 },
		{"i2c_address", required_argument, 0, 0 },
		{0, 0, 0, 0 }
	};

	char *output_name = NULL, *sensor_name = NULL, *adc_name = NULL;
	char *filter_name = NULL;
	yadl_config config;
	outputter *output_funcs;
	int opt = 0, long_index = 0, debug = 0;
	yadl_result *result;

	if (argc <= 1)
		usage();

	memset(&config, 0, sizeof(config));
	config.gpio_pin = -1;
	config.spi_channel = -1;
	config.i2c_address = -1;
	config.min_valid_reading = INT_MIN;
	config.max_valid_reading = INT_MAX;
	config.max_retries = DEFAULT_MAX_RETRIES;
	config.sleep_usecs_between_retries = DEFAULT_SLEEP_USECS_BETWEEN_RETRIES;
	config.sleep_usecs_between_samples = DEFAULT_SLEEP_USECS_BETWEEN_SAMPLES;
	config.analog_channel = -1;
	config.num_results = DEFAULT_NUM_RESULTS;
	config.num_samples_per_result = DEFAULT_NUM_SAMPLES_PER_RESULT;
	config.remove_n_samples_from_ends = DEFAULT_REMOVE_N_SAMPLES_FROM_ENDS;
	config.sleep_usecs_between_samples = DEFAULT_SLEEP_USECS_BETWEEN_SAMPLES;

	while ((opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1) {
		if (opt != 0)
			usage();

		errno = 0;
		switch (long_index) {
		case 0:
			config.gpio_pin = strtol(optarg, NULL, 10);
			break;
		case 1:
			config.min_valid_reading = strtol(optarg, NULL, 10);
			break;
		case 2:
			config.max_valid_reading = strtol(optarg, NULL, 10);
			break;
		case 3:
			output_name = optarg;
			break;
		case 4:
			sensor_name = optarg;
			break;
		case 5:
			debug = 1;
			break;
		case 6:
			config.outfile = optarg;
			break;
		case 7:
			config.spi_channel = strtol(optarg, NULL, 10);
			break;
		case 8:
			adc_name = optarg;
			break;
		case 9:
			config.max_retries = strtol(optarg, NULL, 10);
			break;
		case 10:
			config.sleep_usecs_between_retries = strtol(optarg, NULL, 10);
			break;
		case 11:
			config.num_samples_per_result = strtol(optarg, NULL, 10);
			break;
		case 12:
			config.analog_channel = strtol(optarg, NULL, 10);
			break;
		case 13:
			config.sleep_usecs_between_samples = strtol(optarg, NULL, 10);
			break;
		case 14:
			filter_name = optarg;
			break;
		case 15:
			config.remove_n_samples_from_ends = strtol(optarg, NULL, 10);
			break;
		case 16:
			config.num_results = strtol(optarg, NULL, 10);
			break;
		case 17:
			config.sleep_usecs_between_results = strtol(optarg, NULL, 10);
			break;
		case 18:
			config.i2c_address = strtol(optarg, NULL, 16);
			break;
		default:
			usage();
		}

		if (errno != 0)
			usage();
	}

	if (config.num_samples_per_result <= 0) {
		fprintf(stderr, "num_samples_per_result must be > 0\n");
		usage();
	} else if (config.remove_n_samples_from_ends < 0) {
		fprintf(stderr, "remove_n_samples_from_ends must be >= 0\n");
		usage();
	} else if (config.num_samples_per_result <= (config.remove_n_samples_from_ends * 2)) {
		fprintf(stderr, "remove_n_samples_from_ends * 2 must be less than num_samples_per_result\n");
		usage();
	}

	config.sens = get_sensor(sensor_name);
	if (config.sens == NULL) {
		fprintf(stderr, "You must specify the --sensor argument\n");
		usage();
	}

	output_funcs = get_outputter(output_name);
	if (output_funcs == NULL)
		usage();

	config.filter_func = get_filter(filter_name);
	if (config.filter_func == NULL)
		usage();

	config.adc = get_adc(adc_name);

	config.logger = get_logger(debug);

	config.logger("min_valid_value=%d; max_valid_value=%d\n",
			config.min_valid_reading, config.max_valid_reading);
	config.logger("num_results=%d; sleep_usecs_between_samples=%d; num_samples_per_result=%d; sleep_usecs_between_samples=%d\n",
			config.num_results, config.sleep_usecs_between_samples, config.num_samples_per_result, config.sleep_usecs_between_samples);
	config.logger("max_retries=%d; sleep_usecs_between_retries=%d\n",
			config.max_retries, config.sleep_usecs_between_retries);

	if (wiringPiSetup() == -1)
		exit(1);

	if (config.sens->init != NULL)
		config.sens->init(&config);

	FILE *fd = NULL;
	if (output_funcs->open != NULL)
		fd = output_funcs->open(&config);

	if (output_funcs->write_header != NULL)
		output_funcs->write_header(fd);

	for (int i = 0; i < config.num_results; i++) {
		if (i > 0 && config.sleep_usecs_between_results > 0)
			delayMicroseconds(config.sleep_usecs_between_results);

		result = _perform_all_readings(&config);
		output_funcs->write_result(fd, i, result, &config);
	}

	if (output_funcs->write_footer != NULL)
		output_funcs->write_footer(fd);

	if (output_funcs->close != NULL)
		output_funcs->close(fd, &config);

	free(result);

	return 0;
}

