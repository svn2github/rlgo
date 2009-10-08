#!/usr/bin/python

#----------------------------------------------------------------------------#
# Python script to calculate elo difference from a winning proportion in [0, 1]

import sys

elovalues = [ 
    -1000, -677, -589, -538, -501, -470, -444, -422, -401, -383, 
    -366, -351, -335, -322, -309, -296, -284, -273, -262, -251, 
    -240, -230, -220, -211, -202, -193, -184, -175, -166, -158, 
    -149, -141, -133, -125, -117, -110, -102, -95, -87, -80, 
    -72, -65, -57, -50, -43, -36, -29, -21, -14, -7,
    0, 7, 14, 21, 29, 36, 43, 50, 57, 65, 
    72, 80, 87, 95, 102, 110, 117, 125, 133, 141, 
    149, 158, 166, 175, 184, 193, 202, 211, 220, 230, 
    240, 251, 262, 273, 284, 296, 309, 322, 335, 351, 
    366, 383, 401, 422, 444, 470, 501, 538, 589, 677, 
    1000, 1000 ]

def CalcElo(pwin):
    assert(pwin >= 0 and pwin <= 1)
    low = int(pwin * 100)
    high = low + 1
    p = pwin * 100 - low
    elo = int(elovalues[high] * p + elovalues[low] * (1 - p))
    return elo
    
#----------------------------------------------------------------------------#

def usage():
    print "calc-elo.py pwin"
    print "    pwin: Proportion of wins between 0 and 1"

def main():
    if len(sys.argv) < 2:
        usage()
        exit(1)
    pwin = float(sys.argv[1])
    elo = CalcElo(pwin)
    print elo

if __name__ == "__main__":
    main()
    
    