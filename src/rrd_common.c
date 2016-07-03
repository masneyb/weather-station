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

static void _create_rrd_database(char *rrd_database, char **names)
{
	printf("Error: The RRD database %s does not exist. Please manually\n", rrd_database);
	printf("create this first with your desired parameters. If you would like to poll every\n");
	printf("30 seconds, then you may want to consider creating a database with this command:\n");
	printf("\n");

	printf("\trrdtool create %s \\\n", rrd_database);
	printf("\t\t--start N --step 30 \\\n");

	int num_headers = 0;
	for (; names[num_headers] != NULL; num_headers++);

	for (int j = 0; j < num_headers; j++) {
		printf("\t\tDS:%s:GAUGE:600:U:U \\\n", names[j]);
	}

	printf("\t\tRRA:MIN:0.5:1:120 \\\n");
	printf("\t\tRRA:MIN:0.5:2:120 \\\n");
	printf("\t\tRRA:MIN:0.5:4:120 \\\n");
	printf("\t\tRRA:MIN:0.5:10:288 \\\n");
	printf("\t\tRRA:MIN:0.5:20:1008 \\\n");
	printf("\t\tRRA:MIN:0.5:60:1440 \\\n");
	printf("\t\tRRA:MIN:0.5:80:3240 \\\n");
	printf("\t\tRRA:MIN:0.5:100:5184 \\\n");
	printf("\t\tRRA:MIN:0.5:120:8760 \\\n");
	printf("\t\tRRA:MIN:0.5:240:8760 \\\n");
	printf("\t\tRRA:MIN:0.5:360:8760 \\\n");

	printf("\t\tRRA:MAX:0.5:1:120 \\\n");
	printf("\t\tRRA:MAX:0.5:2:120 \\\n");
	printf("\t\tRRA:MAX:0.5:4:120 \\\n");
	printf("\t\tRRA:MAX:0.5:10:288 \\\n");
	printf("\t\tRRA:MAX:0.5:20:1008 \\\n");
	printf("\t\tRRA:MAX:0.5:60:1440 \\\n");
	printf("\t\tRRA:MAX:0.5:80:3240 \\\n");
	printf("\t\tRRA:MAX:0.5:100:5184 \\\n");
	printf("\t\tRRA:MAX:0.5:120:8760 \\\n");
	printf("\t\tRRA:MAX:0.5:240:8760 \\\n");
	printf("\t\tRRA:MAX:0.5:360:8760\n");
	printf("\n");

	printf("A different solution when polling every 5 minutes and keeping less data\n");
	printf("would be:\n");
	printf("\n");

	printf("\trrdtool create %s \\\n", rrd_database);
	printf("\t\t--start N --step 300 \\\n");

	for (int j = 0; j < num_headers; j++) {
		printf("\t\tDS:%s:GAUGE:600:U:U \\\n", names[j]);
	}

	printf("\t\tRRA:MIN:0.5:1:288 \\\n");
	printf("\t\tRRA:MIN:0.5:12:720 \\\n");
	printf("\t\tRRA:MIN:0.5:288:365 \\\n");

	printf("\t\tRRA:MAX:0.5:1:288 \\\n");
	printf("\t\tRRA:MAX:0.5:12:720 \\\n");
	printf("\t\tRRA:MAX:0.5:288:365\n");

	printf("\n");
	printf("See https://stackoverflow.com/questions/15774423/how-set-rrd-to-store-for-2-years for\n");
	printf("more details.\n");
	exit(1);
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
		_create_rrd_database(rrd_database, names);

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

