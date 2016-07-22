#!/bin/bash -e

validate_timestamp()
{
	TS="${1}"
	# FIXME - validate
}

display_number_in_url_field()
{
	FIELD="${1}"
	NUM="${2}"
	# FIXME - validate that this is a number
	echo -n "&${FIELD}=${NUM}"
}

generate_url()
{
	TMP=$(mktemp)

	jq -r '.result[]|[.temperature, .humidity, .dew_point, .timestamp]|@csv' < "${WEB_BASE_DIR}"/temperature_humidity.json > "${TMP}"

	TEMPF=$(awk -F, '{print $1}' < "${TMP}")
	HUMIDITY=$(awk -F, '{print $2}' < "${TMP}")
	DEWPTF=$(awk -F, '{print $3}' < "${TMP}")
	TS=$(awk -F, '{print $4}' < "${TMP}")

	validate_timestamp "${TS}"


	jq -r '.result[]|[.pressure_in, .timestamp]|@csv' < "${WEB_BASE_DIR}"/bmp180.json > "${TMP}"

	BAROMIN=$(awk -F, '{print $1}' < "${TMP}")
	TS=$(awk -F, '{print $2}' < "${TMP}")

	validate_timestamp "${TS}"


	jq -r '.result[]|[.rain_gauge_1h, .wind_dir_cur, .wind_speed_cur, .wind_speed_avg_2m, .wind_dir_avg_2m, .wind_speed_gust_10m, .wind_dir_gust_10m, .timestamp]|@csv' < "${WEB_BASE_DIR}"/argent_80422.json > "${TMP}"
	
	RAININ=$(awk -F, '{print $1}' < "${TMP}")
	WINDDIR=$(awk -F, '{print $2}' < "${TMP}")
	WINDSPEEDMPH=$(awk -F, '{print $3}' < "${TMP}")
	WINDSPDMPH_AVG2M=$(awk -F, '{print $4}' < "${TMP}")
	WINDDIR_AVG2M=$(awk -F, '{print $5}' < "${TMP}")
	WINDGUSTMPH_10M=$(awk -F, '{print $6}' < "${TMP}")
	WINDGUSTDIR_10M=$(awk -F, '{print $7}' < "${TMP}")
	TS=$(awk -F, '{print $8}' < "${TMP}")

	validate_timestamp "${TS}"

	echo -n "${BASE_URL}?softwaretype=https%3A%2F%2Fgithub.com%2Fmasneyb%2Fweather-station&dateutc=now&action=updateraw&ID=${ID}&PASSWORD=${PASSWORD}"
	display_number_in_url_field tempf "${TEMPF}"
	display_number_in_url_field humidity "${HUMIDITY}"
	display_number_in_url_field dewptf "${DEWPTF}"
	display_number_in_url_field baromin "${BAROMIN}"
	display_number_in_url_field rainin "${RAININ}"
	display_number_in_url_field winddir "${WINDDIR}"
	display_number_in_url_field windspeedmph "${WINDSPEEDMPH}"
	display_number_in_url_field windspdmph_avg2m "${WINDSPDMPH_AVG2M}"
	display_number_in_url_field winddir_avg2m "${WINDDIR_AVG2M}"
	display_number_in_url_field windgustmph_10m "${WINDGUSTMPH_10M}"
	display_number_in_url_field windgustdir_10m "${WINDGUSTDIR_10M}"
	echo ""
}

WEB_BASE_DIR="${1:-}"
ID="${2:-}"
PASSWORD="${3:-}"
BASE_URL="${4:-}"

if [ "${WEB_BASE_DIR}" = "" ] || [ "${ID}" = "" ] || [ "${PASSWORD}" = "" ] ; then
        echo "usage: $0 <path to web/ directory" >&2
        exit 1
fi

if [ "${BASE_URL}" = "" ] ; then
	BASE_URL="http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php"
fi

URL=$(generate_url)

echo "Generated URL ${URL}"

curl "${URL}"

