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

static int show_value(yadl_config *config, yadl_result *result)
{
	if (!config->only_log_value_changes)
		return 1;

	int num_values = get_num_values(config);

	int equal = 1;
	for (int i = 0; i < num_values; i++) {
		if (config->last_values[i] != result->value[i]) {
			equal = 0;
			break;
		}
	}

	if (equal)
		return 0;

	for (int i = 0; i < num_values; i++) {
		config->last_values[i] = result->value[i];
	}

	return 1;
}

static output_metadata *_open_fd(yadl_config *config, char *outfile)
{
	output_metadata *ret = malloc(sizeof(*ret));
	ret->outfile = outfile;

	if (outfile == NULL) {
		ret->fd = stdout;
		return ret;
	}

	config->logger("Writing results to file %s\n", outfile);

	ret->fd = fopen(outfile, "w");
	if (ret->fd == NULL) {
		fprintf(stderr, "Error opening %s: %s\n", outfile, strerror(errno));
		exit(1);
	}

	return ret;
}

static void _close_fd(output_metadata *meta, __attribute__((__unused__)) yadl_config *config)
{
	if (meta->outfile == NULL) {
		free(meta);
		return;
	}

	if (fclose(meta->fd) < 0) {
		fprintf(stderr, "Error closing %s: %s\n", meta->outfile, strerror(errno));
		free(meta);
		exit(1);
	}
}

static void _write_json_header(output_metadata *meta, __attribute__((__unused__)) yadl_config *config)
{
	fprintf(meta->fd, "{ \"result\": [ ");
}

static void _write_json(output_metadata *meta, int reading_number, yadl_result *result, yadl_config *config)
{
	if (!show_value(config, result))
		return;

	fprintf(meta->fd, " {");

	char **header_names = config->sens->get_value_header_names(config);
	for (int i = 0; header_names[i] != NULL; i++) {
		fprintf(meta->fd, " \"%s\": %.1f,", header_names[i], result->value[i]);
	}

	fprintf(meta->fd, " \"timestamp\": %ld }", _get_current_timestamp());

	if (reading_number + 1 < config->num_results)
		fprintf(meta->fd, ",\n");
}

static void _write_json_footer(output_metadata *meta)
{
	fprintf(meta->fd, " ] }\n");
}

static void _write_yaml_header(output_metadata *meta, __attribute__((__unused__)) yadl_config *config)
{
	fprintf(meta->fd, "---\nresult:\n");
}

static void _write_yaml(output_metadata *meta, __attribute__((__unused__)) int reading_number,
		yadl_result *result, __attribute__((__unused__)) yadl_config *config)
{
	if (!show_value(config, result))
		return;

	char **header_names = config->sens->get_value_header_names(config);
	for (int i = 0; header_names[i] != NULL; i++) {
		fprintf(meta->fd, "%c %s: %.1f\n", i == 0 ? '-' : ' ', header_names[i], result->value[i]);
	}
	fprintf(meta->fd, "  timestamp: %ld\n",
		_get_current_timestamp());
}

static void _write_csv_header(output_metadata *meta, yadl_config *config)
{
	fprintf(meta->fd, "reading_number,timestamp");

	char **header_names = config->sens->get_value_header_names(config);
	for (int i = 0; header_names[i] != NULL; i++) {
		fprintf(meta->fd, ",%s", header_names[i]);
	}

	fprintf(meta->fd, "\n");
}

static void _write_csv(output_metadata *meta, int reading_number,
		yadl_result *result, __attribute__((__unused__)) yadl_config *config)
{
	if (!show_value(config, result))
		return;

	fprintf(meta->fd, "%d,%ld", reading_number, _get_current_timestamp());

	char **header_names = config->sens->get_value_header_names(config);
	for (int i = 0; header_names[i] != NULL; i++) {
		fprintf(meta->fd, ",%.1f", result->value[i]);
	}
	fprintf(meta->fd, "\n");
}

static output_metadata *_rrd_open_fd(__attribute__((__unused__)) yadl_config *config, char *outfile)
{
	output_metadata *ret = malloc(sizeof(*ret));
	ret->outfile = outfile;
	ret->fd = NULL;
	return ret;
}

static void _rrd_close_fd(output_metadata *meta, __attribute__((__unused__)) yadl_config *config)
{
	free(meta);
}

static void _write_rrd(output_metadata *meta, __attribute__((__unused__)) int reading_number,
		yadl_result *result, yadl_config *config)
{
	if (!show_value(config, result))
		return;

	if (meta->outfile == NULL) {
		fprintf(stderr, "--outfile must be specified for the RRD output\n");
		exit(1);
	}

	char **header_names = config->sens->get_value_header_names(config);
	write_to_rrd_database(config->logger, meta->outfile, header_names, result->value);
}

static void _write_xml_header(output_metadata *meta, __attribute__((__unused__)) yadl_config *config)
{
	fprintf(meta->fd, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(meta->fd, "<results>\n");
}

static void _write_xml(output_metadata *meta, __attribute__((__unused__)) int reading_number,
		yadl_result *result, __attribute__((__unused__)) yadl_config *config)
{
	if (!show_value(config, result))
		return;

	fprintf(meta->fd, "  <result><timestamp>%ld</timestamp>", _get_current_timestamp());

	char **header_names = config->sens->get_value_header_names(config);
	for (int i = 0; header_names[i] != NULL; i++) {
		fprintf(meta->fd, "<%s>%.1f</%s>", header_names[i], result->value[i], header_names[i]);
	}

	fprintf(meta->fd, "</result>\n");
}

static void _write_xml_footer(output_metadata *meta)
{
	fprintf(meta->fd, "</results>\n");
}

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
	.open = &_rrd_open_fd,
	.write_header = NULL,
	.write_result = &_write_rrd,
	.write_footer = NULL,
	.close = &_rrd_close_fd
};

outputter *get_outputter(char *name)
{
	if (name == NULL)
		return NULL;
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

