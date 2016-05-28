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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rrd_common.h"
#include "yadl.h"

static void _create_rrd_database(logger log, char *rrd_database)
{
	char *createparams[] = {
		"rrdcreate",
		rrd_database,
		"--start",
		"N",
		"--step",
		"300",
		"DS:value:GAUGE:600:U:U",
		"RRA:MIN:0.5:1:12",
		"RRA:MIN:0.5:1:288",
		"RRA:MIN:0.5:12:168",
		"RRA:MIN:0.5:12:720",
		"RRA:MIN:0.5:288:365",
		"RRA:MAX:0.5:1:12",
		"RRA:MAX:0.5:1:288",
		"RRA:MAX:0.5:12:168",
		"RRA:MAX:0.5:12:720",
		"RRA:MAX:0.5:288:365",
		NULL
	};

	log("Creating RRD database %s\n", rrd_database);

	rrd_clear_error();
	rrd_create(18, createparams);
}

void write_to_rrd_database(logger log, char *rrd_database, float value)
{
	struct stat st;
	char buf[100];
	char *updateparams[] = {
		"rrdupdate",
		rrd_database,
		"",
		NULL
	};

	if (stat(rrd_database, &st) == -1)
		_create_rrd_database(log, rrd_database);

	snprintf(buf, sizeof(buf), "N:%.1f", value);
	updateparams[2] = buf;

	log("Writing data %s to RRD database %s\n", buf, rrd_database);

	rrd_clear_error();
	rrd_update(3, updateparams);
}

