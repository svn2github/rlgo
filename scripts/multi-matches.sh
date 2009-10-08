#!/bin/bash

#----------------------------------------------------------------------------#
# Play multiple matches using various settings
if [ $# -lt 9 ]
then
    echo "Usage:"
    echo "multi-play.sh player opponent path setting values size minutes games submit [options]"
    echo ""
    echo "player:   Tag used to identify player in the getprogram.sh script"
    echo "opponent: Tag used to identify opponent in the getprogram.sh script"
    echo "path:     Output path where match data should be saved"
    echo "setting:  Setting to vary in player"
    echo "values:   List of values to try for setting, e.g. \"0.1 0.2 0.3\""
    echo "size:     Board size to use"
    echo "minutes:  Time control to use (0 for no time control)"
    echo "games:    Number of games to play in each match"
    echo "submit:   Script used to play matches, listed below"
    echo "options:  Any set of options supported by RLGO"
    echo ""
    echo "submit-seq.sh:               Play matches sequentially"
    echo "submit-para.sh:              Play matches in parallel"
    echo "\"submit-host.sh hostfile\": Submit matches to hosts listed in file"
    echo "submit-test.sh:              Output match commands without executing"
    exit 1
fi

SCRIPTDIR=`dirname $0`
PLAYER=$1
OPPONENT=$2
PATHSTEM=$3
SETTING=$4
VALUES=$5
SIZE=$6
MINUTES=$7
GAMES=$8
SUBMIT=$9
shift; shift; shift; shift; shift; shift; shift; shift; shift
OPTIONS=$@

DATAPATH="-DataPath ../data"
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

    if [ "$SETTING" == "BoardSize" ]
    then
        SIZE=$VALUE
    fi
    
    NEWPATH=$PATHSTEM/match-$PATHSUB
    if [ -d $NEWPATH ]
    then
        echo "Output path $NEWPATH already exists"
        exit
    fi
    OUTPUTPATH="-OutputPath $NEWPATH"
    PROGRAM1=`$SCRIPTDIR/getprogram.sh $PLAYER -BoardSize $SIZE $OUTPUTPATH $DATAPATH $OVERRIDE $OPTIONS ` 
    PROGRAM2=`$SCRIPTDIR/getprogram.sh $OPPONENT `
    echo "Program1: $PROGRAM1"
    echo "Program2: $PROGRAM2"
    echo "$SETTING = $VALUE"
    if [ "$PROGRAM1" == "Unknown" ] || [ "$PROGRAM2" == "Unknown" ]
    then
        echo "Player $PLAYER is unknown"
        exit 1
    fi
    mkdir -p $NEWPATH
    PREFIX="script-$PATHSUB"
    MATCHCMD=`$SCRIPTDIR/match.sh "$NEWPATH" "$PROGRAM1" "$PROGRAM2" "$SIZE" "$MINUTES" "$GAMES"`
    ANALYZECMD=`$SCRIPTDIR/analyze-match.sh "$NEWPATH"`
    $SCRIPTDIR/$SUBMIT "$NEWPATH" "$PREFIX" "$MATCHCMD" "$ANALYZECMD" "$PIDPATH" "$COUNT"
    COUNT=$((COUNT+1))
done
