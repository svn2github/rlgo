#!/bin/bash

#----------------------------------------------------------------------------#
# Submission script
# Submit matches to multiple hosts (submission: hostplay.sh $HOSTFILE)
HOSTFILE=$1 # list of hosts
NEWPATH=$2
PREFIX=$3
MATCHCMD=$4
COUNT=$5

NUMHOSTS=`wc -l $HOSTFILE | awk '{print $1}'`
HOSTNUM=$((COUNT % NUMHOSTS))
HOST=`head -$HOSTNUM $HOSTFILE | tail -1`
echo "Host: $HOST"

SCRIPT=$NEWPATH/$PREFIX.sh
echo '#!/bin/bash' > $SCRIPT
echo "cd ~/rlgo/scripts" >> $SCRIPT
echo "$MATCHCMD" >> $SCRIPT
chmod 744 $SCRIPT
ssh $HOST "nohup bash -c -l $SCRIPT" > $NEWPATH/$PREFIX.txt &
