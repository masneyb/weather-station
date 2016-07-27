#!/bin/bash -e

# create-min-max-graphs.sh
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

BASE_OUTPUT_NAME="${1:-}"
DESCRIPTION="${2:-}"
RRD_FIELD1_LABEL="${3:-}"
RRD_FILE1="${4:-}"
RRD_FIELD1="${5:-}"
RRD_FIELD2_LABEL="${6:-}"
RRD_FILE2="${7:-}"
RRD_FIELD2="${8:-}"
OTHER_RRDGRAPH_ARGS="${9:-}"
if [ "${BASE_OUTPUT_NAME}" = "" ] || [ "${DESCRIPTION}" = "" ] ||
	[ "${RRD_FILE1}" = "" ] || [ "${RRD_FIELD1}" = "" ] ||
	[ "${RRD_FIELD1_LABEL}" = "" ] ; then
	echo "usage: $0 <base path of created resources> <description or location for graph title>" >&2
	echo "		<Y axis max. Set to -1 for autoscaling>" >&2
	echo "		<RRD field label 1> <RRD file 1> <RRD field name 1>" >&2
	echo "		[ <RRD field label 2> <RRD file 2> <RRD field name 2> ]" >&2
	exit 1
fi

INDEX_HTML="${BASE_OUTPUT_NAME}.html"

create_graph()
{
	SCALE=$1
	OUTFILE=$2
	GRAPH_TITLE=$3

	# Create the graph

	# Intentionally don't put ${OTHER_RRDGRAPH_ARGS} in quotes so
	# that multiple arguments can be passed to the function.

	if [ "${RRD_FIELD2}" = "" ] ; then
		rrdtool graph "${OUTFILE}" \
			--start "-${SCALE}" \
			--title "${GRAPH_TITLE}" \
			${OTHER_RRDGRAPH_ARGS} \
			"DEF:${RRD_FIELD1}max=${RRD_FILE1}:${RRD_FIELD1}:MAX" \
			"DEF:${RRD_FIELD1}min=${RRD_FILE1}:${RRD_FIELD1}:MIN" \
			"AREA:${RRD_FIELD1}max#FF0000:Max ${RRD_FIELD1_LABEL}" \
			"AREA:${RRD_FIELD1}min#FFFF00:Min ${RRD_FIELD1_LABEL}" \
			"GPRINT:${RRD_FIELD1}max:LAST:Last Reading %2.1lf"
	else
		rrdtool graph "${OUTFILE}" \
			--start "-${SCALE}" \
			--title "${GRAPH_TITLE}" \
			${OTHER_RRDGRAPH_ARGS} \
			"DEF:${RRD_FIELD1}1max=${RRD_FILE1}:${RRD_FIELD1}:MAX" \
			"DEF:${RRD_FIELD1}1min=${RRD_FILE1}:${RRD_FIELD1}:MIN" \
			"DEF:${RRD_FIELD2}2max=${RRD_FILE2}:${RRD_FIELD2}:MAX" \
			"DEF:${RRD_FIELD2}2min=${RRD_FILE2}:${RRD_FIELD2}:MIN" \
			"AREA:${RRD_FIELD1}1max#FF0000:Max ${RRD_FIELD1_LABEL}" \
			"AREA:${RRD_FIELD1}1min#FFFF00:Min ${RRD_FIELD1_LABEL}\j" \
			"LINE1:${RRD_FIELD2}2max#FF00FF:Max ${RRD_FIELD2_LABEL}" \
			"LINE1:${RRD_FIELD2}2min#0000FF:Min ${RRD_FIELD2_LABEL}\j" \
			"GPRINT:${RRD_FIELD2}2max:LAST:Last ${RRD_FIELD2_LABEL} %2.1lf" \
			"GPRINT:${RRD_FIELD1}1max:LAST:Last ${RRD_FIELD1_LABEL} %2.1lf\j"
	fi

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


create_graph 4h "${BASE_OUTPUT_NAME}_hourly.png" "${DESCRIPTION} - Hourly"
create_graph 1d "${BASE_OUTPUT_NAME}_daily.png" "${DESCRIPTION} - Daily"
create_graph 1w "${BASE_OUTPUT_NAME}_weekly.png" "${DESCRIPTION} - Weekly"
create_graph 1m "${BASE_OUTPUT_NAME}_monthly.png" "${DESCRIPTION} - Monthly"
create_graph 1y "${BASE_OUTPUT_NAME}_yearly.png" "${DESCRIPTION} - Yearly"


# Write the footer for the HTML file

cat >> "${INDEX_HTML}" << __EOF__
  </div>
</body>
</html>
__EOF__

