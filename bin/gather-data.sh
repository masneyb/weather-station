#!/bin/bash

# gather-data.sh
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

YADL_BIN="${1:-}"
WEB_BASE_DIR="${2:-}"
ID="${3:-}"
PASSWORD="${4:-}"

if [ "${YADL_BIN}" = "" ] || [ "${WEB_BASE_DIR}" = "" ] ; then
        echo "usage: $0 <path to yadl binary> <path to web/ directory" \
		"[ <weather underground ID> <weather underground password> ]" >&2
        exit 1
fi

JSON="${WEB_BASE_DIR}"/temperature_humidity.json
"${YADL_BIN}" --gpio_pin 3 --sensor dht22 --temperature_unit fahrenheit --max_retries 30 \
	--output rrd --outfile "${WEB_BASE_DIR}"/temperature_humidity.rrd \
	--output single_json --outfile "${JSON}"
if [ $? != 0 ] ; then
	rm -f "${JSON}"
fi

JSON="${WEB_BASE_DIR}"/bmp180.json
"${YADL_BIN}" --sensor bmp180 --i2c_address 77 --temperature_unit fahrenheit \
	--num_samples_per_result 7 --filter median \
	--sleep_millis_between_samples 1000 \
	--output rrd --outfile "${WEB_BASE_DIR}"/bmp180.rrd \
	--output single_json --outfile "${JSON}"
if [ $? != 0 ] ; then
	rm -f "${JSON}"
fi

JSON="${WEB_BASE_DIR}"/battery.json
"${YADL_BIN}" --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 3 --adc_millivolts 5000 \
	--output rrd --outfile "${WEB_BASE_DIR}"/battery.rrd \
	--output single_json --outfile "${JSON}"
if [ $? != 0 ] ; then
	rm -f "${JSON}"
fi

JSON="${WEB_BASE_DIR}"/booster.json
"${YADL_BIN}" --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 2 --adc_millivolts 5000 \
	--output rrd --outfile "${WEB_BASE_DIR}"/booster.rrd \
	--output single_json --outfile "${JSON}"
if [ $? != 0 ] ; then
	rm -f "${JSON}"
fi


# Write system uptime JSON file
UPTIME_SECS=$(awk '{print $1}' < /proc/uptime)
NOW=$(date +%s)

cat > "${WEB_BASE_DIR}"/uptime.json << __EOF__
{ "result": [ { "uptime_secs": ${UPTIME_SECS}, "timestamp": ${NOW} } ] }
__EOF__


# Publish to weather underground
if [ "${ID}" != "" ] && [ "${PASSWORD}" != "" ] ; then
	/home/masneyb/data/weather-station/bin/weather-underground-publish.sh "${WEB_BASE_DIR}" \
		"${ID}" "${PASSWORD}"
fi

