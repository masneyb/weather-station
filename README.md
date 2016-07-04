# pi-yadl - Yet Another Data Logger

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

    usage: yadl --sensor <digital|counter|analog|dht11|dht22|ds18b20|tmp36|argent_80422>
    		[ --gpio_pin <wiringPi pin #. Required for digital sensors.> ]
    		  See http://wiringpi.com/pins/ to lookup the pin number.
    		--output <json|yaml|csv|xml|rrd|single_json> [ --output <...> ]
    		[ --outfile <optional output filename. Defaults to stdout> [ --outfile <...> ] ]
    		[ --only_log_value_changes ]
    		[ --num_results <# results returned (default 1). Set to -1 to poll indefinitely.> ]
    		[ --sleep_millis_between_results <milliseconds (default 0)> ]
    		[ --num_samples_per_result <# samples (default 1). See --filter for aggregation.> ]
    		[ --sleep_millis_between_samples <milliseconds (default 0)> ]
    		[ --filter <median|mean|mode|sum|min|max|range (default median)> ]
    		[ --remove_n_samples_from_ends <# samples (default 0)> ]
    		[ --max_retries <# retries (default 10)> ]
    		[ --sleep_millis_between_retries <milliseconds (default 500)> ]
    		[ --min_valid_value <minimum allowable value> ]
    		[ --max_valid_value <maximum allowable value> ]
    		[ --debug ]
    		[ --logfile <path to debug logs. Uses stderr if not specified.> ]
    		[ --daemon ]
    
    Counter specific options
    
    		[ --counter_multiplier <multiplier to convert the requests per second to some other value. (default 1.0)> ]
    		[ --interrupt_edge <rising|falling|both (default rising)> ]
    
    Analog specific options
    
    		--adc <see ADC list below. Required for analog>
    		[ --adc_millivolts <value (default 3300)> ]
    
    Argent 80422 specific options
    
    		--wind_speed_pin <wiringPi pin #.>
    		--rain_gauage_pin <wiringPi pin #.>
    		Analog specific options from above must also be specified for the wind direction.
    
    Temperature sensors specific options
    		--temperature_unit <celsius|fahrenheit|kelvin|rankine>
    
    * ds18b20 - This temperature sensor uses the Dallas 1-Wire protocol.
      --w1_slave <w1 slave device>
      	The w1 slave device will be one of the
      	/sys/bus/w1/devices/28-*/w1_slave files.
    
      	You need to have the w1-gpio and w1-therm kernel modules loaded.
      	You'll also need to have 'dtoverlay=w1-gpio' in your /boot/config.txt
      	and reboot if it was not already present.
    
    * tmp36 - Analog temperature sensor.
      [ --analog_scaling_factor <value> (default 500) ]
    
    Supported Analog to Digital Converters (ADCs)
    
    * mcp3002 / mcp3004 / mcp3008 - 10-bit ADCs with a SPI interface.
      --spi_channel <spi channel>
      	The SPI channel is either 0 or 1 for the Raspberry Pi.
    
      --analog_channel <analog channel>
    
      You need to have the proper spi_bcmXXXX kernel module loaded on the Pi.
    
    * pcf8591 - 8-bit ADC with an I2C interface.
      --i2c_address <I2C hex address>
      	Use i2cdetect to scan your bus. 48 is the default if you have a single board.
    
      --analog_channel <analog channel>
    
    Examples
    
    * Poll a DHT22 temperature sensor on BCM pin 17 (wiringPi pin 0) as JSON.
      $ sudo yadl --gpio_pin 0 --sensor dht22 --temperature_unit fahrenheit --output json
      { "result": [  { "temperature": 68.18, "humidity": 55.30, "dew_point": 51.55, "timestamp": 1467648942 } ] }
    
    * Poll a single sample from BCM digital pin 17 (wiringPi pin 0) as JSON
      $ yadl --sensor digital --gpio_pin 0 --output json
      { "result": [ { "pin_state": 0.0, "timestamp": 1464465651 } ] }
    
    * Poll 7 results from an analog sensor hooked up to channel 0 of a MCP3008.
      Wait 50 milliseconds between each result shown.
      $ yadl --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 0 \
    	--output csv --num_results 7 --sleep_millis_between_results 50
      reading_number,timestamp,reading,millivolts
      0,1465685742,96.0,309.0
      1,1465685742,96.0,309.0
      2,1465685742,96.0,309.0
      3,1465685742,96.0,309.0
      4,1465685742,92.0,296.0
      5,1465685742,96.0,309.0
      6,1465685742,96.0,309.0
    
    * Show 5 averaged results from an ADC. 1000 samples are taken for each result
      shown. 200 samples from each end are removed and the mean is taken of the middle
      600 samples. This is useful for removing noise from analog sensors.
      $ yadl --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 0 \
    	--output csv --num_results 5 --sleep_millis_between_results 2000 \
    	--num_samples_per_result 1000 --remove_n_samples_from_ends 200 --filter mean
      reading_number,timestamp,reading,millivolts
      0,1465685840,96.0,309.0
      1,1465685842,96.0,309.0
      2,1465685844,96.0,309.0
      3,1465685846,96.0,309.0
      4,1465685848,96.0,309.0
    
    * Hook a button up to a digital pin and check for bounce when the button
      is pressed. This polls the digital pin indefinitely until Crtl-C is
      pressed. Note: Newlines were added for clarity between the two button
      presses for illustration purposes.
      $ yadl --sensor digital --gpio_pin 0 --output csv --num_results -1 \
    	--only_log_value_changes
      reading_number,timestamp,pin_state
      0,1464480347,0.0
    
      636673,1464480348,1.0
      687383,1464480348,0.0
    
      1678984,1464480351,1.0
      1731987,1464480351,0.0
      1731988,1464480351,1.0
      1732148,1464480351,0.0
    
    * See systemd/pi-yadl-gatherer.service for an example writing the data to
      a RRD database.


## Using RRDtool to graph the data

If you plan to graph one or more sensors over a long period of time, then
you may want to consider storing the database in a RRD database.

![Screenshot](images/pi-yadl-screenshot.png?raw=1)


## Using gnuplot to graph the data

Here is an example using the pi-yadl project to log the output from an analog
potentiometer hooked up to a 10-bit ADC.

    $ yadl --sensor analog --adc mcp3002 --spi_channel 0 --analog_channel 0 --num_results 7000 \
    				--sleep_usecs_between_results 500 --output csv --outfile data.csv

The associated [gnuplot](http://www.gnuplot.info/) script.

    set datafile separator ","
    set autoscale
    set yrange [-10:1100]
    set xtic auto
    set ytic auto
    set title "Logging an analog potentiometer using the pi-yadl project"
    set xlabel "Sample Number"
    set ylabel "ADC Reading"
    plot 'data.csv' using 1:3 with lines

![Example using GNUplot to log the data](images/pi-yadl-analog-pot-example.png?raw=1)


## Single Node Installation

* `sudo apt-get install wiringpi librrd-dev`
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
* [Argent Data Systems Wind / Rain Sensor Assembly](https://www.argentdata.com/files/80422_datasheet.pdf)

