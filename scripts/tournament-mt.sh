#!/bin/bash

#----------------------------------------------------------------------------#
# Run a multi-threaded tournament between set of programs
SCRIPTDIR=`dirname $0`
NUMTHREADS=$1
shift
ARGS=$@
echo $NUMTHREADS
echo $ARGS

for ((THREAD=1; THREAD<=$NUMTHREADS; ++THREAD))
do
    $SCRIPTDIR/tournament.sh $ARGS $THREAD &
done
