SCRIPTDIR=`dirname $0`

TESTOPTIONS="-Simulator.MaxGames 100 -DefaultTime 1 -SafetyTime 0 -MaxDepth 3"
cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh tdlearn $TESTOPTIONS`
cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh tdsearch $TESTOPTIONS`
cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh dyna2 $TESTOPTIONS`
cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh hybrid $TESTOPTIONS`
cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh tourney $TESTOPTIONS`
cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh farm 2 tdsearch $TESTOPTIONS -MaxSize 2`
cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh tfarm 2 $TESTOPTIONS -MaxSize 2`
cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh gnugo0`
cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh gnugod`
cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh fuego`
#cat $SCRIPTDIR/test.gtp | eval `$SCRIPTDIR/getprogram.sh vanilla-uct`
