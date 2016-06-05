/*
 * yadl-add-rrd-sample.c
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <float.h>
#include <unistd.h>
#include "loggers.h"
#include "rrd_common.h"

void usage(void)
{
	printf("usage: yadl-add-rrd-sample [ --debug ]\n");
	printf("\t\t--outfile <path to RRD database>\n");
	printf("\t\t--value <value>\n");
	printf("\n");
	printf("Note: A new RRD database will be created if it does not exist\n");
	exit(1);
}

int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{"debug", no_argument, 0, 0 },
		{"outfile", required_argument, 0, 0 },
		{"value", required_argument, 0, 0 },
		{"logfile", required_argument, 0, 0 },
		{0, 0, 0, 0 }
	};

	float value = FLT_MIN;
	char *outfile, *logfile;
	int opt = 0, long_index = 0, debug = 0;

	while ((opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1) {
		if (opt != 0)
			usage();

		errno = 0;
		switch (long_index) {
		case 0:
			debug = 1;
			break;
		case 1:
			outfile = optarg;
			break;
		case 2:
			value = strtof(optarg, NULL);
			break;
		case 3:
			logfile = optarg;
			break;
		default:
			usage();
		}

		if (errno != 0)
			usage();
	}

	if (value == FLT_MIN || outfile == NULL)
		usage();

	logger log = get_logger(debug, logfile);

	write_to_rrd_database(log, outfile, value);

	close_logger(logfile);

	return 0;
}

