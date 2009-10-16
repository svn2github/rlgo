#!/usr/bin/python

#----------------------------------------------------------------------------#
# Python script to run multi-process RLGO 
# and forward GTP commands to each process

import gtp
import popen2
import sys
import string
import time
import os
    
def farm(numprocesses, scriptdir, master, slave):
    players = []
    for i in xrange(numprocesses):
        ismaster = (i == 0)
        if ismaster:
            cmdline = "%s -ShareName ipc.mem -Process %d -RandomSeed %d" % (master, i, i)
        else:
            cmdline = "%s -ShareName ipc.mem -Process %d -RandomSeed %d" % (slave, i, i)
        debugname = "RLGO.%d" % i
        player = gtp.GtpConnection(cmdline, debugname)
        players.append(player)

    time.sleep(1)
    print "Farm is ready: %d players" % len(players)
    cmd = ""
    while cmd != "quit":
        cmd = readcmd()
        cmdargs = cmd.split()
        if cmdargs[0] == "genmove":
            responses = execute(players, makecommands2("genmove", "reg_genmove", cmdargs[1], numprocesses))
            execute(players[1:], makecommands("play %s %s" % (cmdargs[1], responses[0]), numprocesses - 1))
        else:
            responses = execute(players, makecommands(cmd, numprocesses))
        writecmd(responses[0])

def execute(players, cmds):

    # Forward command to all players
    for i in xrange(len(players)):
        players[i].write_cmd(cmds[i])

    # Read all responses but only return final (master) response
    responses = []
    for player in players:
        responses.append(player.read_cmd())
        
    return responses

def makecommands(cmd, numprocesses):
    cmds = []
    for i in xrange(numprocesses):
        cmds.append(cmd)
    return cmds

def makecommands2(mastercmd, slavecmd, args, numprocesses):
    cmds = []
    for i in xrange(numprocesses):
        if i == 0:
            cmds.append("%s %s" % (mastercmd, args))
        else:
            cmds.append("%s %s" % (slavecmd, args))
    return cmds

def readcmd():
    result = raw_input()

    # Remove trailing newline from the result
    if result[-1] == "\n":
        result = self.result[:-1]
        
    return result

def writecmd(response):
    print "= %s\n" % response
    sys.stdout.flush()

def main(argv):
    if len(argv) != 4:
        print "farm.py numprocesses master_cmd slave_cmd"
        print "    numprocesses: total number of processes to run with RLGO"
        print "    master_cmd:   command to launch RLGO master process"
        print "    slave_cmd:    command to launch RLGO slave processes"
        sys.exit()
    scriptdir = os.path.dirname(argv[0])
    numprocesses = int(argv[1])
    master = argv[2]
    slave = argv[3]

    farm(numprocesses, scriptdir, master, slave)

if __name__ == "__main__":
    main(sys.argv)
