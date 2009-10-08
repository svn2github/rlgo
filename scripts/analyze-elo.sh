if [ $# -lt 4 ]
then
    echo "Usage:"
    echo "analyze-elo.sh path setting values title"
    echo ""
    echo "path:     Output path where match data should be saved"
    echo "setting:  Primary setting to vary in player"
    echo "values:   Primary values to try for setting, e.g. \"0.1 0.2 0.3\""
    echo "title:    Title of the plot"
    echo ""
    exit 1
fi

PATHSTEM=$1
SETTING1=$2
VALUES1=$3
TITLE=$4

analyze-tournament.sh $PATHSTEM/short-names.txt $PATHSTEM
plot-tournament.sh $PATHSTEM $SETTING1 "$VALUES1" "$TITLE"
plot-tournament2.sh $PATHSTEM $SETTING1 "$VALUES1" "$TITLE"
