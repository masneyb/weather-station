# weather-station

A weather station for the Raspberry Pi that supports the following sensors:

- [Argent Data Systems Weather Sensor Assembly](https://www.sparkfun.com/products/8942)
  contains sensors for the wind direction, wind speed, and the amount of rain. This
  project supports logging the wind speed average and gusts over 2 minute, 10 minute
  and 60 minute periods; rain over the last 1 hour, 6 hour and 24 hours.
- Supports various types of temperature and humidity sensors (DHT11, DHT22, DS18B20,
  TMP36 analog).
- Supports the BMP180 temperature, pressure and altitude sensor.

My [pi-yadl](https://github.com/masneyb/pi-yadl) project is used to gather and
graph the data from these sensors. This weather-station project only contains the
systemd services, systemd timers, and web page for the sensors.

## Screenshot

![Screenshot](images/weather-station-screenshot.png?raw=1)

## High level overview

- For the wind direction, wind speed and rain gauge, the pi-yadl program is ran
  as a daemon continuously in the background so that it can monitor the rain
  gauge and anemometer. The values from these three sensors are
  written out to the RRD database web/argent_80422.rrd and the JSON file
  web/argent_80422.json every 30 seconds.
- The data from the other sensors are polled every 5 minutes via a systemd
  timer. The readings are written out to a JSON file in the web root and to
  a RRD database to allow graphing the historical data.
- The RRD graphs are recreated at the start of each hour.
- The index.html uses Javascript to download the various JSON files to provide
  a dashboard with the current weather readings.

## Hardware Information

This weather station runs on a Raspberry Pi Zero. All of the hardware is
powered by a
[4400mAH 3.7V lithium ion battery](https://www.adafruit.com/products/354)
that is [charged](https://www.adafruit.com/products/390) using a
[6V 9W solar panel](https://www.adafruit.com/products/2747). The 3.7V is
converted to 5V using a [PowerBoost 1000](https://www.adafruit.com/products/2465).
More information about the solar setup can be found on
[Adafruit's Website](https://learn.adafruit.com/usb-dc-and-solar-lipoly-charger/overview).

![Complete Setup](images/weather-station-complete.jpg?raw=1)

The Stevenson screen (left side of the above picture) contains the DHT11
temperature and BMP180 barometric pressure sensors. A 3D model of the screen
was downloaded from
[https://www.thingiverse.com/thing:158039](https://www.thingiverse.com/thing:158039)
and printed using HIPS plastic and spray painted using flat white paint.

![Inside](images/weather-station-inside-box.jpg?raw=1)

The [PIN_LAYOUTS.md](PIN_LAYOUTS.md) file contains the pin layouts of the cables
that leave the box.
