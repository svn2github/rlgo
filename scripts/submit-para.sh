#!/bin/bash

#----------------------------------------------------------------------------#
# Submission script
# Play matches in parallel
NEWPATH=$1
PREFIX=$2
MATCHCMD=$3
PIDFILE=$4

SCRIPT=$NEWPATH/$PREFIX.sh
echo '#!/bin/bash' > $SCRIPT
echo "$MATCHCMD &" >> $SCRIPT
echo " echo \"\$!\" >> ${PIDFILE:-\"$$.PID\"}" >> $SCRIPT
chmod 744 $SCRIPT
$SCRIPT &