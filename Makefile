.PHONY: all install shellcheck

all:

install:
	cp -v systemd/* /etc/systemd/system/
	systemctl daemon-reload

	systemctl start garden-sensors-gatherer.service
	systemctl start garden-sensors-gatherer.timer
	systemctl enable garden-sensors-gatherer.timer

	systemctl start garden-sensors-create-graphs.service
	systemctl start garden-sensors-create-graphs.timer
	systemctl enable garden-sensors-create-graphs.timer

	systemctl start argent-80422-gatherer.service
	systemctl enable argent-80422-gatherer.service

shellcheck:
	shellcheck bin/create-graphs.sh
	shellcheck bin/gather-data.sh
