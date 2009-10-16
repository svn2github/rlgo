#!/bin/bash

#----------------------------------------------------------------------------#
# Submission script
# Play matches in sequence
NEWPATH=$1
PREFIX=$2
MATCHCMD=$3

SCRIPT=$NEWPATH/$PREFIX.sh
echo '#!/bin/bash' > $SCRIPT
echo "$MATCHCMD" >> $SCRIPT
chmod 744 $SCRIPT
$SCRIPT