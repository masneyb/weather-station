# RJ45 wiring pin layouts

All of the external sensors are terminated with a RJ45 connector to make it easy
to remove the project box without having to bring all of the associated sensors
inside. [RJ45 waterproof cable glands](https://www.adafruit.com/products/827)
are used on the project box to get the connections inside the box. Inside the
project box, all of the wires are soldered onto a
[solderable breadboard](https://www.sparkfun.com/products/12070).

![Inside](images/weather-station-outside-rj45-cable-glands.jpg?raw=1)

## Wind / Rain Sensors Cable

The wind / rain sensors came with two separate RJ11 terminated cables. I combined
the two cables into a single RJ45 connector to allow using the RJ45 waterproof
cable glands.

Pin | Description
----|-------------
  1 | Ground
  2 | Wind speed signal - to BCM GPIO pin #18
  3 | +5V
  4 | Wind direction - to analog channel 0 on ADC
  5 | Unused
  6 | Unused
  7 | Rain gauge signal - to BCM GPIO pin #27
  8 | +5V

## Temperature / Humidity / Barometric Pressure Cable

Pin | Description
----|-------------
  1 | DHT Temperature / Humidity Signal
  2 | Unused
  3 | DHT Temperature / Humidity +3.3V
  4 | DHT Temperature / Humidity Ground
  5 | BMP180 I2C SCL
  6 | BMP180 I2C SDA
  7 | BMP180 +3.3V
  8 | Unused

