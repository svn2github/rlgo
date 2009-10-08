#!/bin/bash

#----------------------------------------------------------------------------#
# Run multiple versions of RLGO using various settings
if [ $# -lt 5 ]
then
    echo "Usage:"
    echo "multi-run.sh player path setting values games submit [options]"
    echo ""
    echo "player:   Tag used to identify player in the getprogram.sh script"
    echo "path:     Path where match data should be saved"
    echo "setting:  Setting to vary in player"
    echo "values:   List of values to try for setting, e.g. \"0.1 0.2 0.3\""
    echo "games:    Number of games to run"
    echo "submit:   Script used to play matches, listed below"
    echo "options:  Any set of options supported by RLGO"
    echo ""
    echo "submit-seq.sh:           Play matches sequentially"
    echo "submit-para.sh:          Play matches in parallel"
    echo "submit-host.sh hostfile: Submit matches to hosts listed in file"
    echo "submit-test.sh:          Output match commands without executing"
    exit 1
fi

SCRIPTDIR=`dirname $0`
PLAYER=$1
PATHSTEM=$2
SETTING=$3
VALUES=$4
GAMES=$5
SUBMIT=$6
shift; shift; shift; shift; shift; shift
OPTIONS=$@

DATAPATH="-DataPath ../data"
SELFPLAY="-SelfPlay SelfPlay -SelfPlay.MaxGames $GAMES"
COUNT=1

if [ ! -d $PATHSTEM ] 
then
    mkdir -p $PATHSTEM
fi

PIDPATH=$PATHSTEM/PID-LOG
touch $PIDPATH

# Generate jobs to run
for VALUE in $VALUES
do
    OVERRIDE="-$SETTING $VALUE"
    if echo $VALUE | grep -q '[^a-zA-Z0-9_./\-]'
    then
        PATHSUB=$COUNT
    else
        PATHSUB=$VALUE
    fi
    
    NEWPATH=$PATHSTEM/match-$PATHSUB
    if [ -d $NEWPATH ]
    then
        echo "Output path $NEWPATH already exists"
        exit
    fi
    OUTPUTPATH="-OutputPath $NEWPATH"
    PROGRAM=`$SCRIPTDIR/getprogram.sh $PLAYER $BOARDSIZE $OUTPUTPATH $DATAPATH $OVERRIDE $SELFPLAY $OPTIONS ` 
    echo "Program: $PROGRAM"
    echo "$SETTING = $VALUE"
    if [ "$PROGRAM" == "Unknown" ]
    then
        echo "Player $PLAYER is unknown"
        exit 1
    fi
    mkdir -p $NEWPATH
    PREFIX="script-$PATHSUB"
    MATCHCMD=$PROGRAM
    ANALYZECMD=":"
    $SUBMIT "$NEWPATH" "$PREFIX" "$MATCHCMD" "$ANALYZECMD" "$PIDPATH" "$COUNT"
    COUNT=$((COUNT+1))
done



