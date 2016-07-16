# RJ45 wiring pin layouts

This is mostly useful for my future reference. It contains the pin layout of
the cables that connect to the outside of the project box.

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
  8 | BMP180 Ground

