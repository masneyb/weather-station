/*
 * filters.c
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

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yadl.h"

static int _list_len(float_node *list)
{
	int ret = 0;

	for (float_node *node = list; node != NULL; node = node->next)
		ret++;
	return ret;
}

static float sum_filter(float_node *list)
{
	float sum = 0.0;

	for (float_node *node = list; node != NULL; node = node->next)
		sum += node->value;
	return sum;
}

static float min_filter(float_node *list)
{
	return list->value;
}

static float max_filter(float_node *list)
{
	float_node *curval = list;

	while (curval->next != NULL)
		curval = curval->next;
	return curval->value;
}

/* Return the difference between the maximum and minimum number */
static float range_filter(float_node *list)
{
	float min = min_filter(list);
	float max = max_filter(list);

	return max - min;
}

static float median_filter(float_node *list)
{
	float_node *curval = list;

	int median_pos = _list_len(list) / 2;

	for (int i = 0; i < median_pos; i++)
		curval = curval->next;
	return curval->value;
}

static float mean_filter(float_node *list)
{
	float sum = sum_filter(list);

	return sum / _list_len(list);
}

/* Return the number that occurs the most number of times. Fallback to the
 *  mean if no numbers appear more than once.
 */
static float mode_filter(float_node *list)
{
	float cur_number = list->value;
	float cur_count = 0;
	float max_number = FLT_MIN;
	float max_count = 0;

	for (float_node *curval = list; curval != NULL; curval = curval->next) {
		if (cur_number == curval->value) {
			cur_count++;
			if (cur_count > 1 && cur_count > max_count) {
				max_number = cur_number;
				max_count = cur_count;
			}
		} else {
			cur_number = curval->value;
			cur_count = 1;
		}
	}

	/* no mode? */
	if (max_number == FLT_MIN)
		return mean_filter(list);

	return max_number;
}

filter get_filter(char *name)
{
	if (name == NULL)
		return &median_filter;
	else if (strcmp(name, "min") == 0)
		return &min_filter;
	else if (strcmp(name, "max") == 0)
		return &max_filter;
	else if (strcmp(name, "median") == 0)
		return &median_filter;
	else if (strcmp(name, "mean") == 0)
		return &mean_filter;
	else if (strcmp(name, "range") == 0)
		return &range_filter;
	else if (strcmp(name, "mode") == 0)
		return &mode_filter;
	else if (strcmp(name, "sum") == 0)
		return &sum_filter;

	fprintf(stderr, "Unknown filter '%s'\n", name);
	return NULL;
}

