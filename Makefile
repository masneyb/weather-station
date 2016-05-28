YATL_C_DEPS=src/adc_mcp3002.c src/adc_mcp3004.c src/adc_pcf8591.c src/adcs.c \
	src/filters.c src/loggers.c src/outputters.c src/rrd_common.c \
	src/sensor_analog.c src/sensor_digital.c src/sensors.c src/yadl.c

YATL_ADD_RRD_SAMPLE_C_DEPS=src/loggers.c src/rrd_common.c \
	src/yadl-add-rrd-sample.c

YATL_BIN=bin/yadl
YATL_ADD_RRD_SAMPLE_BIN=bin/yadl-add-rrd-sample

.PHONY: all clean install shellcheck

all: ${YATL_BIN} ${YATL_ADD_RRD_SAMPLE_BIN}

${YATL_BIN}: ${YATL_C_DEPS} src/yadl.h
	gcc -g -Wall -Wextra -pedantic -std=c11 -o ${YATL_BIN} ${YATL_C_DEPS} -lwiringPi -lrrd

${YATL_ADD_RRD_SAMPLE_BIN}: ${YATL_ADD_RRD_SAMPLE_C_DEPS}
	gcc -g -Wall -Wextra -pedantic -std=c11 -o ${YATL_ADD_RRD_SAMPLE_BIN} ${YATL_ADD_RRD_SAMPLE_C_DEPS} -lrrd

clean:
	rm -f ${YATL_BIN} ${YATL_ADD_RRD_SAMPLE_BIN}

install:
	cp -v systemd/* /etc/systemd/system/
	systemctl daemon-reload

	systemctl start pi-yadl-gatherer.service
	systemctl start pi-yadl-gatherer.timer
	systemctl enable pi-yadl-gatherer.timer

	systemctl start pi-yadl-create-graphs.service
	systemctl start pi-yadl-create-graphs.timer
	systemctl enable pi-yadl-create-graphs.timer

shellcheck:
	shellcheck bin/create-graphs.sh
	shellcheck bin/log-remote-sensor.sh
