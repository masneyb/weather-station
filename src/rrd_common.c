/*
 * rrd_common.c
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

#include <rrd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rrd_common.h"
#include "yadl.h"

static void _create_rrd_database(logger log, char *rrd_database, char **names)
{
	char **create_params;

	log("Creating RRD database %s\n", rrd_database);

	int num_headers = 0;
	for (; names[num_headers] != NULL; num_headers++);

	void **to_free = malloc(sizeof(void *) * num_headers);

	int argc = 16 + num_headers;
	int i = 0;

	create_params = malloc(sizeof(char *) * (argc + 1));

	create_params[i++] = "rrdcreate";
	create_params[i++] = rrd_database;
	create_params[i++] = "--start";
	create_params[i++] = "N";
	create_params[i++] = "--step";
	create_params[i++] = "300";

	for (int j = 0; j < num_headers; j++) {
		char *buf = malloc(100);
		snprintf(buf, 100, "DS:%s:GAUGE:600:U:U", names[j]);
		create_params[i++] = buf;
		to_free[j] = buf;
	}

	create_params[i++] = "RRA:MIN:0.5:1:12";
	create_params[i++] = "RRA:MIN:0.5:1:288";
	create_params[i++] = "RRA:MIN:0.5:12:168";
	create_params[i++] = "RRA:MIN:0.5:12:720";
	create_params[i++] = "RRA:MIN:0.5:288:365";
	create_params[i++] = "RRA:MAX:0.5:1:12";
	create_params[i++] = "RRA:MAX:0.5:1:288";
	create_params[i++] = "RRA:MAX:0.5:12:168";
	create_params[i++] = "RRA:MAX:0.5:12:720";
	create_params[i++] = "RRA:MAX:0.5:288:365";
	create_params[i++] = NULL;

	rrd_clear_error();
	rrd_create(argc, create_params);

	for (int j = 0; j < num_headers; j++) {
		free(to_free[j]);
	}
	free(to_free);
}

void write_to_rrd_database(logger log, char *rrd_database, char **names, float *values)
{
	struct stat st;
	char update_buf[255];
	char *updateparams[] = {
		"rrdupdate",
		rrd_database,
		"",
		NULL
	};

	if (stat(rrd_database, &st) == -1)
		_create_rrd_database(log, rrd_database, names);

	snprintf(update_buf, sizeof(update_buf), "N");
	for (int i = 0; names[i] != NULL; i++) {
		char str_value[20];

		snprintf(str_value, sizeof(str_value), ":%.1f", values[i]);
		strncat(update_buf, str_value, sizeof(update_buf));
	}

	updateparams[2] = update_buf;

	log("Writing data %s to RRD database %s\n", update_buf, rrd_database);

	rrd_clear_error();
	rrd_update(3, updateparams);
}

