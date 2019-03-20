#!/bin/bash -e

# log-remote-sensor.sh - Downloads a JSON file that was created by
# yadl on a remote server and logs the result to a local RRD database.
#
# Copyright (C) 2016 Brian Masney <masneyb@onstation.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

# See ../REMOTE_POLLING.md file for installation tips.

URL=${1:-}
RRD_DATABASE=${2:-}
ADD_RRD_SAMPLE_BIN=${3:-}

if [ "${URL}" = "" ] || [ "${RRD_DATABASE}" = "" ] || [ "${ADD_RRD_SAMPLE_BIN}" = "" ] ; then
	echo "usage: $0 <URL> <RRD database> <path to yadl-add-rrd-sample binary>" 1>&2
	exit 1
fi

TMP=$(mktemp)
curl --silent --output "${TMP}" "${URL}"

VALUE=$(jq .result[0].value < "${TMP}")

if [ "${VALUE}" = "" ] ; then
	echo "Received bad data from remote URL ${URL}. Leaving temporary file ${TMP} in place."
	exit 1
fi

"${ADD_RRD_SAMPLE_BIN}" --outfile "${RRD_DATABASE}" --value "${VALUE}"

rm -f "${TMP}"
