# Solar Powered Weather Station

![Screenshot](images/weather-station-screenshot.png?raw=1)

## High level overview

- My [pi-yadl](https://github.com/masneyb/pi-yadl) project is used to gather and
  graph the data from the following types of sensors:
  - [Argent Data Systems Weather Sensor Assembly](https://www.sparkfun.com/products/8942)
    contains a wind vane, anemometer, and rain gauge. The wind speed average and gusts
    over the last 2 minute, 10 minute and 60 minute periods are logged. The amount of
    rain over the last 1 hour, 6 hours, 24 hours, and since midnight are logged.
  - Supports various types of temperature and humidity sensors: DHT11, DHT22,
    DS18B20, TMP36 (analog).
  - Supports the BMP180 pressure, altitude and temperature sensor.
  - Battery charge level is read via an analog to digital converter (ADC).
- For the wind vane, wind speed and rain gauge, the pi-yadl program is ran
  as a daemon in the background so that it can continuously monitor the rain
  gauge and anemometer via interrupts. The values from these three sensors are
  written out to a RRD database and JSON file every 30 seconds. The wind speed
  values are updated internally every second so that it can obtain the correct
  wind gusts.
- The other sensors are polled every 5 minutes via a systemd timer and written
  to various RRD databases and JSON files.
- The RRD databases are used to show the historical readings and are
  recreated at the beginning of each hour.
- The web page uses Javascript and JQuery to download the various JSON files to
  provide a dashboard showing the current sensor readings. The web page checks
  for updated JSON files on the server every 10 seconds.
- The nginx webserver runs on the Pi and only needs to serve out static
  content. Everything runs on the Pi; no third-party services are required.
- Optional ability to publish the sensor readings to Weather Underground.
  You can
  [view my weather station on Weather Underground](https://www.wunderground.com/personal-weather-station/dashboard?ID=KWVMORGA45).


## Hardware Information

![Complete Setup](images/weather-station-complete.jpg?raw=1)


### Project Box

I used
[this project box on Amazon](https://www.amazon.com/uxcell%C2%AE-Waterproof-Connect-Junction-200x120x75mm/dp/B00O9YY1G2),
although I am a little concerned about the quality of the seal on the box. The
solar panel is mounted to the lid of the project box and the panel is wider
than the box so it should also help to keep the water away from the seal.

All of the external sensors are terminated with a RJ45 connector to make it easy
to remove the project box without having to bring the entire weather station
inside. [RJ45 waterproof cable glands](https://www.adafruit.com/products/827)
are used on the project box to get the connections inside the box. The wire for
the solar panel enters the project box using a
[PG-9](https://www.adafruit.com/products/761) cable gland.

The box is mounted to the top of my fence using hose clamps with some rubber
stops that were purchased at a local hardware store.


### Solar and Power Setup

This weather station runs on a Raspberry Pi Zero running the latest Raspbian
Testing Lite. All of the hardware is powered by a
[6600mAH 3.7V lithium ion battery](https://www.adafruit.com/products/353)
that is [charged](https://www.adafruit.com/products/390) using a
[6V 9W solar panel](https://www.adafruit.com/products/2747). The 3.7V is
converted to 5V using a [PowerBoost 1000](https://www.adafruit.com/products/2465).
The solar panel is attached to the top of the project box using several large
pieces of Velcro. More information about the solar setup can be found on
[Adafruit's Website](https://learn.adafruit.com/usb-dc-and-solar-lipoly-charger/overview).
Be sure to connect the PowerBoost 1000 to the battery charge output pins; not to
the load terminal. This is because the solar panel can put out 6V however the
PowerBoost can only accept a maximum input voltage of 5.5V. See
[this post](https://forums.adafruit.com/viewtopic.php?f=19&t=59523) on the Adafruit
forums for more details. There should not be anything hooked up to the load
terminal on the charger. I fried a Pi Zero and a PowerBoost 1000 on a bright,
sunny afternoon with the PowerBoost hooked up to the load terminal.

To reduce the power usage of the Raspberry Pi, the LED and display on the Pi was
disabled. `powertop --auto-tune` was used to enable other power saving features.
See the files
[systemd/power-savings.service](systemd/power-savings.service) and
[bin/power-savings](bin/power-savings) for details. The power requirements
could be reduced even further by desoldering the various LEDs on the solar
charger and PowerBoost 1000.

I used a [USB Charger Doctor](https://www.adafruit.com/products/1852) to roughly
measure the power utilization of the entire weather station at 140 mAH with just
the wind / rain collector running in the background and 200 mAH when the main
collection processes runs every 5 minutes for just a few seconds. This is with a
USB WiFi dongle running the entire time. I would expect to get around 35 hours
of usage on a fully charged battery without any kind of backup from the solar
panel.

The end of the solar panel is terminated with one of
[these waterproof cables](https://www.adafruit.com/products/744) to make it
easy to detach the solar panel from the box. This would also make it easy to
run an extension cord outside and plug the unit in to charge without removing
or disassembling the project box if there were several days in a row of very
cloudy weather.

I mounted this [waterproof on / off switch](https://www.adafruit.com/products/917)
on the project box. Two wires from the switch go to the run pins on the
Raspberry Pi Zero.


### Sensors

A [MCP3008 analog to digital converter](https://www.adafruit.com/products/856)
was soldered onto a [solderable breadboard](https://www.sparkfun.com/products/12070).
The ADC is used for the wind vane and obtaining the voltage levels from the
battery and PowerBoost 1000. This ADC communicates with the Raspberry Pi
using the SPI bus.

The wind vane supports reporting 16 different positions by using a series of
reed switches and different size resistors. The wind vane is connected to the
ADC and the voltage indicates the direction. For example, according to the 
[data sheet](https://www.argentdata.com/files/80422_data sheet.pdf), 0 degrees (N)
is 3.84V; 45 degrees (NE) is 2.25V; and 90 degrees (E) is 0.45V. I had an issue
with getting accurate readings from the wind vane between 270 and 337.5 degrees
that was caused by having the reference ADC voltage in software set to 5V
instead of 5.1V when converting the value read from the ADC to millivolts.
Adding the argument `--adc_millivolts 5100` to the yadl binary fixed the issue.

The anemometer is hooked up to a GPIO pin on the Pi. According to the
[data sheet](https://www.argentdata.com/files/80422_data sheet.pdf), one click
of the reed switch over a second corresponds to a wind speed of 1.492 mph
(2.4 km/h). A pull down resistor is used and the switch is debounced in
software. One complete revolution of the anemometer will cause the pin to go
high twice.

The rain gauge is very similar to the anemometer. Each click of the switch
over a second corresponds to 0.011 inches (0.2794 mm) of rain according to
the [data sheet](https://www.argentdata.com/files/80422_data sheet.pdf).
This also uses a pull down resistor and the switch is debounced in
software.

The Stevenson screen (left side of the above picture) contains the temperature,
humidity and barometric pressure sensors. A 3D model of the screen was
downloaded from
[https://www.thingiverse.com/thing:158039](https://www.thingiverse.com/thing:158039),
3D printed using HIPS plastic and spray painted using flat white paint.

The temperature and humidity is obtained using a DHT22 sensor. The dew point is
calculated using the
[August-Roche-Magnus approximation](http://andrew.rsmas.miami.edu/bmcnoldy/Humidity.html).
The DHT sensor communicates with the Pi over one of the GPIO pins.

A BMP180 sensor is used to obtain the barometric pressure and communicates with
the Pi over the i2c bus. The BMP180 driver came from
[this project](https://github.com/lexruee/bmp180).

![Inside](images/weather-station-inside-box.jpg?raw=1)

![Inside](images/weather-station-breadboard.jpg?raw=1)

The [PIN_LAYOUTS.md](PIN_LAYOUTS.md) file contains the pin layouts of the cables
that leave the box.

![Inside](images/weather-station-outside-rj45-cable-glands.jpg?raw=1)

[![Weather Underground PWS KWVMORGA45](http://banners.wunderground.com/cgi-bin/banner/ban/wxBanner?bannertype=pws250&weatherstationcount=KWVMORGA45)](http://www.wunderground.com/weatherstation/WXDailyHistory.asp?ID=KWVMORGA45)


## Installation

- Note: This project iniitally started out using Raspian based on Debian
  Jessie, but the latest version of Raspbian Testing is required for the
  newer version of rrdtool that supports the `--left-axis-format` argument.
- Clone this repository and the [pi-yadl](https://github.com/masneyb/pi-yadl)
  repository.
- Follow the installation instructions to compile the pi-yadl project.
- Update the paths in this repository's [systemd service files](systemd/).
- Run `bin/create-rrds.sh <path to web/ directory>` to create the initial
  empty RRD databases.
- `sudo make install`
- `sudo apt-get install nginx`
- `sudo ln -s /path/to/web/directory /var/www/html/weather-station`


## Contact

Brian Masney <masneyb@onstation.org>
