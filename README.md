# weather-station

A weather station for the Raspberry Pi that supports the following sensors:

- [Argent Data Systems Weather Sensor Assembly](https://www.sparkfun.com/products/8942) -
  Supports wind direction, wind speed, and rain gauge.
- [Waterproof DS18B20 Temperature Sensor](https://www.sparkfun.com/products/11050)

My [pi-yadl](https://github.com/masneyb/pi-yadl) project is used to gather and
graph the data from these sensors. This project contains the systemd services,
systemd timers, and web page for the sensors.

## High level overview

- For the Wind direction, wind speed and rain gauge, the pi-yadl program is ran
  as a daemon continuously in the background. It needs to constantly run so that
  it can monitor the rain gauge. The values from these three sensors are written
  out to the RRD database web/argent_80422.rrd and the JSON file
  web/argent_80422.json every 30 seconds.
- The data from the temperature sensor is polled every 5 minutes via a systemd
  timer and written to the RRD database web/temperature.rrd and JSON file
  web/temperature.json.
- The RRD graphs are recreated at the start of each hour.

## Screenshot

![Screenshot](images/weather-station-screenshot.png?raw=1)

