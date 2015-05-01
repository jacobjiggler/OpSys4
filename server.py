import sys, copy, multiprocessing, SocketServer
from client import Client

class RequestHandler(SocketServer.BaseRequestHandler):
    def __init__ (self):
        self.socketsOpen = 0


    def receive_message(self):
        #delimit by /n
        return
if __name__ == '__main__':
    HOST, PORT = "localhost", 9999
    print "Listening on port" + str(PORT)
    server = SocketServer.TCPServer((HOST, PORT), RequestHandler)


    #instantiate the client
    Client();

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()
