# Solar Powered Weather Station

![Screenshot](images/weather-station-screenshot.png?raw=1)

## Supported Sensors

- [Argent Data Systems Weather Sensor Assembly](https://www.sparkfun.com/products/8942)
  contains a wind vane, anemometer, and rain gauge. This project supports logging the
  wind speed average and gusts over 2 minute, 10 minute and 60 minute periods. The
  rain over the last 1 hour, 6 hours and 24 hours are logged.
- Supports various types of temperature and humidity sensors: DHT11, DHT22, DS18B20,
  TMP36 (analog).
- Supports the BMP180 pressure, altitude and temperature sensor.
- Battery charge level is read via an analog to digital converter (ADC).

## High level overview

- My [pi-yadl](https://github.com/masneyb/pi-yadl) project is used to gather and
  graph the data from these sensors. This weather-station project only contains
  the systemd services, systemd timers, and web page for the various sensors.
- For the wind vane, wind speed and rain gauge, the pi-yadl program is ran
  as a daemon in the background so that it can monitor the rain gauge and
  anemometer. The values from these three sensors are written out to a RRD
  database and JSON file every 30 seconds.
- The data from the other sensors are polled every 5 minutes via a systemd
  timer and written to various RRD databases and JSON files.
- The RRD databases are used to show the historical readings and are
  recreated at the beginning of each hour.
- The web page uses Javascript to download the various JSON files to provide
  a dashboard showing the current weather readings. The web page checks for
  updated JSON files on the server every 10 seconds.
- The Apache webserver runs on the server and it only needs to serve out static
  files.


## Hardware Information

![Complete Setup](images/weather-station-complete.jpg?raw=1)

This weather station runs on a Raspberry Pi Zero running Rasbian Jessie Lite.
All of the hardware is powered by a
[4400mAH 3.7V lithium ion battery](https://www.adafruit.com/products/354)
that is [charged](https://www.adafruit.com/products/390) using a
[6V 9W solar panel](https://www.adafruit.com/products/2747). The 3.7V is
converted to 5V using a [PowerBoost 1000](https://www.adafruit.com/products/2465).
The solar panel is attached to the top of the project box using several large
pieces of velcrow. More information about the solar setup can be found on
[Adafruit's Website](https://learn.adafruit.com/usb-dc-and-solar-lipoly-charger/overview).
Be sure to connect the PowerBoost 1000 to the battery charge output pins instead
of to the load terminal. This is because the solar panel can put out 6V however the
PowerBoost can only accept a maximum input voltage of 5.5V. See
[this post](https://forums.adafruit.com/viewtopic.php?f=19&t=59523) on the Adafruit
forums for more details. There should not be anything hooked up to the load
terminal on the charger.

To reduce the power usage of the Raspberry Pi, the LED and display on the Pi was
disabled. `powertop --auto-tune` was used to enable other power saving features.
See the files
[systemd/power-savings.service](systemd/power-savings.service) and
[bin/power-savings](bin/power-savings) for details. The power requirements
could be reduced even further by desoldering the various LEDs on the solar
charger and PowerBoost 1000.

I used
[this project box on Amazon](https://www.amazon.com/uxcell%C2%AE-Waterproof-Connect-Junction-200x120x75mm/dp/B00O9YY1G2),
although I am a little concerned about the quality of the seal on the box. I
put a piece of tape around the box where the lid and body meet to help keep the
wind-driven rain out.

All of the external sensors are terminated with a RJ45 connector to make it easy
to remove the project box without having to bring the entire weather station
inside. [RJ45 waterproof cable glands](https://www.adafruit.com/products/827)
are used on the project box to get the connections inside the box.

A [MCP3008 analog to digital converter](https://www.adafruit.com/products/856)
was soldered onto a [solderable breadboard](https://www.sparkfun.com/products/12070).
The ADC is used for the wind vane and obtaining the voltage levels from the
battery and PowerBoost 1000. This ADC communicates with the Raspberry Pi
using the SPI bus.

The anemometer is hooked up to a GPIO pin on the Pi. According to the
[datasheet](https://www.argentdata.com/files/80422_datasheet.pdf), one click
of the reed switch over a second corresponds to a wind speed of 1.492 mph
(2.4 km/h). A pull down resistor is used and the switch is debounced in
software.

The rain gauge is very similiar to the anemometer. Each click of the switch
over a second corresponds to 0.011 inches (0.2794 mm) of rain according to
the [datasheet](https://www.argentdata.com/files/80422_datasheet.pdf).
The rain gauge is sensitive to movement and high winds sometimes cause the
switch to change state. This also uses a pull down resistor and the switch
is debounced in software.

The Stevenson screen (left side of the above picture) contains the temperature,
humidity and barometric pressure sensors. A 3D model of the screen was
downloaded from
[https://www.thingiverse.com/thing:158039](https://www.thingiverse.com/thing:158039).
It was 3D printed using HIPS plastic and spray painted using flat white paint.

The temperature / humidity is obtained using a DHT22 sensor. The dew point is
calculated using the
[August-Roche-Magnus approximation](http://andrew.rsmas.miami.edu/bmcnoldy/Humidity.html).
The DHT sensor communicates with the Pi over one of the GPIO pins.

A BMP180 sensor is used to obtain the barometric pressure and communicates with
the Pi over the i2c bus.

![Inside](images/weather-station-inside-box.jpg?raw=1)

![Inside](images/weather-station-breadboard.jpg?raw=1)

The [PIN_LAYOUTS.md](PIN_LAYOUTS.md) file contains the pin layouts of the cables
that leave the box.

![Inside](images/weather-station-outside-rj45-cable-glands.jpg?raw=1)


## Installation

- Clone this repository and the [pi-yadl](https://github.com/masneyb/pi-yadl)
  repository.
- Follow the installation instructions to compile the pi-yadl project.
- Update the paths in this repository's [systemd service files](systemd/).
- Run `bin/create-rrds.sh <path to web/ directory>` to create the initial
  empty RRD databases.
- `sudo make install`
- Symlink the [web/](web/) directory somewhere into your web root.

