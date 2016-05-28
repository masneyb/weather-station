# pi-yadl - Yet Another Data Logger

An extensible data collector for the Raspberry Pi that supports returning the
data in JSON, YAML, CSV, XML, text, and RRD. RRDtool can be used to graph the
data over time.

Some sensors can occasionally return erratic readings that cause large spikes
or dips in your graphs. You can set valid thresholds to ignore these erroneous
readings. It also supports taking multiple samples to help smooth the results.

    usage: yadl --sensor <analog|digital See sensor options below>
    		--output <text|json|yaml|csv|xml|rrd>
    		[ --outfile <optional output filename. Defaults to stdout> ]
    		[ --num_results <# results returned (default 1)> ]
    		[ --sleep_usecs_between_results <usecs (default 0)> ]
    		[ --num_samples_per_result <# samples (default 1). See --filter for aggregation.> ]
    		[ --sleep_usecs_between_samples <usecs (default 0)> ]
    		[ --filter <median|mean|mode|min|max|range (default median)> ]
    		[ --remove_n_samples_from_ends <# samples (default 0)> ]
    		[ --max_retries <# retries (default 1)> ]
    		[ --sleep_usecs_between_retries <usecs (default 500000)> ]
    		[ --min_valid_value <minimum allowable value> ]
    		[ --max_valid_value <maximum allowable value> ]
    		[ --debug ]
    
    Supported Sensors
    
    * digital
      --gpio_pin <wiringPi pin #>
      	See http://wiringpi.com/pins/ to lookup the pin number.
    
    * analog
      --adc <see ADC list below>
    
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
    
    * Polling an analog sensor hooked up to channel 0 of a MCP3008
      $ yadl --sensor analog --adc mcp3008 --spi_channel 0 --analog_channel 0 --output csv --num_results 7 --sleep_usecs_between_results 50000
      timestamp,value
      1464465367,716.0
      1464465367,712.0
      1464465367,712.0
      1464465367,708.0
      1464465367,712.0
      1464465367,712.0
      1464465367,712.0
    
    * Polling BCM digital pin 17 (wiringPi pin 0) as JSON
      $ yadl --sensor digital --gpio_pin 0 --output json --num_results 1 --num_samples_per_result 1
      { "result": [ { "value": 0.0, "timestamp": 1464465651 } ] }

## Data sheets for the supported ADCs

* [MCP3002 ADC converter](http://ww1.microchip.com/downloads/en/DeviceDoc/21294C.pdf)
* [MCP3008 ADC converter](https://www.adafruit.com/datasheets/MCP3008.pdf)
* [PCF8591 ADC converter](http://www.nxp.com/documents/data_sheet/PCF8591.pdf)

## Single Node Installation

* `sudo apt-get install wiringpi rrdtool librrd-dev`
* `make`

