# pi-yadl - Yet Another Data Logger

An extensible analog and digital data collector for the Raspberry Pi that supports
returning the data in JSON, YAML, CSV, XML, text, and RRD. It also supports
counting the number of times that a digital pin closes, which is useful for
logging some types of sensors such as anemometer, rain gauge, etc.

Some sensors can occasionally return erratic readings that cause large spikes
or dips in your graphs. You can set valid thresholds to ignore these erroneous
readings. It also supports taking multiple samples to help smooth the results.
See the examples below in the usage for some use cases.

    usage: yadl --sensor <analog|digital|counter>
    		[ --gpio_pin <wiringPi pin #. Required for digital pins> ]
    		  See http://wiringpi.com/pins/ to lookup the pin number.
    		[ --adc <see ADC list below. Required for analog> ]
    		--output <text|json|yaml|csv|xml|rrd>
    		[ --outfile <optional output filename. Defaults to stdout> ]
    		[ --only_log_value_changes ]
    		[ --num_results <# results returned (default 1). Set to -1 to poll indefinitely.> ]
    		[ --sleep_usecs_between_results <usecs (default 0)> ]
    		[ --num_samples_per_result <# samples (default 1). See --filter for aggregation.> ]
    		[ --sleep_usecs_between_samples <usecs (default 0)> ]
    		[ --filter <median|mean|mode|min|max|range (default median)> ]
    		[ --remove_n_samples_from_ends <# samples (default 0)> ]
    		[ --max_retries <# retries (default 1)> ]
    		[ --sleep_usecs_between_retries <usecs (default 500000)> ]
    		[ --min_valid_value <minimum allowable value> ]
    		[ --max_valid_value <maximum allowable value> ]
    		[ --counter_poll_secs <seconds to poll each sample in counter mode (default 5)> ]
    		[ --counter_multiplier <multiplier to convert the requests per second to some other value. (default 1.0)> ]
    		[ --debug ]
    		[ --logfile <path to debug logs. Uses stderr if not specified.> ]
    		[ --daemon ]
    
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
    
    * Poll a single sample from BCM digital pin 17 (wiringPi pin 0) as JSON
      $ yadl --sensor digital --gpio_pin 0 --output json
      { "result": [ { "value": 0.0, "timestamp": 1464465651 } ] }
    
    * Poll 7 results from an analog sensor hooked up to channel 0 of a MCP3008.
      Wait 0.05 seconds between each result shown.
      $ yadl --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 0 \
    	--output csv --num_results 7 --sleep_usecs_between_results 50000
      reading_number,timestamp,value
      0,1464465367,716.0
      1,1464465367,712.0
      2,1464465367,712.0
      3,1464465367,708.0
      4,1464465367,712.0
      5,1464465367,712.0
      6,1464465367,712.0
    
    * Show 5 averaged results from a photoresistor hooked up to an ADC.
      1000 samples are taken for each result shown. 200 samples from each end
      are removed and the mean is taken of the middle 600 samples. This is
      useful for removing noise from analog sensors.
      $ yadl --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 0 \
    	--output csv --num_results 5 --sleep_usecs_between_results 2000000 \
    	--num_samples_per_result 1000 --remove_n_samples_from_ends 200 --filter mean
      reading_number,timestamp,value
      0,1464469264,779.4
      1,1464469267,778.7
      2,1464469269,779.7
      3,1464469271,779.8
      4,1464469273,779.2
    
    * Hook an anemometer (wind speed meter) up to a digital pin and count the
      number of times that the switch closes over a 5 second period. Multiply the
      requests per second by 1.492 to get the wind speed in miles per hour. Show
      5 different results.
      $ yadl --sensor counter --gpio_pin 1 --output csv --num_results 5 \
      	--counter_poll_secs 5 --counter_multiplier 1.492
      reading_number,timestamp,value
      0,1465084823,6.9
      1,1465084828,6.9
      2,1465084833,7.2
      3,1465084838,6.9
      4,1465084843,6.9
    
    * Hook a button up to a digital pin and check for bounce when the button
      is pressed. This polls the digital pin indefinitely until Crtl-C is
      pressed. Note: Newlines were added for clarity between the two button
      presses for illustration purposes.
      $ yadl --sensor digital --gpio_pin 0 --output csv --num_results -1 \
    	--only_log_value_changes
      reading_number,timestamp,value
      0,1464480347,0.0
    
      636673,1464480348,1.0
      687383,1464480348,0.0
    
      1678984,1464480351,1.0
      1731987,1464480351,0.0
      1731988,1464480351,1.0
      1732148,1464480351,0.0


## Using gnuplot to graph the data

Here is an example using the pi-yadl project to log the output from an analog
potentiometer hooked up to a 10-bit ADC.

    $ yadl --sensor analog --adc mcp3002 --spi_channel 0 --analog_channel 0 --num_results 7000 --sleep_usecs_between_results 500 --output csv --outfile data.csv

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


## Using RRDtool to graph the data over time

If you plan to graph one or more sensors over a long period of time, then you
may want to consider storing the data in a RRD database. See my
[pi-yatl](https://github.com/masneyb/pi-yatl) project for suggestions about how
to set this up.


## Single Node Installation

* `sudo apt-get install wiringpi librrd-dev`
* `make`


## Multinode Installation

See my [pi-yatl](https://github.com/masneyb/pi-yatl) project for suggestions
about how to set this up.


## Data sheets for the supported ADCs

* [MCP3002 ADC converter](http://ww1.microchip.com/downloads/en/DeviceDoc/21294C.pdf)
* [MCP3008 ADC converter](https://www.adafruit.com/datasheets/MCP3008.pdf)
* [PCF8591 ADC converter](http://www.nxp.com/documents/data_sheet/PCF8591.pdf)

