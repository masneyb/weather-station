#!/bin/bash

YADL_BIN="${1:-}"
WEB_BASE_DIR="${2:-}"

if [ "${YADL_BIN}" = "" ] || [ "${WEB_BASE_DIR}" = "" ] ; then
        echo "usage: $0 <path to yadl binary> <path to web/ directory" >&2
        exit 1
fi

"${YADL_BIN}" --gpio_pin 3 --sensor dht22 --temperature_unit fahrenheit --output rrd --outfile "${WEB_BASE_DIR}"/temperature_humidity.rrd --output single_json --outfile "${WEB_BASE_DIR}"/temperature_humidity.json

"${YADL_BIN}" --sensor bmp180 --i2c_device /dev/i2c-1 --i2c_address 77 --temperature_unit fahrenheit --output rrd --outfile "${WEB_BASE_DIR}"/bmp180.rrd --output single_json --outfile "${WEB_BASE_DIR}"/bmp180.json

"${YADL_BIN}" --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 3 --adc_millivolts 5000 --output rrd --outfile "${WEB_BASE_DIR}"/battery.rrd --output single_json --outfile "${WEB_BASE_DIR}"/battery.json

"${YADL_BIN}" --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 2 --adc_millivolts 5000 --output rrd --outfile "${WEB_BASE_DIR}"/booster.rrd --output single_json --outfile "${WEB_BASE_DIR}"/booster.json
