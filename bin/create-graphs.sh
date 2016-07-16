#!/bin/bash

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
	"Temperature and Humidity" \
	"Temperature" \
	"${WEB_BASE_DIR}"/temperature_humidity.rrd \
	temperature \
	"Relative Humidity" \
	"${WEB_BASE_DIR}"/temperature_humidity.rrd \
	humidity

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
	"Rain Gauge (in)" \
	"Rain Gauge (in)" \
	"${WEB_BASE_DIR}"/argent_80422.rrd \
	rain_gauge_cur

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/pressure_in \
	"Pressure (in)" \
	"Pressure (in)" \
	"${WEB_BASE_DIR}"/bmp180.rrd \
	pressure_in

"${CREATE_MIN_MAX_GRAPHS}" \
	"${WEB_BASE_DIR}"/battery \
	"Voltages" \
	"Battery Charge (mV)" \
	"${WEB_BASE_DIR}"/battery.rrd \
	millivolts \
	"Booster (mV)" \
	"${WEB_BASE_DIR}"/booster.rrd \
	millivolts

