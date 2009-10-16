#!/bin/bash

#----------------------------------------------------------------------------#
# Run a multi-process tournament between set of programs
if [ $# -lt 7 ]
then
    echo "Usage:"
    echo "tournament.sh numprocesses shortnames programnames path size minutes games nummatches submit"
    echo ""
    echo "numprocesses: Number of processes to execute"
    echo "shortnames:   File containing list of short names to label programs"
    echo "programnames: File containing list of program names to use"
    echo "path:         Path for matches played in tournament"
    echo "size:         Board size to use"
    echo "minutes:      Time control to use"
    echo "games:        Number of games to play in each match"
    echo "nummatches:   Number of matches to run in tournament"
    echo "submit:       Script used to play matches, listed below"
    echo ""
    echo "submit-seq.sh:               Play matches sequentially"
    echo "submit-para.sh:              Play matches in parallel"
    echo "\"submit-host.sh hostfile\": Submit matches to hosts listed in file"
    echo "submit-test.sh:              Output match commands without executing"
    exit 1
fi

SCRIPTDIR=`dirname $0`
NUMPROCS=$1
shift
ARGS=$@
echo $NUMPROCS
echo $ARGS

for ((PROC=1; PROC<=$NUMPROCS; ++PROC))
do
    $SCRIPTDIR/tournament.sh $ARGS $PROC &
done
