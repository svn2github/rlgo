#!/bin/bash

#----------------------------------------------------------------------------#
# Run a tournament between set of programs
if [ $# -lt 7 ]
then
    echo "Usage:"
    echo "tournament.sh shortnames programnames path size minutes games nummatches [thread]"
    echo ""
    echo "shortnames:   File containing list of short names to label programs"
    echo "programnames: File containing list of program names to use"
    echo "path:         Path for matches played in tournament"
    echo "size:         Board size to use"
    echo "minutes:      Time control to use"
    echo "games:        Number of games to play in each match"
    echo "nummatches:   Number of matches to run in tournament"
    echo "thread:       Thread number (optional)"
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
THREAD=$8

if [ $THREAD == ""]
then
    THREAD=0
fi

NUMPLAYERS=`awk 'END {print NR}' $SHORTNAMES`

if [ $MINUTES == 0 ]
then
    GAMETIME=""
else
    GAMETIME="-time $MINUTES"
fi

REFEREE=`$SCRIPTDIR/getprogram.sh gnugod`
mkdir -p $PATHSTEM

# Initialise win file to all zeros
echo -n "" > $PATHSTEM/wins.txt
for ((i=0; i<NUMPLAYERS; ++i))
do
    SHORT=`awk "NR==$((i+1))"' { print }' $SHORTNAMES`
    echo -e -n "$SHORT\t0\n" >> $PATHSTEM/wins.txt
done

if [ -e $PATHSTEM/thread-$THREAD ]
then
    echo "Tournament data already exists in $PATHSTEM/thread-$THREAD"
    exit
fi

for ((i=0; i<NUMMATCHES; ++i))
do
    NEWPATH=$PATHSTEM/thread-$THREAD/match-$i
    mkdir -p $NEWPATH
    PB=$((RANDOM % NUMPLAYERS))
    #PW=$((RANDOM % NUMPLAYERS)) # For uniform random pairing
    PW=`python $SCRIPTDIR/pair.py $NUMPLAYERS $PB < $PATHSTEM/wins.txt` # For win-biased pairing
    BLACK=`awk "NR==$((PB+1))"' { print }' $PROGRAMNAMES`
    WHITE=`awk "NR==$((PW+1))"' { print }' $PROGRAMNAMES`
    BSHORT=`awk "NR==$((PB+1))"' { print }' $SHORTNAMES`
    WSHORT=`awk "NR==$((PW+1))"' { print }' $SHORTNAMES`
    echo "Playing match $((i+1)) of $NUMMATCHES between $BSHORT and $WSHORT ($NUMGAMES games)"
    twogtp -black "$BLACK" -white "$WHITE" -referee "$REFEREE" -auto -alternate -komi 7.5 -size $SIZE $GAMETIME -games $NUMGAMES -maxmoves 200 -force -sgffile $NEWPATH/games
    awk '$4~/B+/ {print "addresult '"${PW} ${PB}"' 0" } $4~/W+/ {print "addresult '"${PW} ${PB}"' 2" }' $NEWPATH/games.dat >> $PATHSTEM/thread-$THREAD/results.txt
    BWINS=`awk 'BEGIN {bwins=0} $4~/B+/ {bwins++} END {print bwins}' $NEWPATH/games.dat`
    WWINS=`awk 'BEGIN {wwins=0} $4~/W+/ {wwins++} END {print wwins}' $NEWPATH/games.dat`
    echo "Result: $BWINS-$WWINS"
    awk "{ if (NR==$((PB+1))) print \$1,\$2+$BWINS; else if (NR==$((PW+1))) print \$1,\$2+$WWINS; else print \$1,\$2 }" $PATHSTEM/wins.txt > $PATHSTEM/new-wins.txt
    cp $PATHSTEM/new-wins.txt $PATHSTEM/wins.txt
done
