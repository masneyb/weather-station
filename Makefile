.PHONY: all install shellcheck

all:

install:
	cp -v systemd/* /etc/systemd/system/
	systemctl daemon-reload

	systemctl start weather-station-gatherer.service
	systemctl start weather-station-gatherer.timer
	systemctl enable weather-station-gatherer.timer

	systemctl start weather-station-create-graphs.service
	systemctl start weather-station-create-graphs.timer
	systemctl enable weather-station-create-graphs.timer

	systemctl start argent-80422-gatherer.service
	systemctl enable argent-80422-gatherer.service

shellcheck:
	shellcheck bin/create-graphs.sh
	shellcheck bin/gather-data.sh
