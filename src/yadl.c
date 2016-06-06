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
#include <sys/types.h>
#include "yadl.h"

#define DEFAULT_SLEEP_MILLIS_BETWEEN_RETRIES 500
#define DEFAULT_SLEEP_MILLIS_BETWEEN_SAMPLES 0
#define DEFAULT_SLEEP_MILLIS_BETWEEN_RESULTS 0
#define DEFAULT_MAX_RETRIES                 1
#define DEFAULT_NUM_SAMPLES_PER_RESULT      1
#define DEFAULT_NUM_RESULTS                 1
#define DEFAULT_REMOVE_N_SAMPLES_FROM_ENDS  0
#define DEFAULT_COUNTER_MULTIPLIER          1.0
#define DEFAULT_INTERRUPT_EDGE              "rising"

void usage(void)
{
	printf("usage: yadl --sensor <analog|digital|counter>\n");
	printf("\t\t[ --gpio_pin <wiringPi pin #. Required for digital pins> ]\n");
	printf("\t\t  See http://wiringpi.com/pins/ to lookup the pin number.\n");
	printf("\t\t[ --adc <see ADC list below. Required for analog> ]\n");
	printf("\t\t--output <text|json|yaml|csv|xml|rrd>\n");
	printf("\t\t[ --outfile <optional output filename. Defaults to stdout> ]\n");
	printf("\t\t[ --only_log_value_changes ]\n");
	printf("\t\t[ --num_results <# results returned (default %d). Set to -1 to poll indefinitely.> ]\n", DEFAULT_NUM_RESULTS);
	printf("\t\t[ --sleep_millis_between_results <milliseconds (default %d)> ]\n", DEFAULT_SLEEP_MILLIS_BETWEEN_RESULTS);
	printf("\t\t[ --num_samples_per_result <# samples (default %d). See --filter for aggregation.> ]\n", DEFAULT_NUM_SAMPLES_PER_RESULT);
	printf("\t\t[ --sleep_millis_between_samples <milliseconds (default %d)> ]\n", DEFAULT_SLEEP_MILLIS_BETWEEN_SAMPLES);
	printf("\t\t[ --filter <median|mean|mode|min|max|range (default median)> ]\n");
	printf("\t\t[ --remove_n_samples_from_ends <# samples (default %d)> ]\n", DEFAULT_REMOVE_N_SAMPLES_FROM_ENDS);
	printf("\t\t[ --max_retries <# retries (default %d)> ]\n", DEFAULT_MAX_RETRIES);
	printf("\t\t[ --sleep_millis_between_retries <milliseconds (default %d)> ]\n", DEFAULT_SLEEP_MILLIS_BETWEEN_RETRIES);
	printf("\t\t[ --min_valid_value <minimum allowable value> ]\n");
	printf("\t\t[ --max_valid_value <maximum allowable value> ]\n");
	printf("\t\t[ --debug ]\n");
	printf("\t\t[ --logfile <path to debug logs. Uses stderr if not specified.> ]\n");
	printf("\t\t[ --daemon ]\n");
	printf("\n");
	printf("Counter specific options\n");
	printf("\n");
	printf("\t\t[ --counter_multiplier <multiplier to convert the requests per second to some other value. (default %.1f)> ]\n", DEFAULT_COUNTER_MULTIPLIER);
	printf("\t\t[ --interrupt_edge <rising|falling|both (default %s)> ]\n", DEFAULT_INTERRUPT_EDGE);
	printf("\t\t[ --counter_show_speed ]\n");
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
	printf("* Poll a single sample from BCM digital pin 17 (wiringPi pin 0) as JSON\n");
	printf("  $ yadl --sensor digital --gpio_pin 0 --output json\n");
	printf("  { \"result\": [ { \"value\": 0.0, \"timestamp\": 1464465651 } ] }\n");
	printf("\n");
	printf("* Poll 7 results from an analog sensor hooked up to channel 0 of a MCP3008.\n");
	printf("  Wait 50 milliseconds between each result shown.\n");
	printf("  $ yadl --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 0 \\\n");
	printf("	--output csv --num_results 7 --sleep_millis_between_results 50\n");
	printf("  reading_number,timestamp,value\n");
	printf("  0,1464465367,716.0\n");
	printf("  1,1464465367,712.0\n");
	printf("  2,1464465367,712.0\n");
	printf("  3,1464465367,708.0\n");
	printf("  4,1464465367,712.0\n");
	printf("  5,1464465367,712.0\n");
	printf("  6,1464465367,712.0\n");
	printf("\n");
	printf("* Show 5 averaged results from a photoresistor hooked up to an ADC.\n");
	printf("  1000 samples are taken for each result shown. 200 samples from each end\n");
	printf("  are removed and the mean is taken of the middle 600 samples. This is\n");
	printf("  useful for removing noise from analog sensors.\n");
	printf("  $ yadl --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 0 \\\n");
	printf("	--output csv --num_results 5 --sleep_millis_between_results 2000 \\\n");
	printf("	--num_samples_per_result 1000 --remove_n_samples_from_ends 200 --filter mean\n");
	printf("  reading_number,timestamp,value\n");
	printf("  0,1464469264,779.4\n");
	printf("  1,1464469267,778.7\n");
	printf("  2,1464469269,779.7\n");
	printf("  3,1464469271,779.8\n");
	printf("  4,1464469273,779.2\n");
	printf("\n");
	printf("* Hook an anemometer (wind speed meter) up to a digital pin and count the\n");
	printf("  number of times that the switch closes over a 5 second period. Multiply the\n");
	printf("  requests per second by 0.746 to get the wind speed in miles per hour. Show\n");
	printf("  5 different results.\n");
	printf("  $ yadl --sensor counter --counter_show_speed --gpio_pin 1 --output csv --num_results 5 \\\n");
	printf("  	--sleep_millis_between_results 5000 --counter_multiplier 0.746\n");
	printf("  reading_number,timestamp,value\n");
	printf("  0,1465084823,6.9\n");
	printf("  1,1465084828,6.9\n");
	printf("  2,1465084833,7.2\n");
	printf("  3,1465084838,6.9\n");
	printf("  4,1465084843,6.9\n");
	printf("\n");
	printf("* Hook a button up to a digital pin and check for bounce when the button\n");
	printf("  is pressed. This polls the digital pin indefinitely until Crtl-C is\n");
	printf("  pressed. Note: Newlines were added for clarity between the two button\n");
	printf("  presses for illustration purposes.\n");
	printf("  $ yadl --sensor digital --gpio_pin 0 --output csv --num_results -1 \\\n");
	printf("	--only_log_value_changes\n");
	printf("  reading_number,timestamp,value\n");
	printf("  0,1464480347,0.0\n");
	printf("\n");
	printf("  636673,1464480348,1.0\n");
	printf("  687383,1464480348,0.0\n");
	printf("\n");
	printf("  1678984,1464480351,1.0\n");
	printf("  1731987,1464480351,0.0\n");
	printf("  1731988,1464480351,1.0\n");
	printf("  1732148,1464480351,0.0\n");

	exit(1);
}

static yadl_result *_perform_reading(yadl_config *config)
{
	yadl_result *result;

	int success = 0;

	for (int i = 0; i < config->max_retries; i++) {
		if (i > 0 && config->sleep_millis_between_retries > 0)
			delay(config->sleep_millis_between_retries);

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
		if (i > 0 && config->sleep_millis_between_samples > 0)
			delay(config->sleep_millis_between_samples);

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

static void _dofork(yadl_config *config)
{
	pid_t pid;

	pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Error forking: %s\n", strerror(errno));
		exit(1);
	}
	else if (pid > 0) {
		config->logger("Terminating parent process %d\n", pid);
		/* In parent process */
		exit(0);
	}

	/* Now running in child process */
}

static void _daemonize(yadl_config *config)
{
	config->logger("Note: Running in daemon mode\n");

	if (chdir("/") < 0) {
		fprintf(stderr, "Error changing current path to /: %s\n", strerror(errno));
		exit(1);
	}

	_dofork(config);
	_dofork(config);

	if (setsid() < 0) {
		fprintf(stderr, "Error calling setsid(): %s\n", strerror(errno));
		exit(1);
	}

	close(0);
	close(1);
	close(2);
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
		{"sleep_millis_between_retries", required_argument, 0, 0 },
		{"num_samples_per_result", required_argument, 0, 0 },
		{"analog_channel", required_argument, 0, 0 },
		{"sleep_millis_between_samples", required_argument, 0, 0 },
		{"filter", required_argument, 0, 0 },
		{"remove_n_samples_from_ends", required_argument, 0, 0 },
		{"num_results", required_argument, 0, 0 },
		{"sleep_millis_between_results", required_argument, 0, 0 },
		{"i2c_address", required_argument, 0, 0 },
		{"only_log_value_changes", no_argument, 0, 0 },
		{"counter_multiplier", required_argument, 0, 0 },
		{"logfile", required_argument, 0, 0 },
		{"daemon", no_argument, 0, 0 },
		{"interrupt_edge", required_argument, 0, 0 },
		{"counter_show_speed", no_argument, 0, 0 },
		{0, 0, 0, 0 }
	};

	char *output_name = NULL, *sensor_name = NULL, *adc_name = NULL;
	char *filter_name = NULL, *logfile = NULL;
	yadl_config config;
	outputter *output_funcs;
	int opt = 0, long_index = 0, debug = 0, daemon = 0;
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
	config.sleep_millis_between_retries = DEFAULT_SLEEP_MILLIS_BETWEEN_RETRIES;
	config.sleep_millis_between_samples = DEFAULT_SLEEP_MILLIS_BETWEEN_SAMPLES;
	config.analog_channel = -1;
	config.num_results = DEFAULT_NUM_RESULTS;
	config.num_samples_per_result = DEFAULT_NUM_SAMPLES_PER_RESULT;
	config.remove_n_samples_from_ends = DEFAULT_REMOVE_N_SAMPLES_FROM_ENDS;
	config.sleep_millis_between_samples = DEFAULT_SLEEP_MILLIS_BETWEEN_SAMPLES;
	config.only_log_value_changes = 0;
	config.last_value = -1;
	config.counter_multiplier = DEFAULT_COUNTER_MULTIPLIER;
	config.interrupt_edge = DEFAULT_INTERRUPT_EDGE;

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
			config.sleep_millis_between_retries = strtol(optarg, NULL, 10);
			break;
		case 11:
			config.num_samples_per_result = strtol(optarg, NULL, 10);
			break;
		case 12:
			config.analog_channel = strtol(optarg, NULL, 10);
			break;
		case 13:
			config.sleep_millis_between_samples = strtol(optarg, NULL, 10);
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
			config.sleep_millis_between_results = strtol(optarg, NULL, 10);
			break;
		case 18:
			config.i2c_address = strtol(optarg, NULL, 16);
			break;
		case 19:
			config.only_log_value_changes = 1;
			break;
		case 20:
			config.counter_multiplier = strtof(optarg, NULL);
			break;
		case 21:
			logfile = optarg;
			break;
		case 22:
			daemon = 1;
			break;
		case 23:
			config.interrupt_edge = optarg;
			break;
		case 24:
			config.counter_show_speed = 1;
			break;
		default:
			usage();
		}

		if (errno != 0)
			usage();
	}

	if (config.num_samples_per_result <= 0) {
		fprintf(stderr, "--num_samples_per_result must be > 0\n");
		usage();
	} else if (config.remove_n_samples_from_ends < 0) {
		fprintf(stderr, "--remove_n_samples_from_ends must be >= 0\n");
		usage();
	} else if (config.num_samples_per_result <= (config.remove_n_samples_from_ends * 2)) {
		fprintf(stderr, "--remove_n_samples_from_ends * 2 must be less than --num_samples_per_result\n");
		usage();
	} else if (daemon && debug && logfile == NULL) {
		fprintf(stderr, "You must specify the --logfile argument with --daemon\n");
		usage();
	} else if (daemon && config.outfile == NULL) {
		fprintf(stderr, "You must specify the --outfile argument with --daemon\n");
		usage();
	}

	config.logger = get_logger(debug, logfile);

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

	config.logger("min_valid_value=%d; max_valid_value=%d\n",
			config.min_valid_reading, config.max_valid_reading);
	config.logger("num_results=%d; sleep_millis_between_samples=%d; num_samples_per_result=%d; sleep_millis_between_samples=%d\n",
			config.num_results, config.sleep_millis_between_samples, config.num_samples_per_result, config.sleep_millis_between_samples);
	config.logger("max_retries=%d; sleep_millis_between_retries=%d\n",
			config.max_retries, config.sleep_millis_between_retries);

	if (wiringPiSetup() == -1)
		exit(1);

	if (config.sens->init != NULL)
		config.sens->init(&config);

	FILE *fd = NULL;
	if (output_funcs->open != NULL)
		fd = output_funcs->open(&config);

	if (output_funcs->write_header != NULL)
		output_funcs->write_header(fd);

	if (daemon) {
		/* don't write duplicate headers to the output file*/
		fflush(fd);

		_daemonize(&config);
	}

	for (int i = 0; i < config.num_results || config.num_results < 0; i++) {
		if (i > 0 && config.sleep_millis_between_results > 0)
			delay(config.sleep_millis_between_results);

		result = _perform_all_readings(&config);
		output_funcs->write_result(fd, i, result, &config);
	}

	if (output_funcs->write_footer != NULL)
		output_funcs->write_footer(fd);

	if (output_funcs->close != NULL)
		output_funcs->close(fd, &config);

	free(result);

	close_logger(logfile);

	return 0;
}

