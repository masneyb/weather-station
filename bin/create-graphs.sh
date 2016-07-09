#!/bin/bash

YADL_BIN_DIR=/home/masneyb/data/pi-yadl/bin
CREATE_MIN_MAX_GRAPHS="${YADL_BIN_DIR}"/create-min-max-graphs.sh
WEB_BASE_DIR=/home/masneyb/data/weather-station/web

"${YADL_BIN_DIR}"/create-temperature-humidity-graphs.sh \
	"${WEB_BASE_DIR}"/temperature_humidity "Temperature and Humidity"

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/argent_80422.rrd \
	wind_speed_cur \
	"${WEB_BASE_DIR}"/wind_speed \
	"Wind Speed (mph)"

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/argent_80422.rrd \
	wind_dir_cur \
	"${WEB_BASE_DIR}"/wind_direction \
	"Wind Direction (degrees)"

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/argent_80422.rrd \
	rain_gauge_cur \
	"${WEB_BASE_DIR}"/rain_gauge \
	"Rain Gauge (in)"

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/bmp180.rrd \
	pressure_in \
	"${WEB_BASE_DIR}"/pressure_in \
	"Pressure (in)"

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/battery.rrd \
	millivolts \
	"${WEB_BASE_DIR}"/battery \
	"Battery Charge (mV)"

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/booster.rrd \
	millivolts \
	"${WEB_BASE_DIR}"/booster \
	"Booster (mV)"

