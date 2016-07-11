# How to show current and historical data on a website

If you plan to graph one or more sensors over a long period of time, then
you may want to consider using a RRD database.

![Screenshot](images/pi-yadl-screenshot.png?raw=1)

My [weather station](https://github.com/masneyb/weather-station) project writes the
current values to a JSON file inside the web root and to a RRD database so that the
values over time can be graphed with these arguments:

    `yadl ... --output rrd --outfile outfile.rrd --output single_json --outfile outfile.json`


# Using gnuplot to graph the data

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

