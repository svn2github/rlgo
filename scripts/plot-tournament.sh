#----------------------------------------------------------------------------#
# Plot the results of a tournament using gnuplot

if [ $# -lt 3 ]
then
    echo "Usage:"
    echo "plot-tournament.sh path setting values [title] [keypos]"
    echo ""
    echo "path:     Path stem for all values"
    echo "setting:  Setting varied"
    echo "values:   Values of setting used"
    echo "title:    Title for plot"
    echo "keypos:   Position for key"
    exit 1
fi

PATHSTEM=$1
SETTING=$2
VALUES=$3
TITLE=$4
KEYPOS=$5
MINELO=0
MAXELO=1800
XLABEL="Simulations per move" # or "Training games"

if [ "$KEYPOS"=="" ]
then
    KEYPOS="top left"
fi

echo "set terminal postscript colour \"Helvetica\" 18" > $PATHSTEM/ratings.plt
echo "set output \"$PATHSTEM/$SETTING-ratings.ps\"" >> $PATHSTEM/ratings.plt
echo "set xlabel \"$XLABEL\"" >> $PATHSTEM/ratings.plt
echo "set ylabel \"Elo rating\"" >> $PATHSTEM/ratings.plt
echo "set logscale x" >> $PATHSTEM/ratings.plt
echo "set key $KEYPOS" >> $PATHSTEM/ratings.plt
echo "set xrange [10:*]" >> $PATHSTEM/ratings.plt
echo "set yrange [$MINELO:$MAXELO]" >> $PATHSTEM/ratings.plt
echo "set title \"$TITLE\"" >> $PATHSTEM/ratings.plt
echo "set style increment user" >> $PATHSTEM/ratings.plt
echo "set style line 6 lt rgb \"brown\"" >> $PATHSTEM/ratings.plt # Avoid yellow!
echo -n "plot " >> $PATHSTEM/ratings.plt

ELOBASE=`awk '$2 == "GnuGoD" { total += $3; count++ } END { if (count == 0) print 1800; else print 1800 - total/count; }' $PATHSTEM/ratings.txt`
COMMA=0
COUNT=1
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

    # Names are assumed to be of the form: "NAME_VALUE1:VALUE2"
    # Where VALUE1 identifies the plot and VALUE2 is the value on the x-axis
    awk '$1 != "Rank" {line=index($2,"_"); colon=index($2, ":"); value1=substr($2,line+1,colon-line-1); value2=substr($2,colon+1,length($2)-colon); value2=substr($2,colon+1,length($2)); if (value1 == "'$PATHSUB'") print value2,$3+'$ELOBASE',$4}' $PATHSTEM/ratings.txt | sort -n > $PATHSTEM/$PATHSUB-ratings.txt
    if [ $COMMA == 1 ]
    then
        echo -n ", " >> $PATHSTEM/ratings.plt
    fi
    if [ $VALUE == "UCT-Random" ] || [ $VALUE == "UCT-Fuego" ]
    then
        PLOTTITLE="$VALUE"
    else
        PLOTTITLE="$SETTING=$VALUE"
    fi
    echo -n "\"$PATHSTEM/$PATHSUB-ratings.txt\" u 1:2:3 w errorlines title \"$PLOTTITLE\"" >> $PATHSTEM/ratings.plt
    COMMA=1
    COUNT=$((COUNT+1))
done

gnuplot $PATHSTEM/ratings.plt
open $PATHSTEM/$SETTING-ratings.ps
