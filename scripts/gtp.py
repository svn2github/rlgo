#----------------------------------------------------------------------------#
# A very simple python interface for the Go Text Protocol

import popen2
import sys

alwaysdebug = 0
debug = 0

#----------------------------------------------------------------------------#

class GtpError(Exception):
    """ Simple error class for GTP connections """
    
    def __init__(self, connection, errorstring):
        self.engine = connection.connectionID
        self.cmd = connection.cmd
        self.errorstring = errorstring
        self.response = connection.result
    def __str__(self):
        return "\nSubprocess: %s\nCommand: %s\nError: %s\nResponse: %s\n" % \
            (self.engine, self.cmd, self.errorstring, self.response)

#----------------------------------------------------------------------------#
    
class GtpConnection:
    """ Class maintaining a connection to a single Go engine
        Commands are passed to connection via GTP protocol """

    def __init__(self, command, debugname):
        self.connectionID = debugname
        self.invoke = command
        self.cmd = command
        self.result = ""
        self.quit = False
        self.copyfiles = []

        print "Executing %s" % command
        try:
            self.subprocess = popen2.Popen3(command)
        except:
            raise GtpError(self, "Failed to open pipe")

    def exec_cmd(self, cmd, commandlog = None, showboard = False):

        self.cmd = cmd

        if commandlog:
            commandlog.write("%s\n" % cmd)
            commandlog.flush()

        if self.quit:
            print "Program has quit, skipping command %s" % cmd
            return None

        for copyfile in self.copyfiles:
            copyfile.write("%s\n" % cmd)

        write_cmd(cmd)
        return read_cmd()

    def write_cmd(self, cmd):

        if debug or alwaysdebug:
            print "GTP command [%s]: %s\n" % (self.connectionID, cmd)

        try:
            self.subprocess.tochild.write("%s\n\n" % cmd)
            self.subprocess.tochild.flush()

        except IOError:
            raise GtpError(self, "Pipe broken")
            
    def read_cmd(self):

        self.result = ""
        line = self.readline()
        while line != "\n":
            self.result = self.result + line
            line = self.readline()
            
        #! This can happen if previous command was interrupted
        if len(self.result) == 0:
            raise GtpError(self, "No response - interrupted command?")

        # Remove trailing newline from the result
        if self.result[-1] == "\n":
            self.result = self.result[:-1]

        if debug or alwaysdebug:
            print "Response: %s\n" % self.result[2:]

        if (self.result[0] == "?"):
            raise GtpError(self, "Command [%s] failed" % self.cmd)

        if (self.result[0] == "="):
            return self.result[2:]

        raise GtpError(self, "Unknown response")

    def readline(self):
        """ Read a line from the subprocess
            First checking that it hasn't terminated unexpectedly """
            
        poll = self.subprocess.poll()
        if poll != -1:
            self.quit = True
            if not (poll == 0 and self.cmd == "quit"):
                raise GtpError(self, "Subprocess terminated unexpectedly")
            
        return self.subprocess.fromchild.readline()
        
    def copyto(self, copyfile):
        """ Copy each GTP command into specified file """
        
        self.copyfiles.append(copyfile)
        
#----------------------------------------------------------------------------#

