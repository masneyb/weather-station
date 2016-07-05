#!/bin/bash

ARGENT_RRD=/home/masneyb/data/weather-station/web/argent_80422.rrd
if [ ! -d "${ARGENT_RRD}" ] ; then
	rrdtool create "${ARGENT_RRD}" \
		--start N --step 30 \
		DS:wind_dir_cur:GAUGE:600:U:U \
		DS:wind_speed_cur:GAUGE:600:U:U \
		DS:wind_dir_avg_2m:GAUGE:600:U:U \
		DS:wind_speed_avg_2m:GAUGE:600:U:U \
		DS:wind_dir_gust_2m:GAUGE:600:U:U \
		DS:wind_speed_gust_2m:GAUGE:600:U:U \
		DS:wind_dir_avg_10m:GAUGE:600:U:U \
		DS:wind_speed_avg_10m:GAUGE:600:U:U \
		DS:wind_dir_gust_10m:GAUGE:600:U:U \
		DS:wind_speed_gust_10m:GAUGE:600:U:U \
		DS:wind_dir_avg_60m:GAUGE:600:U:U \
		DS:wind_speed_avg_60m:GAUGE:600:U:U \
		DS:wind_dir_gust_60m:GAUGE:600:U:U \
		DS:wind_speed_gust_60m:GAUGE:600:U:U \
		DS:rain_gauge_cur:GAUGE:600:U:U \
		DS:rain_gauge_1h:GAUGE:600:U:U \
		DS:rain_gauge_6h:GAUGE:600:U:U \
		DS:rain_gauge_24h:GAUGE:600:U:U \
		RRA:MIN:0.5:1:120 \
		RRA:MIN:0.5:2:120 \
		RRA:MIN:0.5:4:120 \
		RRA:MIN:0.5:10:288 \
		RRA:MIN:0.5:20:1008 \
		RRA:MIN:0.5:60:1440 \
		RRA:MIN:0.5:80:3240 \
		RRA:MIN:0.5:100:5184 \
		RRA:MIN:0.5:120:8760 \
		RRA:MIN:0.5:240:8760 \
		RRA:MIN:0.5:360:8760 \
		RRA:MAX:0.5:1:120 \
		RRA:MAX:0.5:2:120 \
		RRA:MAX:0.5:4:120 \
		RRA:MAX:0.5:10:288 \
		RRA:MAX:0.5:20:1008 \
		RRA:MAX:0.5:60:1440 \
		RRA:MAX:0.5:80:3240 \
		RRA:MAX:0.5:100:5184 \
		RRA:MAX:0.5:120:8760 \
		RRA:MAX:0.5:240:8760 \
		RRA:MAX:0.5:360:8760
fi


