SCRIPTDIR=`dirname $0`

RLGO=`$SCRIPTDIR/getprogram.sh localshape` 
GNUGO0=`$SCRIPTDIR/getprogram.sh gnugo0`

mkdir -p test-matches
eval `$SCRIPTDIR/match.sh test-matches "$RLGO" "$GNUGO0" 9 0 10`
eval `$SCRIPTDIR/analyze-match.sh test-matches`
$SCRIPTDIR/multi-matches.sh tdlearn gnugo0 test-matches/para-matches MaxSize "1 2 3" 9 0 10 submit-para.sh
$SCRIPTDIR/multi-matches.sh tdlearn gnugo0 test-matches/seq-matches MaxSize "1 2 3" 9 0 10 submit-seq.sh
$SCRIPTDIR/multi-run.sh tdlearn test-matches/para-runs MaxSize "1 2 3" 10 submit-para.sh
$SCRIPTDIR/multi-run.sh tdlearn test-matches/seq-runs MaxSize "1 2 3" 10 submit-seq.sh
$SCRIPTDIR/analyze-matches.sh test-matches/seq-matches MaxSize "1 2 3" 1600
$SCRIPTDIR/analyze-matches.sh test-matches/para-matches MaxSize "1 2 3" 1600
$SCRIPTDIR/experiment.sh tdlearn localshape test-matches/experiment -MaxSize "1 2 3" "Test experiment" 10 20 0 -Interval 8 -LogMode 1
rm -rf test-matches
