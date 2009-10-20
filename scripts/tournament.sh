#!/bin/bash

#----------------------------------------------------------------------------#
# Run a tournament between set of programs
if [ $# -lt 7 ]
then
    echo "Usage:"
    echo "tournament.sh shortnames programnames path size minutes games nummatches submit [process]"
    echo ""
    echo "shortnames:   File containing list of short names to label programs"
    echo "programnames: File containing list of program names to use"
    echo "path:         Path for matches played in tournament"
    echo "size:         Board size to use"
    echo "minutes:      Time control to use"
    echo "games:        Number of games to play in each match"
    echo "nummatches:   Number of matches to run in tournament"
    echo "submit:       Script used to play matches, listed below"
    echo "process:      Process number (optional)"
    echo ""
    echo "submit-seq.sh:               Play matches sequentially"
    echo "submit-para.sh:              Play matches in parallel"
    echo "\"submit-host.sh hostfile\": Submit matches to hosts listed in file"
    echo "submit-test.sh:              Output match commands without executing"
    exit 1
fi

SCRIPTDIR=`dirname $0`
SHORTNAMES=$1
PROGRAMNAMES=$2
PATHSTEM=$3
SIZE=$4
MINUTES=$5
NUMGAMES=$6
NUMMATCHES=$7
SUBMIT=$8
PROC=$9

if [ $PROC == "" ]
then
    PROC=0
fi

NUMPLAYERS=`awk 'END {print NR}' $SHORTNAMES`

mkdir -p $PATHSTEM
PIDPATH=$PATHSTEM/PID-LOG
touch $PIDPATH

# Initialise win file to all zeros
echo -n "" > $PATHSTEM/wins.txt
for ((i=0; i<NUMPLAYERS; ++i))
do
    SHORT=`awk "NR==$((i+1))"' { print }' $SHORTNAMES`
    echo -e -n "$SHORT\t0\n" >> $PATHSTEM/wins.txt
done

if [ -e $PATHSTEM/process-$PROC ]
then
    echo "Tournament data already exists in $PATHSTEM/process-$PROC"
    exit
fi

for ((i=0; i<NUMMATCHES; ++i))
do
    NEWPATH=$PATHSTEM/process-$PROC/match-$i
    mkdir -p $NEWPATH
    PB=$((RANDOM % NUMPLAYERS))
    #PW=$((RANDOM % NUMPLAYERS)) # For uniform random pairing
    PW=`python $SCRIPTDIR/pair.py $NUMPLAYERS $PB < $PATHSTEM/wins.txt` # For win-biased pairing
    BLACK=`awk "NR==$((PB+1))"' { print }' $PROGRAMNAMES`
    WHITE=`awk "NR==$((PW+1))"' { print }' $PROGRAMNAMES`
    BSHORT=`awk "NR==$((PB+1))"' { print }' $SHORTNAMES`
    WSHORT=`awk "NR==$((PW+1))"' { print }' $SHORTNAMES`
    echo "Playing match $((i+1)) of $NUMMATCHES between $BSHORT and $WSHORT ($NUMGAMES games)"
    MATCHCMD=`$SCRIPTDIR/match.sh "$NEWPATH" "$BLACK" "$WHITE" "$SIZE" "$MINUTES" "$NUMGAMES"`
    $SCRIPTDIR/$SUBMIT "$NEWPATH" "$PREFIX" "$MATCHCMD" "$i" "$PIDPATH"

    awk '$4~/B+/ {print "addresult '"${PW} ${PB}"' 0" } $4~/W+/ {print "addresult '"${PW} ${PB}"' 2" }' $NEWPATH/games.dat >> $PATHSTEM/process-$PROC/results.txt
    BWINS=`awk 'BEGIN {bwins=0} $4~/B+/ {bwins++} END {print bwins}' $NEWPATH/games.dat`
    WWINS=`awk 'BEGIN {wwins=0} $4~/W+/ {wwins++} END {print wwins}' $NEWPATH/games.dat`
    echo "Result: $BWINS-$WWINS"
    awk "{ if (NR==$((PB+1))) print \$1,\$2+$BWINS; else if (NR==$((PW+1))) print \$1,\$2+$WWINS; else print \$1,\$2 }" $PATHSTEM/wins.txt > $PATHSTEM/new-wins.txt
    cp $PATHSTEM/new-wins.txt $PATHSTEM/wins.txt
done
