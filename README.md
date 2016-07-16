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
The solar panel is attached to the top of the project box using several large
pieces of velcrow. More information about the solar setup can be found on
[Adafruit's Website](https://learn.adafruit.com/usb-dc-and-solar-lipoly-charger/overview).
I used
[this project box on Amazon](https://www.amazon.com/uxcell%C2%AE-Waterproof-Connect-Junction-200x120x75mm/dp/B00O9YY1G2),
although I am a little concerned about the quality of the seal of the box. I
put a piece of tape around the whole box where the lip and box body meet to
help keep the wind-driven rain out.

A [MCP3008 analog to digital converter](https://www.adafruit.com/products/856)
was soldered onto the [solderable breadboard](https://www.sparkfun.com/products/12070).
The ADC is used for the wind direction, obtaining the voltage levels of the
battery and the PowerBoost 1000. This ADC communicates with the Raspberry Pi
using the SPI bus.

![Complete Setup](images/weather-station-complete.jpg?raw=1)

The Stevenson screen (left side of the above picture) contains the DHT22
temperature/humidity sensor and the BMP180 barometric pressure sensor. A 3D
model of the screen was downloaded from
[https://www.thingiverse.com/thing:158039](https://www.thingiverse.com/thing:158039)
and printed using HIPS plastic and spray painted using flat white paint.

![Inside](images/weather-station-inside-box.jpg?raw=1)

![Inside](images/weather-station-breadboard.jpg?raw=1)

The [PIN_LAYOUTS.md](PIN_LAYOUTS.md) file contains the pin layouts of the cables
that leave the box.
