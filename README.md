# weather-station

A weather station for the Raspberry Pi that supports the following sensors:

- [Argent Data Systems Weather Sensor Assembly](https://www.sparkfun.com/products/8942)
  contains sensors for the wind vane, wind speed, and rain gauge. This
  project supports logging the wind speed average and gusts over 2 minute, 10 minute
  and 60 minute periods; rain over the last 1 hour, 6 hour and 24 hours.
- Supports various types of temperature and humidity sensors (DHT11, DHT22, DS18B20,
  TMP36 analog).
- Supports the BMP180 pressure, altitude and temperature sensor.
- Battery charge level.

My [pi-yadl](https://github.com/masneyb/pi-yadl) project is used to gather and
graph the data from these sensors. This weather-station project only contains the
systemd services, systemd timers, and web page for the various sensors.

## Screenshot

![Screenshot](images/weather-station-screenshot.png?raw=1)

## High level overview

- For the wind vane, wind speed and rain gauge, the pi-yadl program is ran
  as a daemon continuously in the background so that it can monitor the rain
  gauge and anemometer. The values from these three sensors are
  written out to a RRD database and JSON file every 30 seconds.
- The data from the other sensors are polled every 5 minutes via a systemd
  timer. The readings are also written out to a RRD database and JSON file.
- The RRD databases are used to show the historical readings and are
  recreated at the beginning of each hour.
- The index.html uses Javascript to download the various JSON files to provide
  a dashboard showing the current weather readings.

## Hardware Information

![Complete Setup](images/weather-station-complete.jpg?raw=1)

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
was soldered onto a [solderable breadboard](https://www.sparkfun.com/products/12070).
The ADC is used for the wind vane and obtaining the voltage levels from the
battery and PowerBoost 1000. This ADC communicates with the Raspberry Pi
using the SPI bus.

The anemometer is hooked up to a GPIO pin on the Pi. According to the
[datasheet](https://www.argentdata.com/files/80422_datasheet.pdf), each click
of the reed switch corresponds to a wind speed of 1.492 MPH. A pull down
resistor is used and the switch is debounced in software.

The rain gauge is very similiar to the anemometer. Each click of the switch
corresponds to 0.011 inches of rain according to the
[datasheet](https://www.argentdata.com/files/80422_datasheet.pdf).
The rain gauge is very sensitive to movement and the high winds sometimes
causes the switch to activate.

The Stevenson screen (left side of the above picture) contains the temperature,
humidity and barometric pressure sensors. A 3D model of the screen was
downloaded from
[https://www.thingiverse.com/thing:158039](https://www.thingiverse.com/thing:158039).
It was 3D printed using HIPS plastic and spray painted using flat white paint.

The temperature / humidity is obtained using a the DHT22 sensor. The dew point is
calculated based on these two values.

A BMP180 sensor is used to obtain the barometric pressure. It also collects the
temperature and altitude, although I am not using these values.

![Inside](images/weather-station-inside-box.jpg?raw=1)

![Inside](images/weather-station-breadboard.jpg?raw=1)

The [PIN_LAYOUTS.md](PIN_LAYOUTS.md) file contains the pin layouts of the cables
that leave the box.

![Inside](images/weather-station-outside-rj45-cable-glands.jpg?raw=1)

