#!/bin/bash

/home/masneyb/data/pi-yadl/bin/yadl --sensor ds18b20 --w1_slave /sys/bus/w1/devices/28-0000075d50ee/w1_slave --temperature_unit fahrenheit --output rrd --outfile /home/masneyb/data/weather-station/web/temperature.rrd --output single_json --outfile /home/masneyb/data/weather-station/web/temperature.json

