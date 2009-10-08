#----------------------------------------------------------------------------#
# Python script to select pair of programs with approximately equal results

import sys
import math
import random

k=2

def selectrandom(probs):
    # Select a random index from discrete probability distribution
    size = len(probs);
    r = random.random();
    for i in xrange(size):
        r -= probs[i]
        if r < 0:
            return i
    
    assert(math.fabs(r) < 0.001);
    return size - 1;

def readinput(np):    
    names = ["" for i in xrange(np)]
    wins = [0 for i in xrange(np)]
    for i in xrange(np):
        parsedline = sys.stdin.readline().split()
        names[i] = parsedline[0]
        wins[i] = int(parsedline[1])
    return names,wins

def pair(pb, np, names, wins):
    diffs = [0 for i in xrange(np)]
    prefs = [0 for i in xrange(np)]
    probs = [0 for i in xrange(np)]
    totalwins = 0.0
    totalprefs = 0.0

    for i in xrange(np):
        totalwins = totalwins + wins[i]
    if totalwins == 0: # no information yet, select randomly
        return random.randint(0, np - 1)

    meanwins = totalwins / np
    for i in xrange(np):
        if names[i] != names[pb]:
            diffs[i] = abs(wins[i] - wins[pb])
            prefs[i] = math.exp(-k * diffs[i] / meanwins)
            totalprefs = totalprefs + prefs[i]

    for i in xrange(np):
        probs[i] = prefs[i] / totalprefs
        
    return selectrandom(probs)

def main(argv):
    assert(len(argv) == 3)
    np = int(argv[1])
    pb = int(argv[2])
    assert(np >0)
    assert(pb >= 0 and pb < np)
    names, wins = readinput(np)
    pw = pair(pb, np, names, wins)
    print pw

if __name__ == "__main__":
    main(sys.argv)
