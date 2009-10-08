#!/bin/bash

#----------------------------------------------------------------------------#
# Analyze multiple matches using variations on settings file
if [ $# -lt 3 ]
then
    echo "Usage:"
    echo "analyze.sh path setting values [elo]"
    echo ""
    echo "path:     Path where match data is saved"
    echo "setting:  Setting varied in player"
    echo "values:   List of values tried for setting"
    echo "elo:      Base Elo rating for comparison"
    exit 1
fi

SCRIPTDIR=`dirname $0`
PATHSTEM=$1
SETTING=$2
VALUES=$3
ELOBASE=0

if [ $# -eq 4 ]
then
    ELOBASE=$4
fi

echo -e "$SETTING\tGames\tWins\tCPU\tWins%\tError\tElo" > $PATHSTEM/results.txt

COUNT=1
for VALUE in $VALUES
do
    FILES=""
    OVERRIDE="-$SETTING $VALUE"
    if echo $VALUE | grep -q '[^a-zA-Z0-9_./\-]'
    then
        PATHSUB=$COUNT
    else
        PATHSUB=$VALUE
    fi

    NEWPATH=$PATHSTEM/match-$PATHSUB
    DATFILE=$NEWPATH/games.dat
    if [ -e $DATFILE ]
    then
        `$SCRIPTDIR/analyze-match.sh $NEWPATH`
        awk '$1 != "#" && $1 != "#GAME"' $DATFILE > $NEWPATH/filtered.dat
        FILES="$FILES $NEWPATH/filtered.dat"
        RESULTS=`awk '{cpu+=$8; games++} $4~/B+/ {wins++} END { pwins=wins/games; printf "%d\t%d\t%.1f\t%.1f\t%.1f", games, wins, cpu/games, 100*pwins, 100*sqrt(pwins*(1.0-pwins)/games) }' $FILES`
        WINS=`awk '{games++} $4~/B+/ {wins++} END { pwins=wins/games; print pwins }' $FILES`
        ELO=$((`$SCRIPTDIR/calc-elo.py $WINS` + ELOBASE))
        echo -e "$VALUE\t$RESULTS\t$ELO" >> $PATHSTEM/results.txt
    else
        echo -e "$VALUE" >> $PATHSTEM/results.txt
    fi
    COUNT=$((COUNT+1))
done

RESULTS=`awk '{rows++; games+=$2; wins+=$3; cpu+=$4} END { pwins=wins/games; printf "%d\t%d\t%.1f\t%.1f\t%.1f", games, wins, cpu/(rows-1), 100*pwins, 100*sqrt(pwins*(1.0-pwins)/games) }' $PATHSTEM/results.txt`
WINS=`awk '{games+=$2; wins+=$3} END { pwins=wins/games; print pwins }' $PATHSTEM/results.txt`
ELO=$((`python $SCRIPTDIR/calc-elo.py $WINS` + ELOBASE))
echo -e "Total\t$RESULTS\t$ELO" >> $PATHSTEM/results.txt

cat $PATHSTEM/results.txt
