# Remote polling

This page describes how to make the temperature and humidity available
on a webserver as JSON. A separate node can log and graph that data.


## Installation on the temperature gathering node

Some sensors, such as the DHT11 and DHT22, require root access. This
is a restriction in wiringPi since it needs to interact with /dev/mem.
At a high level, this process sets up a webserver and uses a systemd
timer to write a JSON file every 2 minutes in the webserver document
root.

* Install dependencies: `sudo apt-get install apache2 wiringpi librrd-dev`

* Setup a systemd timer to update the JSON file every 2 minutes.

  /etc/systemd/system/yadl-json-output.service

      [Unit]
      Description=YADL
      [Service]
      User=root
      Type=simple
      ExecStart=/home/masneyb/data/pi-yadl/bin/yadl --gpio_pin 0 --sensor dht22 --temperature_unit fahrenheit --output json --outfile /var/www/html/humiture.json

  /etc/systemd/system/yadl-json-output.timer

      [Unit]
      Description=YADL timer
      [Timer]
      OnCalendar=*:0/2
      [Install]
      WantedBy=multi-user.target

  Reload systemd and enable the services

      sudo systemctl daemon-reload
      sudo systemctl start yadl-json-output.service
      sudo systemctl start yadl-json-output.timer
      sudo systemctl enable yadl-json-output.timer

* Check that the /var/www/html/humiture.json exists and that it is being updated
  every 2 minutes. Also verify in your web browser that the URL
  http://IP_ADDRESS_TO_YOUR_PI/humiture.json returns the JSON.


## Installation on the graphing node

* Install dependencies: `sudo apt-get install librrd-dev rrdtool`

* The yadl-add-rrd-sample binary adds the temperature and humidity readings to a
  RRD database. It has the following syntax:

      usage: yadl-add-rrd-sample [ --debug ]
      		--outfile <path to RRD database>
      		--temperature <temperature>
      		--humidity <humidity>
      
      Note: A new RRD database will be created if it does not exist

* The bin/log-remote-sensor.sh shell script fetches the JSON file from
  webserver and logs it to the RRD database using the
  yadl-add-rrd-sample binary.

      usage: log-remote-sensor.sh <URL> <RRD database> <path to yadl-add-rrd-sample binary>

* Setup a systemd timer to log the remote readings to a RRD database.

  /etc/systemd/system/yadl-remote-readings.service

      [Unit]
      Description=YADL remote readings
      [Service]
      User=root
      Type=simple
      ExecStart=/home/masneyb/data/pi-yadl/bin/log-remote-sensor.sh http://IP_ADDRESS_TO_YOUR_PI/humiture.json /home/masneyb/data/pi-yadl/web/remote.rrd /home/masneyb/data/pi-yadl/bin/yadl-add-rrd-sample

  /etc/systemd/system/yadl-remote-readings.timer

      [Unit]
      Description=YADL remote readings timer
      [Timer]
      OnCalendar=*:0/5
      [Install]
      WantedBy=multi-user.target

  Reload systemd and enable the services

      sudo systemctl daemon-reload
      sudo systemctl start yadl-remote-readings.service
      sudo systemctl start yadl-remote-readings.timer
      sudo systemctl enable yadl-remote-readings.timer

* Setup a systemd timer to create the graphs every hour

  /etc/systemd/system/yadl-remote-graphs.service

      [Unit]
      Description=pi-yadl remote graphs
      [Service]
      User=masneyb
      Type=simple
      ExecStart=/home/masneyb/data/pi-yadl/bin/create-graphs.sh /home/masneyb/data/pi-yadl/web/remote "Graph Description"

  /etc/systemd/system/yadl-remote-graphs.timer

      [Unit]
      Description=pi-yadl remote graphs timer
      [Timer]
      OnCalendar=hourly
      [Install]
      WantedBy=multi-user.target

  Reload systemd and enable the services

      sudo systemctl daemon-reload
      sudo systemctl start yadl-remote-graphs.service
      sudo systemctl start yadl-remote-graphs.timer
      sudo systemctl enable yadl-remote-graphs.timer


## Plotting multiple sensors on the same graph

Once you have multiple RRD files on a single computer, you can use RRDtool to create
a combined graph if desired. This is left as an exercise for the reader. See the
`rrdtool graph` section of bin/create-graphs.sh for a starting point. Currently, this
project creates a separate HTML page for each of the sensors.

