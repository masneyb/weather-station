YADL_C_DEPS=src/adc_mcp3002.c src/adc_mcp3004.c src/adc_pcf8591.c src/adcs.c \
	src/filters.c src/float_list.c src/loggers.c src/outputters.c src/rrd_common.c \
	src/sensor_analog.c src/sensor_argent_80422.c src/sensor_digital.c \
	src/sensor_digital_counter.c src/sensor_temperature_dht.c \
	src/sensor_temperature_ds18b20.c src/sensor_temperature_tmp36.c \
	src/sensor_bmp180.c src/sensor_bme280.c src/sensors.c src/temperature_units.c \
	src/yadl.c

YADL_ADD_RRD_SAMPLE_C_DEPS=src/loggers.c src/rrd_common.c \
	src/yadl-add-rrd-sample.c

YADL_BIN=bin/yadl
YADL_ADD_RRD_SAMPLE_BIN=bin/yadl-add-rrd-sample

.PHONY: all clean install shellcheck

all: ${YADL_BIN} ${YADL_ADD_RRD_SAMPLE_BIN}

${YADL_BIN}: ${YADL_C_DEPS} src/yadl.h
	gcc -g -Wall -Wextra -pedantic -std=c11 -D_DEFAULT_SOURCE -D_BSD_SOURCE -o ${YADL_BIN} ${YADL_C_DEPS} -lwiringPi -lrrd -lm -lpthread

${YADL_ADD_RRD_SAMPLE_BIN}: ${YADL_ADD_RRD_SAMPLE_C_DEPS}
	gcc -g -Wall -Wextra -pedantic -std=c11 -o ${YADL_ADD_RRD_SAMPLE_BIN} ${YADL_ADD_RRD_SAMPLE_C_DEPS} -lrrd

clean:
	rm -f ${YADL_BIN} ${YADL_ADD_RRD_SAMPLE_BIN}

shellcheck:
	shellcheck bin/create-min-max-graphs.sh || true
	shellcheck bin/create-max-graphs.sh || true
	shellcheck bin/log-remote-sensor.sh
