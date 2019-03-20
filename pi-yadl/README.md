# pi-yadl - Yet Another Data Logger

*Note: If I was starting this weather station project over, I would use the
[Linux Industrial I/O Subsystem](https://wiki.analog.com/software/linux/docs/iio/iio)
instead of coding this. This was a good learning exercise.*

An extensible analog and digital data collector for the Raspberry Pi with the
following features:

* Supports various types of sensors and pins:
  * Supports logging raw values from digital or analog pins.
  * Supports logging the number of times that a digital pin either goes high
    or low using interrupts. Some sensors, such as rain gauges and wind
    anemometers, use a reed switch and it is up to the microcontroller to count
    the number of times that the pin changes state.
  * Supports 4 different temperature and humidity sensors: DHT11, DHT22,
    DS18B20 and TMP36 (analog). Supports multiple temperature units.
  * Supports anemometer, wind direction, and rain gauage for the
    Argent Data Systems 80422 Wind / Rain sensors. Other types of similar
    sensors should be easily supported.
  * Supports BMP180 pressure, altitude and temperature sensor. The BMP180
    driver came from [this project](https://github.com/lexruee/bmp180).
  * Supports BME280 pressure, humidity, altitude and temperature sensor. The
    BME280 driver came from
    [this project](https://github.com/andreiva/raspberry-pi-bme280).
* Supports returning the data in JSON, YAML, CSV, XML, and RRD. RRDtool can
  be used to graph the data over time. It also supports writing multiple
  files from a single sensor reading. For example, you may want to write the
  readings to a RRD database and also to a JSON file inside your web root.
* Some sensors can occasionally return erratic readings that cause large spikes
  or dips in your graphs. You can optionally set valid thresholds to ignore
  these erroneous readings.
* Supports taking multiple samples to help smooth the results. This is
  particularly useful when reading from some types of analog sensors. For
  example, you can tell the program to take 1000 samples from a sensor, discard
  the 400 samples that are the outliers and return the mean of the remaining 600
  samples.


## Usage

    usage: yadl --sensor <digital|counter|analog|dht11|dht22|ds18b20|tmp36|bmp180|bme280|argent_80422>
    	--output <json|yaml|csv|xml|rrd|single_json> [ --output <...> ]
    	[ --outfile <optional output filename. Defaults to stdout> [ --outfile <...> ] ]
    	[ --only_log_value_changes ]
    	[ --num_results <# results returned (default 1). Set to -1 to poll indefinitely.> ]
    	[ --sleep_millis_between_results <milliseconds (default 0)> ]
    	[ --num_samples_per_result <# samples (default 1). See --filter for aggregation.> ]
    	[ --sleep_millis_between_samples <milliseconds (default 0)> ]
    	[ --filter <median|mean|mode|sum|min|max|range (default median)> ]
    	[ --remove_n_samples_from_ends <# samples (default 0)> ]
    	[ --max_retries <# retries (default 20)> ]
    	[ --sleep_millis_between_retries <milliseconds (default 500)> ]
    	[ --debug ]
    	[ --logfile <path to debug logs. Uses stderr if not specified.> ]
    	[ --daemon ]
    
    Sensor Specific Options
    
    * digital - Reads from a digital pin
    	--gpio_pin <wiringPi pin #. See http://wiringpi.com/pins/>
    
    * counter - Counts the number of times the digital pin state changes.
    	--gpio_pin <wiringPi pin #. See http://wiringpi.com/pins/>
    	[ --counter_multiplier <multiplier to convert the requests per second to some other value. (default 1.0)> ]
    	[ --interrupt_edge <rising|falling|both (default rising)> ]
    
    * analog
    	--adc <see ADC options below>
    
    * dht11 / dht22 - Temperature and Humidity sensors
    	--gpio_pin <wiringPi pin #. See http://wiringpi.com/pins/>
    	--temperature_unit <celsius|fahrenheit|kelvin|rankine>
    
    * ds18b20 - Temperature sensor that uses the Dallas 1-Wire protocol.
    	--w1_slave <w1 slave device. See /sys/bus/w1/devices/28-*/w1_slave>
    	--temperature_unit <celsius|fahrenheit|kelvin|rankine>
    
    	You need to have the w1-gpio and w1-therm kernel modules loaded.
    	You'll also need to have 'dtoverlay=w1-gpio' in your /boot/config.txt
    	and reboot if it was not already present.
    
    * tmp36 - Analog temperature sensor.
    	--adc <see ADC options below>
    	--temperature_unit <celsius|fahrenheit|kelvin|rankine>
    	[ --analog_scaling_factor <value> (default 500) ]
    
    * bmp180 - Temperature, pressure and altitude sensor.
    	--i2c_address <I2C hex address. Use i2cdetect command to look up.>
    	--temperature_unit <celsius|fahrenheit|kelvin|rankine>
    
    * bme280 - Temperature, humidity, pressure and altitude sensor.
    	--i2c_address <I2C hex address. Use i2cdetect command to look up.>
    	--temperature_unit <celsius|fahrenheit|kelvin|rankine>
    
    * argent_80422 - Wind vane, anemometer, and rain gauge.
    	--wind_speed_pin <wiringPi pin #.>
    	--wind_speed_unit <mph|kmh>
    	--rain_gauage_pin <wiringPi pin #.>
    	--rain_gauage_unit <in|mm>
    	--adc <see ADC options below. This is for the wind vane.>
    
    ADC Options and Supported Types
    	[ --adc_millivolts <value (default 3300)> ]
    	[ --adc_multiplier <value (default 1.0)> ]
    
    	The --adc_multiplier can be used if have a voltage divider and
    	and want to convert the reading back to the original value.
    
    * mcp3002 / mcp3004 / mcp3008 - 10-bit ADCs with a SPI interface.
    	--spi_channel <spi channel. Either 0 or 1 for the Pi.>
    	--analog_channel <analog channel>
    
    	You need to have the proper spi_bcmXXXX kernel module loaded on the Pi.
    
    * pcf8591 - 8-bit ADC with an I2C interface.
    	--i2c_address <I2C hex address. Use i2cdetect command to look up.>
    	--analog_channel <analog channel>
    
    Examples
    
    * Poll a DHT22 temperature sensor on BCM pin 17 (wiringPi pin 0) as JSON.
      $ sudo yadl --gpio_pin 0 --sensor dht22 --temperature_unit fahrenheit --output json
      { "result": [  { "temperature": 68.18, "humidity": 55.30, "dew_point": 51.55, "temperature_unit": "F", "timestamp": 1467648942 } ] }
    
    * Show 5 averaged results from an ADC. 1000 samples are taken for each result
      shown. 200 samples from each end are removed and the mean is taken of the middle
      600 samples. This is useful for removing noise from analog sensors. Wait 2 seconds
      between each result shown.
      $ yadl --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 0 \
    	--output csv --num_results 5 --sleep_millis_between_results 2000 \
    	--num_samples_per_result 1000 --remove_n_samples_from_ends 200 --filter mean
      reading_number,timestamp,reading,millivolts
      0,1465685840,96.0,309.0
      1,1465685842,96.0,309.0
      2,1465685844,96.0,309.0
      3,1465685846,96.0,309.0
      4,1465685848,96.0,309.0
    
    * See the files in the examples/ directory for more examples.
    
    * See systemd/pi-yadl-gatherer.service for an example writing the data to
      a RRD database.


## Examples

* See my [weather station project](https://github.com/masneyb/weather-station/) for
  an example polling the different types of sensors supported by this project.
* The [examples directory](examples/) shows several more examples of how to poll
  various types of sensors.


## Graphing

See the [GRAPHING.md](GRAPHING.md) file for some tips about ways to graph the data
over time.


## Single Node Installation

* `sudo apt-get install wiringpi librrd-dev libi2c-dev`
* `make`


## Multinode Installation

See [REMOTE_POLLING.md](REMOTE_POLLING.md) for some tips on how to set it up
so that you can publish the temperature and humidity on a webserver and graph the
data on a separate computer.


## Data sheets for the supported sensors

* [MCP3002 ADC converter](http://ww1.microchip.com/downloads/en/DeviceDoc/21294C.pdf)
* [MCP3008 ADC converter](https://www.adafruit.com/datasheets/MCP3008.pdf)
* [PCF8591 ADC converter](http://www.nxp.com/documents/data_sheet/PCF8591.pdf)
* [DHT11](http://www.micropik.com/PDF/dht11.pdf)
* [DHT22](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)
* [DS18B20](http://cdn.sparkfun.com/datasheets/Sensors/Temp/DS18B20.pdf)
* [TMP36 analog temperature sensor](http://cdn.sparkfun.com/datasheets/Sensors/Temp/TMP35_36_37.pdf)
* [Argent Data Systems Wind / Rain Sensor Assembly](http://www.sparkfun.com/datasheets/Sensors/Weather/Weather%20Sensor%20Assembly..pdf)
* [BMP180](http://cdn.sparkfun.com/datasheets/Sensors/Pressure/BMP180.pdf)

