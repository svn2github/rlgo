#!/bin/bash

#----------------------------------------------------------------------------#
#$Id$
#$Source$
#----------------------------------------------------------------------------#

# Analyze the latest results of an experiment 
if [ $# -lt 4 ]
then
    echo "Usage:"
    echo "analyze-experiment.sh path setting values title"
    echo ""
    echo "path:     Path where match data should be saved"
    echo "setting:  Setting to vary in player"
    echo "values:   List of values to try for setting, e.g. \"0.1 0.2 0.3\""
    echo "title:    Title of the plot"
    exit 1
fi

SCRIPTDIR=`dirname $0`
PATHSTEM=$1
SETTING=$2
VALUES=$3
TITLE=$4

$SCRIPTDIR/analyze-tournament.sh $PATHSTEM/short-names.txt $PATHSTEM
$SCRIPTDIR/plot-tournament.sh $PATHSTEM $SETTING "$VALUES" "$TITLE"
