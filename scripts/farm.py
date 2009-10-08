#!/usr/bin/python

#----------------------------------------------------------------------------#
# Python script to farm out many processes and forward GTP commands

import gtp
import popen2
import sys
import string
import time
    
def farm(numprocesses, dir, master, slave, args):
    players = []
    for i in xrange(numprocesses):
        ismaster = (i == 0)
        if ismaster:
            cmdline = shellex("getprogram.sh %s -Process %d -RandomSeed %d %s" % (master, i, i, args))
        else:
            cmdline = shellex("getprogram.sh %s -Process %d -RandomSeed %d %s" % (slave, i, i, args))
        debugname = "RLGO.%d" % i
        player = gtp.GtpConnection(cmdline, debugname)
        players.append(player)

    time.sleep(1)
    print "Farm is ready"
    cmd = ""
    while cmd != "quit":
        cmd = readcmd()
        cmdargs = cmd.split()
        if cmdargs[0] == "genmove":
            response = execute(players[0:1], cmd)
            execute(players[1:], "play %s %s" % (cmdargs[1], response))
        else:
            response = execute(players, cmd)
        writecmd(response)

def execute(players, cmd):

    # Forward command to all players
    for player in players:
        player.write_cmd(cmd)

    # Read all responses but only return final (master) response
    response = ""
    for player in players:
        response = player.read_cmd()
        
    return response

def readcmd():
    result = raw_input()

    # Remove trailing newline from the result
    if result[-1] == "\n":
        result = self.result[:-1]
        
    return result

def writecmd(response):
    print "= %s\n" % response
    sys.stdout.flush()

def shellex(command):
    subprocess = popen2.Popen3(command)
    return subprocess.fromchild.readline()    

def main(argv):
    if len(argv) < 4:
        print "farm.py numprocesses rlgo_dir master slave args"
        sys.exit()
    numprocesses = int(argv[1])
    dir = argv[2]
    master = argv[3]
    slave = argv[4]
    args = ""
    for arg in argv[5:]:
        args = "%s %s" % (args, arg)

    if "-DataPath" not in argv[5:]:
       args = "%s -DataPath %s" % (args, dir) 

    farm(numprocesses, dir, master, slave, args)

if __name__ == "__main__":
    main(sys.argv)
