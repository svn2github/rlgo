#!/bin/bash

#----------------------------------------------------------------------------#
# Generate elo table of tournament results
if [ $# -lt 2 ]
then
    echo "Usage:"
    echo "analyze-tournament.sh shortnames path"
    echo ""
    echo "shortnames:   File containing list of short names to label programs"
    echo "path:         Path of tournament"
    exit 1
fi

SHORTNAMES=$1
PATHSTEM=$2

awk '{print "addplayer " $1}' $SHORTNAMES > $PATHSTEM/eloscript.txt
for PROC in $PATHSTEM/process-*
do
    cat $PROC/results.txt >> $PATHSTEM/eloscript.txt
done

echo -n -e "elo\n" >> $PATHSTEM/eloscript.txt
echo -n -e "  mm\n" >> $PATHSTEM/eloscript.txt
echo -n -e "  ratings >$PATHSTEM/ratings.txt\n  x\nx" >> $PATHSTEM/eloscript.txt
bayeselo <$PATHSTEM/eloscript.txt
echo
echo "Bayesian Elo ratings"
cat $PATHSTEM/ratings.txt
