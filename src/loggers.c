/*
 * loggers.c
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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "yadl.h"

static void _logger_noop(__attribute__((__unused__)) const char *format, ...)
{
	/* NOOP */
}

static void _logger_stderr(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

static FILE *_logfd = NULL;

static void _logger_fd(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(_logfd, format, args);
	va_end(args);
}

logger get_logger(int debug, char *logfile)
{
	if (!debug) {
		if (logfile != NULL) {
			fprintf(stderr, "You must also specify the --debug flag with the --logfile argument\n");
			usage();
		}

		return &_logger_noop;
	}

	if (logfile != NULL) {
		_logfd = fopen(logfile, "a");
		if (_logfd == NULL) {
			fprintf(stderr, "Error opening %s: %s\n", logfile, strerror(errno));
			exit(1);
		}
		return &_logger_fd;
	}

	return &_logger_stderr;
}

void close_logger(char *logfile)
{
	if (logfile == NULL)
		return;

	int ret = fclose(_logfd);
	if (ret < 0) {
		fprintf(stderr, "Error closing logfile %s: %s\n", logfile,
			strerror(errno));
		exit(1);
	}

	_logfd = NULL;
}

