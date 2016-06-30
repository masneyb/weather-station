#!/bin/bash

## Use a GPIO pin to power the soil moisture sensors to avoid corrosion
#SOIL_POWER_GPIO_PIN=17
#if [ ! -d /sys/class/gpio/gpio"${SOIL_POWER_GPIO_PIN}"/ ] ; then
#	echo "${SOIL_POWER_GPIO_PIN}" > /sys/class/gpio/export
#fi
#echo out > /sys/class/gpio/gpio"${SOIL_POWER_GPIO_PIN}"/direction
#echo 1 > /sys/class/gpio/gpio"${SOIL_POWER_GPIO_PIN}"/value
#sleep 0.5
#
#/home/masneyb/data/pi-yadl/bin/yadl --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 1 --output csv --num_samples 1 --num_results -1
#
#
## Power off soil sensors
#echo 0 > /sys/class/gpio/gpio"${SOIL_POWER_GPIO_PIN}"/value
#

#/home/masneyb/data/pi-yadl/bin/yadl --sensor argent_80422 --wind_speed_pin 1 --rain_gauge_pin 2 --adc mcp3008 --spi_channel 0 --analog_channel 0 --sleep_millis_between_results 5000 --num_results 1 --output rrd --outfile /home/masneyb/data/garden-sensors/web/argent_80422.rrd

/home/masneyb/data/pi-yadl/bin/yadl --sensor ds18b20 --w1_slave /sys/bus/w1/devices/28-0000075d50ee/w1_slave --temperature_unit fahrenheit --output rrd --outfile /home/masneyb/data/garden-sensors/web/temperature.rrd --output single_json --outfile /home/masneyb/data/garden-sensors/web/temperature.json


