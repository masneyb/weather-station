#!/bin/bash -e

# download-weather-stats.sh - script to download a summary
#                             from the weather station and write the
#                             output to a file. You can use this
#                             output as part of your motd on a server.
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

# Example output:
#
# Weather: 76.28F, 52.9% humidity, Wind Avg: 3.15 mph, Wind Gust: 10.44 mph
#          Rain today: 0 in, Battery: 4023 mV, Uptime: 20.05 days

BASE_URL="${1:-}"
OUTFILE="${2:-}"

if [ "${BASE_URL}" = "" ] || [ "${OUTFILE}" == "" ] ; then
	echo "usage: $0 <base url> <output filename>"
fi

TMP=$(mktemp)

wget -q "${BASE_URL}"/temperature_humidity.json -O "${TMP}"

TEMPERATURE=$(jq .result[0].temperature < "${TMP}")
TEMPERATURE_UNIT=$(jq .result[0].temperature_unit < "${TMP}" | sed s/\"//g)
HUMIDITY=$(jq .result[0].humidity < "${TMP}")

wget -q "${BASE_URL}"/argent_80422.json -O "${TMP}"

#WIND_DIR_AVG=$(jq .result[0].wind_dir_avg_60m < "${TMP}")
WIND_SPEED_AVG=$(jq .result[0].wind_speed_avg_60m < "${TMP}")

#WIND_DIR_GUST=$(jq .result[0].wind_dir_gust_60m < "${TMP}")
WIND_SPEED_GUST=$(jq .result[0].wind_speed_gust_60m < "${TMP}")

RAIN_TODAY=$(jq .result[0].rain_gauge_today < "${TMP}")

wget -q "${BASE_URL}"/battery.json -O "${TMP}"

BATTERY=$(jq .result[0].millivolts < "${TMP}")

wget -q "${BASE_URL}"/uptime.json -O "${TMP}"

UPTIME_SECS=$(jq .result[0].uptime_secs < "${TMP}")
UPTIME_DAYS=$(bc <<< "scale=2; ${UPTIME_SECS} / 86400")

rm -f "${TMP}"

echo "Weather: ${TEMPERATURE}${TEMPERATURE_UNIT}," \
     "${HUMIDITY}% humidity," \
     "Wind Avg: ${WIND_SPEED_AVG} mph," \
     "Wind Gust: ${WIND_SPEED_GUST} mph" > "${OUTFILE}"

echo "        " \
     "Rain today: ${RAIN_TODAY} in," \
     "Battery: ${BATTERY} mV," \
     "Uptime: ${UPTIME_DAYS} days" >> "${OUTFILE}"

if [ "${BATTERY}" != "" ] && [ "${BATTERY}" -lt 3300 ] ; then
	echo "" >> "${OUTFILE}"
	echo "WARNING: BATTERY CHARGE LEVEL IS LOW!" >> "${OUTFILE}"

	echo "Battery charge level ${BATTERY} mV is low."
	exit 1
fi
