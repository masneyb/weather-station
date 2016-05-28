#!/bin/bash -e

# create-graphs.sh
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

BASE_NAME="${1:-}"
DESCRIPTION="${2:-}"
if [ "${BASE_NAME}" = "" ] || [ "${DESCRIPTION}" = "" ] ; then
	echo "usage: $0 <path to RRD file without .rrd extension> <description or location for graph title>" >&2
	exit 1
fi

RRDFILE="${BASE_NAME}".rrd
INDEX_HTML="${BASE_NAME}.html"

create_graph()
{
	SCALE=$1
	OUTFILE=$2
	GRAPH_TITLE=$3

	# Create the graph
	rrdtool graph "${OUTFILE}" \
		--start "-${SCALE}" \
		--title "${GRAPH_TITLE}" \
		DEF:valuemax="${RRDFILE}":value:MAX \
		DEF:valuemin="${RRDFILE}":value:MIN \
		AREA:valuemax#FF0000:"Max Reading" \
		AREA:valuemin#FFFF00:"Min Reading" \
		GPRINT:valuemax:LAST:"Last Reading %2.1lf"

	# Add the graph to the HTML file
	BASENAME=$(basename "${OUTFILE}")
	cat >> "${INDEX_HTML}" << __EOF__
    <div class="graph"><img src="${BASENAME}"/></div>
__EOF__
}


# Write the header for the HTML file

cat > "${INDEX_HTML}" << __EOF__
<html>
<head>
  <meta http-equiv="cache-control" content="no-cache"/>
  <meta http-equiv="refresh" content="3600"/>
  <title>${DESCRIPTION}</title>
  <style>
    h2 { text-align: center; }
    .graph { width: 481px; float: left; padding: 10px; }
  </style>
</head>

<body>
  <h2>${DESCRIPTION}</h2>
  <div>
__EOF__


create_graph 4h "${BASE_NAME}_hourly.png" "${DESCRIPTION} - Hourly"
create_graph 1d "${BASE_NAME}_daily.png" "${DESCRIPTION} - Daily"
create_graph 1w "${BASE_NAME}_weekly.png" "${DESCRIPTION} - Weekly"
create_graph 1m "${BASE_NAME}_monthly.png" "${DESCRIPTION} - Monthly"
create_graph 1y "${BASE_NAME}_yearly.png" "${DESCRIPTION} - Yearly"


# Write the footer for the HTML file

cat >> "${INDEX_HTML}" << __EOF__
  </div>
</body>
</html>
__EOF__

