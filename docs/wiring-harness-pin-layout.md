All of the wires from the sensors are soldered onto a
[solderable breadboard](https://www.sparkfun.com/products/12070).
Between the breadboard and the sensors, I use a RJ45 coupler to make
it easy to disconnect the sensors if needed. This is the pin layout
of that part of the wiring harness:

Pin | Description
----|-------------
  1 | +5V
  2 | Ground
  3 | Wind speed signal - to BCM GPIO pin #18
  4 | Rain gauge signal - to BCM GPIO pin #27
  5 | Wind direction - to analog channel 0 on ADC
  6 | W1 Temperature sensor - to BCM GPIO pin #4
  7 | Soil moisture sensor +5V - to BCM GPIO pin #17
  8 | Soil moisture sensor signal - to analog channel 1 on ADC

