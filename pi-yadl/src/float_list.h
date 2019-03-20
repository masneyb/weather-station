/*
 * float_list.h
 *
 * Copyright (C) 2016-2017 Brian Masney <masneyb@onstation.org>
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

typedef struct float_node_tag float_node;

struct float_node_tag {
	float value;
	float_node *next;
};

int list_len(float_node *list);

float list_sum(float_node *list);

float_node *list_last_node(float_node *list);

float_node *new_list_node(float value);

void free_list(float_node *list);

