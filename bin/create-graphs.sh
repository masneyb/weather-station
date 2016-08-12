#!/bin/bash

# create-graphs.sh
#
# Copyright (C) 2016 Brian Masney <masneyb@onstation.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

YADL_BIN_DIR="${1:-}"
WEB_BASE_DIR="${2:-}"

if [ "${YADL_BIN_DIR}" = "" ] || [ "${WEB_BASE_DIR}" = "" ] ; then
	echo "usage: $0 <path to pi-yadl/bin/ directory> <path to web/ directory" >&2
	exit 1
fi

CREATE_MIN_MAX_GRAPHS="${YADL_BIN_DIR}"/create-min-max-graphs.sh
CREATE_MAX_GRAPHS="${YADL_BIN_DIR}"/create-max-graphs.sh

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/temperature_humidity \
	"Temperature and Dew Point" \
	"Temperature" \
	"${WEB_BASE_DIR}"/temperature_humidity.rrd \
	temperature \
	"Dew Point" \
	"${WEB_BASE_DIR}"/temperature_humidity.rrd \
	dew_point

"${CREATE_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/wind_speed \
	"Wind Speed" \
	"Wind Speed Average" \
	"${WEB_BASE_DIR}"/argent_80422.rrd \
	wind_speed_avg_2m \
	"Wind Speed Gust" \
	"${WEB_BASE_DIR}"/argent_80422.rrd \
	wind_speed_gust_2m

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/rain_gauge \
	"Rain Gauge" \
	"1 Hour Totals" \
	"${WEB_BASE_DIR}"/argent_80422.rrd \
	rain_gauge_1h \
	"Daily Totals" \
	"${WEB_BASE_DIR}"/argent_80422.rrd \
	rain_gauge_today

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/wind_dir \
	"Wind Direction" \
	"Wind Direction" \
	"${WEB_BASE_DIR}"/argent_80422.rrd \
	wind_dir_avg_2m \
	"" \
	"" \
	"" \
	"--lower-limit 0 --upper-limit 360 --rigid"

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/pressure_in \
	"Pressure (in)" \
	"Pressure (in)" \
	"${WEB_BASE_DIR}"/bmp180.rrd \
	pressure_in \
	"" \
	"" \
	"" \
	"--alt-autoscale --left-axis-format %2.2lf"

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/battery \
	"Voltages" \
	"Battery Charge (mV)" \
	"${WEB_BASE_DIR}"/battery.rrd \
	millivolts \
	"Booster (mV)" \
	"${WEB_BASE_DIR}"/booster.rrd \
	millivolts

