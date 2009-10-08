#!/bin/bash

#----------------------------------------------------------------------------#
# Submission script
# Play matches in sequence
NEWPATH=$1
PREFIX=$2
MATCHCMD=$3
ANALYZECMD=$4

SCRIPT=$NEWPATH/$PREFIX.sh
echo '#!/bin/bash' > $SCRIPT
echo "$MATCHCMD" >> $SCRIPT
echo "$ANALYZECMD" >> $SCRIPT
chmod 744 $SCRIPT
$SCRIPT
