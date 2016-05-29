/*
 * outputters.c
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rrd_common.h"
#include "yadl.h"

static long _get_current_timestamp()
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) < 0)
		return 0;
	return tv.tv_sec;
}

static FILE *_open_fd(yadl_config *config)
{
	if (config->outfile == NULL)
		return stdout;

	FILE *fd = fopen(config->outfile, "w");
	if (fd == NULL) {
		fprintf(stderr, "Error opening %s: %s\n", config->outfile, strerror(errno));
		exit(1);
	}
	return fd;
}

static void _close_fd(FILE *fd, yadl_config *config)
{
	if (config->outfile == NULL)
		return;

	if (fclose(fd) < 0) {
		fprintf(stderr, "Error closing%s: %s\n", config->outfile, strerror(errno));
		exit(1);
	}
}

static void _write_text(FILE *fd, __attribute__((__unused__)) int reading_number,
			yadl_result *result, __attribute__((__unused__)) yadl_config *config)
{
	if (config->only_log_value_changes) {
		if (config->last_value == result->value)
			return;
		config->last_value = result->value;
	}

	fprintf(fd, "Value: %.1f\nTimestamp: %ld\n",
		result->value,
		_get_current_timestamp());
}

static void _write_json_header(FILE *fd)
{
	fprintf(fd, "{ \"result\": [ ");
}

static void _write_json(FILE *fd, int reading_number, yadl_result *result, yadl_config *config)
{
	if (config->only_log_value_changes) {
		if (config->last_value == result->value)
			return;
		config->last_value = result->value;
	}

	fprintf(fd, "{ \"value\": %.1f, \"timestamp\": %ld }",
		result->value,
		_get_current_timestamp());

	if (reading_number + 1 < config->num_results)
		fprintf(fd, ",\n");
}

static void _write_json_footer(FILE *fd)
{
	fprintf(fd, " ] }\n");
}

static void _write_yaml_header(FILE *fd)
{
	fprintf(fd, "---\nresult:\n");
}

static void _write_yaml(FILE *fd, __attribute__((__unused__)) int reading_number,
		yadl_result *result, __attribute__((__unused__)) yadl_config *config)
{
	if (config->only_log_value_changes) {
		if (config->last_value == result->value)
			return;
		config->last_value = result->value;
	}

	fprintf(fd, "- value: %.1f\n  timestamp: %ld\n",
		result->value,
		_get_current_timestamp());
}

static void _write_csv_header(FILE *fd)
{
	fprintf(fd, "reading_number,timestamp,value\n");
}

static void _write_csv(FILE *fd, int reading_number,
		yadl_result *result, __attribute__((__unused__)) yadl_config *config)
{
	if (config->only_log_value_changes) {
		if (config->last_value == result->value)
			return;
		config->last_value = result->value;
	}

	fprintf(fd, "%d,%ld,%.1f\n", reading_number, _get_current_timestamp(),
		result->value);
}

static void _write_rrd(__attribute__((__unused__)) FILE *fd, __attribute__((__unused__)) int reading_number,
		yadl_result *result, yadl_config *config)
{
	if (config->only_log_value_changes) {
		if (config->last_value == result->value)
			return;
		config->last_value = result->value;
	}

	if (config->outfile == NULL) {
		fprintf(stderr, "--outfile must be specified for the RRD output\n");
		exit(1);
	}

	write_to_rrd_database(config->logger, config->outfile, result->value);
}

static void _write_xml_header(FILE *fd)
{
	fprintf(fd, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fd, "<results>\n");
}

static void _write_xml(FILE *fd, __attribute__((__unused__)) int reading_number,
		yadl_result *result, __attribute__((__unused__)) yadl_config *config)
{
	if (config->only_log_value_changes) {
		if (config->last_value == result->value)
			return;
		config->last_value = result->value;
	}

	fprintf(fd, "  <result><value>%.1f</value><timestamp>%ld</timestamp></result>\n",
		result->value,
		_get_current_timestamp());
}

static void _write_xml_footer(FILE *fd)
{
	fprintf(fd, "</results>\n");
}


static outputter _text_output_funcs = {
	.open = &_open_fd,
	.write_header = NULL,
	.write_result = &_write_text,
	.write_footer = NULL,
	.close = &_close_fd
};
static outputter _json_output_funcs = {
	.open = &_open_fd,
	.write_header = &_write_json_header,
	.write_result = &_write_json,
	.write_footer = &_write_json_footer,
	.close = &_close_fd
};
static outputter _yaml_output_funcs = {
	.open = &_open_fd,
	.write_header = &_write_yaml_header,
	.write_result = &_write_yaml,
	.write_footer = NULL,
	.close = &_close_fd
};
static outputter _csv_output_funcs = {
	.open = &_open_fd,
	.write_header = &_write_csv_header,
	.write_result = &_write_csv,
	.write_footer = NULL,
	.close = &_close_fd
};
static outputter _xml_output_funcs = {
	.open = &_open_fd,
	.write_header = &_write_xml_header,
	.write_result = &_write_xml,
	.write_footer = &_write_xml_footer,
	.close = &_close_fd
};
static outputter _rrd_output_funcs = {
	.open = NULL,
	.write_header = NULL,
	.write_result = &_write_rrd,
	.write_footer = NULL,
	.close = NULL
};

outputter *get_outputter(char *name)
{
	if (name == NULL)
		return NULL;
	else if (strcmp(name, "text") == 0)
		return &_text_output_funcs;
	else if (strcmp(name, "json") == 0)
		return &_json_output_funcs;
	else if (strcmp(name, "yaml") == 0)
		return &_yaml_output_funcs;
	else if (strcmp(name, "csv") == 0)
		return &_csv_output_funcs;
	else if (strcmp(name, "xml") == 0)
		return &_xml_output_funcs;
	else if (strcmp(name, "rrd") == 0)
		return &_rrd_output_funcs;

	fprintf(stderr, "Unknown output type '%s'\n", name);
	return NULL;
}

