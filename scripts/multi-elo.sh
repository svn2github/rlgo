#!/bin/bash

#----------------------------------------------------------------------------#
# Play tournament between RLGO with various settings, 
# generate Elo ratings and plot results
if [ $# -lt 9 ]
then
    echo "Usage:"
    echo "multi-elo.sh player path setting values size minutes games matches title [options]"
    echo ""
    echo "player:   Tag used to identify player in the getprogram.sh script"
    echo "path:     Output path where match data should be saved"
    echo "setting1: Primary setting to vary in player"
    echo "values1:  Primary values to try for setting, e.g. \"0.1 0.2 0.3\""
    echo "setting2: Secondary setting to vary in player"
    echo "values2:  Secondary values to try for setting, e.g. \"0.1 0.2 0.3\""
    echo "size:     Board size to use"
    echo "matches:  Number of matches to play in the tournament"
    echo "title:    Title of the plot"
    echo "options:  Any set of options supported by RLGO"
    echo ""
    exit 1
fi

SCRIPTDIR=`dirname $0`
PLAYER=$1
PATHSTEM=$2
SETTING1=$3
VALUES1=$4
SETTING2=$5
VALUES2=$6
SIZE=$7
MATCHES=$8
TITLE=$9

shift; shift; shift; shift; shift; shift; shift; shift; shift
OPTIONS=$@

USEFUEGO=1 # Whether to include vanilla UCT players based on Fuego
NUMGNUGO=7 # Should set to the length of VALUES2
GNUGO=`$SCRIPTDIR/getprogram.sh gnugod`

if [ ! -d $PATHSTEM ] 
then
    mkdir -p $PATHSTEM
fi

for ((i=1; i<=NUMGNUGO; ++i))
do
    echo "GnuGoD" > $PATHSTEM/short-names.txt
    echo $GNUGO > $PATHSTEM/program-names.txt
done

# Generate players to run
COUNT1=1
for VALUE1 in $VALUES1
do
    if echo $VALUE1 | grep -q '[^a-zA-Z0-9_./\-]'
    then
        PATHSUB1=$COUNT1
    else
        PATHSUB1=$VALUE1
    fi

    if [ "$SETTING1" == "BoardSize" ]
    then
        SIZE=$VALUE1
    fi

    COUNT2=1
    for VALUE2 in $VALUES2
    do
        echo $SETTING1 $VALUE1 $SETTING2 $VALUE2
    
        if echo $VALUE2 | grep -q '[^a-zA-Z0-9_./\-]'
        then
            PATHSUB2=$COUNT2
        else
            PATHSUB2=$VALUE2
        fi

        if [ "$SETTING2" == "BoardSize" ]
        then
            SIZE=$VALUE2
        fi
    
        OVERRIDE="-$SETTING1 $VALUE1 -$SETTING2 $VALUE2"
        NEWPATH=$PATHSTEM/player-$PATHSUB1-$PATHSUB2
        if [ -d $NEWPATH ]
        then
            echo "Output path $NEWPATH already exists"
            exit
        fi
        mkdir -p $NEWPATH
        OUTPUTPATH="-OutputPath $NEWPATH"
        PROGRAM=`$SCRIPTDIR/getprogram.sh $PLAYER -BoardSize $SIZE $OUTPUTPATH $OVERRIDE $OPTIONS`
        NAME="$PLAYER""_$PATHSUB1"":$PATHSUB2"

        echo "$NAME" >> $PATHSTEM/short-names.txt
        echo "$PROGRAM" >> $PATHSTEM/program-names.txt
        
        if [ "$PROGRAM" == "Unknown" ]
        then
            echo "Player $PLAYER is unknown"
            exit 1
        fi

        COUNT1=$((COUNT+1))
    done
done

# Generate vanilla UCT players
if [ $USEFUEGO == 1 ] && [ $SETTING2 == "MaxGames" ]
then
for VALUE2 in $VALUES2
    do
        NAME="vanilla_UCT:$VALUE2"
        NEWPATH=$PATHSTEM/$NAME
        mkdir $NEWPATH
        echo $NAME
        echo $NAME >> $PATHSTEM/short-names.txt
        `$SCRIPTDIR/getprogram.sh fuego -config $NEWPATH/gtpconfig.cfg` >> $PATHSTEM/program-names.txt
        cat $SCRIPTDIR/../settings/vanilla-uct.cfg > $NEWPATH/gtpconfig.cfg
        echo "uct_param_player max_games $VALUE2" >> $NEWPATH/gtpconfig.cfg
    done
fi

# Run tournament and plot results
$SCRIPTDIR/tournament.sh $PATHSTEM/short-names.txt $PATHSTEM/program-names.txt $PATHSTEM $SIZE 0 2 $MATCHES submit-seq.sh 1
$SCRIPTDIR/analyze-tournament.sh $PATHSTEM/short-names.txt $PATHSTEM
$SCRIPTDIR/plot-tournament.sh $PATHSTEM $SETTING1 "$VALUES1" "$TITLE"
