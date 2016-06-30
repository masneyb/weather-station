#!/bin/bash

/home/masneyb/data/pi-yadl/bin/create-min-max-graphs.sh \
	/home/masneyb/data/garden-sensors/web/argent_80422.rrd \
	wind_speed \
	/home/masneyb/data/garden-sensors/web/wind_speed \
	"Wind Speed (mph)"

/home/masneyb/data/pi-yadl/bin/create-min-max-graphs.sh \
	/home/masneyb/data/garden-sensors/web/argent_80422.rrd \
	wind_direction \
	/home/masneyb/data/garden-sensors/web/wind_direction \
	"Wind Direction (degrees)"

/home/masneyb/data/pi-yadl/bin/create-min-max-graphs.sh \
	/home/masneyb/data/garden-sensors/web/argent_80422.rrd \
	rain_gauge_cur \
	/home/masneyb/data/garden-sensors/web/rain_gauge \
	"Rain Gauge (in)"

/home/masneyb/data/pi-yadl/bin/create-min-max-graphs.sh \
	/home/masneyb/data/garden-sensors/web/temperature.rrd \
	temperature \
	/home/masneyb/data/garden-sensors/web/temperature \
	"Temperature (F)"
