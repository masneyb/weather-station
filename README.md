# weather-station

A weather station for the Raspberry Pi that supports the following sensors:

- [Argent Data Systems Weather Sensor Assembly](https://www.sparkfun.com/products/8942) -
  Supports wind direction, wind speed, and rain gauge. Supports showing the wind speed
  average and gusts over 2 minute, 10 minute and 60 minute periods.
- Various types of temperature and humidity sensors.
- BMP180 temperature, pressure and altitude sensor.

My [pi-yadl](https://github.com/masneyb/pi-yadl) project is used to gather and
graph the data from these sensors. This weather-station project contains the systemd
services, systemd timers, and web page for the sensors.

## Screenshot

![Screenshot](images/weather-station-screenshot.png?raw=1)

## High level overview

- For the Wind direction, wind speed and rain gauge, the pi-yadl program is ran
  as a daemon continuously in the background so that it can monitor the rain
  gauge and anemometer. The values from these three sensors are
  written out to the RRD database web/argent_80422.rrd and the JSON file
  web/argent_80422.json every 30 seconds.
- The data from the temperature, humidity, pressure sensors is polled every 5
  minutes via a systemd timer. The readings are written out to a JSON file
  in the web root and to a RRD database to allow graphing the historical
  data.
- The RRD graphs are recreated at the start of each hour.

## Solar Powered

This weather station runs on a Raspberry Pi Zero. Everything is
powered by a
[4400mAH 3.7V lithium ion battery](https://www.adafruit.com/products/354)
that is [charged](https://www.adafruit.com/products/390) using a
[6V 9W solar panel](https://www.adafruit.com/products/2747). The 3.7V is
converted to 5V using a [PowerBoost 1000](https://www.adafruit.com/products/2465).
More information about the solar setup can be found on
[Adafruit's Website](https://learn.adafruit.com/usb-dc-and-solar-lipoly-charger/overview).

