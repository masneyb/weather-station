/*
 * float_list.c
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

#include <stdlib.h>
#include "float_list.h"

int list_len(float_node *list)
{
        int ret = 0;

        for (float_node *node = list; node != NULL; node = node->next)
                ret++;

        return ret;
}

float list_sum(float_node *list)
{
        float sum = 0.0;

        for (float_node *node = list; node != NULL; node = node->next)
                sum += node->value;

        return sum;
}

float_node *list_last_node(float_node *list)
{
        float_node *curval = list;

        while (curval->next != NULL)
                curval = curval->next;

        return curval;
}

float_node *new_list_node(float value)
{
        float_node *ret = malloc(sizeof(*ret));
        ret->value = value;
        ret->next = NULL;
        return ret;
}

void free_list(float_node *list)
{
        float_node *val = list;

        while (val != NULL) {
                float_node *nextval = val->next;

                free(val);
                val = nextval;
        }
}

