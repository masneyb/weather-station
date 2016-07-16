#!/bin/bash

BASE_DIR="${1:-}"
if [ "${BASE_DIR}" = "" ] ; then
	echo "usage: $0 <path to web/ directory>" >&2
	exit 1
fi

ARGENT_RRD="${BASE_DIR}"/argent_80422.rrd
TEMPERATURE_HUMIDTY_RRD="${BASE_DIR}"/temperature_humidity.rrd
BMP180_RRD="${BASE_DIR}"/bmp180.rrd
BATTERY_RRD="${BASE_DIR}"/battery.rrd
BOOSTER_RRD="${BASE_DIR}"/booster.rrd

if [ ! -f "${ARGENT_RRD}" ] ; then
	echo "Creating ${ARGENT_RRD}"
	rrdtool create "${ARGENT_RRD}" \
		--start N --step 30 \
		DS:wind_dir_cur:GAUGE:600:U:U \
		DS:wind_speed_cur:GAUGE:600:U:U \
		DS:wind_dir_avg_2m:GAUGE:600:U:U \
		DS:wind_speed_avg_2m:GAUGE:600:U:U \
		DS:wind_dir_gust_2m:GAUGE:600:U:U \
		DS:wind_speed_gust_2m:GAUGE:600:U:U \
		DS:wind_dir_avg_10m:GAUGE:600:U:U \
		DS:wind_speed_avg_10m:GAUGE:600:U:U \
		DS:wind_dir_gust_10m:GAUGE:600:U:U \
		DS:wind_speed_gust_10m:GAUGE:600:U:U \
		DS:wind_dir_avg_60m:GAUGE:600:U:U \
		DS:wind_speed_avg_60m:GAUGE:600:U:U \
		DS:wind_dir_gust_60m:GAUGE:600:U:U \
		DS:wind_speed_gust_60m:GAUGE:600:U:U \
		DS:rain_gauge_cur:GAUGE:600:U:U \
		DS:rain_gauge_1h:GAUGE:600:U:U \
		DS:rain_gauge_6h:GAUGE:600:U:U \
		DS:rain_gauge_24h:GAUGE:600:U:U \
		RRA:MIN:0.5:1:120 \
		RRA:MIN:0.5:2:120 \
		RRA:MIN:0.5:4:120 \
		RRA:MIN:0.5:10:288 \
		RRA:MIN:0.5:20:1008 \
		RRA:MIN:0.5:60:1440 \
		RRA:MIN:0.5:80:3240 \
		RRA:MIN:0.5:100:5184 \
		RRA:MIN:0.5:120:8760 \
		RRA:MIN:0.5:240:8760 \
		RRA:MIN:0.5:360:8760 \
		RRA:MAX:0.5:1:120 \
		RRA:MAX:0.5:2:120 \
		RRA:MAX:0.5:4:120 \
		RRA:MAX:0.5:10:288 \
		RRA:MAX:0.5:20:1008 \
		RRA:MAX:0.5:60:1440 \
		RRA:MAX:0.5:80:3240 \
		RRA:MAX:0.5:100:5184 \
		RRA:MAX:0.5:120:8760 \
		RRA:MAX:0.5:240:8760 \
		RRA:MAX:0.5:360:8760
fi

if [ ! -f "${TEMPERATURE_HUMIDTY_RRD}" ] ; then
	rrdtool create "${TEMPERATURE_HUMIDTY_RRD}" \
		--start N --step 300 \
		DS:temperature:GAUGE:600:U:U \
		DS:humidity:GAUGE:600:U:U \
		DS:dew_point:GAUGE:600:U:U \
		RRA:MIN:0.5:1:288 \
		RRA:MIN:0.5:12:720 \
		RRA:MIN:0.5:288:365 \
		RRA:MAX:0.5:1:288 \
		RRA:MAX:0.5:12:720 \
		RRA:MAX:0.5:288:365
fi

if [ ! -f "${BMP180_RRD}" ] ; then
	rrdtool create "${BMP180_RRD}" \
		--start N --step 300 \
		DS:temperature:GAUGE:600:U:U \
		DS:pressure_millibars:GAUGE:600:U:U \
		DS:pressure_in:GAUGE:600:U:U \
		DS:altitude:GAUGE:600:U:U \
		RRA:MIN:0.5:1:288 \
		RRA:MIN:0.5:12:720 \
		RRA:MIN:0.5:288:365 \
		RRA:MAX:0.5:1:288 \
		RRA:MAX:0.5:12:720 \
		RRA:MAX:0.5:288:365
fi

if [ ! -f "${BATTERY_RRD}" ] ; then
	rrdtool create "${BATTERY_RRD}" \
		--start N --step 300 \
		DS:reading:GAUGE:600:U:U \
		DS:millivolts:GAUGE:600:U:U \
		RRA:MIN:0.5:1:288 \
		RRA:MIN:0.5:12:720 \
		RRA:MIN:0.5:288:365 \
		RRA:MAX:0.5:1:288 \
		RRA:MAX:0.5:12:720 \
		RRA:MAX:0.5:288:365
fi

if [ ! -f "${BOOSTER_RRD}" ] ; then
	rrdtool create "${BOOSTER_RRD}" \
		--start N --step 300 \
		DS:reading:GAUGE:600:U:U \
		DS:millivolts:GAUGE:600:U:U \
		RRA:MIN:0.5:1:288 \
		RRA:MIN:0.5:12:720 \
		RRA:MIN:0.5:288:365 \
		RRA:MAX:0.5:1:288 \
		RRA:MAX:0.5:12:720 \
		RRA:MAX:0.5:288:365
fi

