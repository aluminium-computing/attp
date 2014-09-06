#!/usr/bin/python
# Copyright (c) 2014 Aluminium Computing, Inc
# Tests ATTP server
import socket

class Test(object):
	def __init__(self, command, uri):
		self.command = command
		self.uri = uri
	def doTest(self, sock):
		sendbuf = "%s %s ATTP\\10" % (self.command, self.uri)
		print "Sending: %s" % (sendbuf)
		sock.sendall(sendbuf)
		sockfile = sock.makefile()
		line = sockfile.readline() # we want line 2
		if line != "ATTP\\10 Connection Established\n":
			print "fail: line 1: %s" % (line)
			return -1
		
		sockfile.readline() #skip boundary
		line = sockfile.readline()
		if line == "ATTP(R)\n":
			print "Test passed"
		else:
			print "Test failed: line 3 was %s" % line
			return -1
 
			

def getSocket():
	sock = socket.create_connection(["localhost", 4779])
	return sock

def main():
	testCase = Test("FETCH", "/test")
	ts = getSocket()
	testCase.doTest(ts)
	ts.sendall("EXIT")
	ts.close()


if __name__ == '__main__':
	main()
