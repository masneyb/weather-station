#!/bin/bash -e

# weather-underground-publish.sh
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

validate_timestamp()
{
	TS="${1}"
	NOW=$(date +%s)
	DIFF=$((NOW - TS))
	if [ "${DIFF}" -gt 300 ] ; then
		echo "Timestamp ${TS} is ${DIFF} seconds from now and is too old to publish." >&2
		return 1
	elif [ "${DIFF}" -lt 0 ] ; then
		echo "Timestamp ${TS} is ${DIFF} seconds in the future and cannot be published." >&2
		return 1
	fi

	return 0
}

display_number_in_url_field()
{
	FIELD="${1}"
	NUM="${2}"

	if [[ ! "${NUM}" =~ ^[0-9]+(\.[0-9]+){0,1}$ ]] ; then
		echo "${NUM} is not a valid number for field ${FIELD}" >&2
		exit 1
	fi

	echo -n "&${FIELD}=${NUM}"
}

generate_url()
{
	TMP=$(mktemp)

	# See http://wiki.wunderground.com/index.php/PWS_-_Upload_Protocol
	# for the supported URL parameters.

	echo -n "${BASE_URL}?softwaretype=tinyurl.com%2Fhyft8kp&dateutc=now&action=updateraw&ID=${ID}&PASSWORD=${PASSWORD}"

	NUM_OK=0
	jq -r '.result[]|[.temperature, .humidity, .dew_point, .timestamp]|@csv' < "${WEB_BASE_DIR}"/temperature_humidity.json > "${TMP}"
	TEMPF=$(awk -F, '{print $1}' < "${TMP}")
	HUMIDITY=$(awk -F, '{print $2}' < "${TMP}")
	DEWPTF=$(awk -F, '{print $3}' < "${TMP}")
	TS=$(awk -F, '{print $4}' < "${TMP}")
	validate_timestamp "${TS}"
	if [ $? = 0 ] ; then
		NUM_OK=$((NUM_OK + 1))
		display_number_in_url_field tempf "${TEMPF}"
		display_number_in_url_field humidity "${HUMIDITY}"
		display_number_in_url_field dewptf "${DEWPTF}"
	else
		echo "Warning: Temperature / humidity readings are not current. Not sending these values." >&2
	fi

	jq -r '.result[]|[.pressure_in, .timestamp]|@csv' < "${WEB_BASE_DIR}"/bmp180.json > "${TMP}"
	BAROMIN=$(awk -F, '{print $1}' < "${TMP}")
	TS=$(awk -F, '{print $2}' < "${TMP}")
	validate_timestamp "${TS}"
	if [ $? = 0 ] ; then
		NUM_OK=$((NUM_OK + 1))
		display_number_in_url_field baromin "${BAROMIN}"
	else
		echo "Warning: Barometer reading is not current. Not sending this value." >&2
	fi

	jq -r '.result[]|[.rain_gauge_1h, .wind_dir_cur, .wind_speed_cur, .wind_speed_avg_2m, .wind_dir_avg_2m, .wind_speed_gust_10m, .wind_dir_gust_10m, .wind_speed_gust_2m, .wind_dir_gust_2m, .timestamp]|@csv' < "${WEB_BASE_DIR}"/argent_80422.json > "${TMP}"
	RAININ=$(awk -F, '{print $1}' < "${TMP}")
	WINDDIR=$(awk -F, '{print $2}' < "${TMP}")
	WINDSPEEDMPH=$(awk -F, '{print $3}' < "${TMP}")
	WINDSPDMPH_AVG2M=$(awk -F, '{print $4}' < "${TMP}")
	WINDDIR_AVG2M=$(awk -F, '{print $5}' < "${TMP}")
	WINDGUSTMPH_10M=$(awk -F, '{print $6}' < "${TMP}")
	WINDGUSTDIR_10M=$(awk -F, '{print $7}' < "${TMP}")
	WINDGUSTMPH_2M=$(awk -F, '{print $8}' < "${TMP}")
	WINDGUSTDIR_2M=$(awk -F, '{print $9}' < "${TMP}")
	TS=$(awk -F, '{print $10}' < "${TMP}")
	validate_timestamp "${TS}"
	if [ $? = 0 ] ; then
		NUM_OK=$((NUM_OK + 1))
		display_number_in_url_field rainin "${RAININ}"
		display_number_in_url_field winddir "${WINDDIR}"
		display_number_in_url_field windspeedmph "${WINDSPEEDMPH}"
		display_number_in_url_field windgustdir "${WINDGUSTDIR_2M}"
		display_number_in_url_field windgustmph "${WINDGUSTMPH_2M}"
		display_number_in_url_field windspdmph_avg2m "${WINDSPDMPH_AVG2M}"
		display_number_in_url_field winddir_avg2m "${WINDDIR_AVG2M}"
		display_number_in_url_field windgustmph_10m "${WINDGUSTMPH_10M}"
		display_number_in_url_field windgustdir_10m "${WINDGUSTDIR_10M}"
	else
		echo "Warning: Wind / rain readings are not current. Not sending these values." >&2
	fi

	echo ""

	rm -f "${TMP}"

	if [ "${NUM_OK}" = 0 ] ; then
		return 1
	fi
	return 0
}

WEB_BASE_DIR="${1:-}"
ID="${2:-}"
PASSWORD="${3:-}"
BASE_URL="${4:-}"

if [ "${WEB_BASE_DIR}" = "" ] || [ "${ID}" = "" ] || [ "${PASSWORD}" = "" ] ; then
        echo "usage: $0 <path to web/ directory> <wunderground ID> <wunderground password> [ <wunderground base URL> ]" >&2
        exit 1
fi

if [ "${BASE_URL}" = "" ] ; then
	BASE_URL="http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php"
fi

URL=$(generate_url)
if [ $? = 0 ] ; then
	echo "Generated URL ${URL}"
	curl "${URL}"
else
	echo "Error: No readings are current; not publishing to Weather Underground" >&2
fi

