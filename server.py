import sys, copy, multiprocessing, socket
from client import Client
def handleRequest(connection, address):
        while True:
            data = connection.recv(1024)
            if data == "":
                print"Socket closed remotely"
                break
            print "Received data" + data
            connection.sendall("ack")


class Server():
    def __init__ (self,ip, port):
        self.socketsOpen = 0
        self.ip = ip
        self.port = port
    def start(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind((self.ip, self.port))
        self.socket.listen(1)

        while True:
            conn, address = self.socket.accept()
            process = multiprocessing.Process(target=handleRequest, args=(conn, address))
            process.daemon = True
            process.start()





if __name__ == "__main__":
    HOST, PORT = "0.0.0.0", 9000
    #Client(HOST,PORT);
    server = Server(HOST, PORT)
    try:
        print "Listening on port " + str(PORT)
        server.start()
    except:
        logging.exception("Unexpected exception")
    finally:
        print "Shutting down"
        for process in multiprocessing.active_children():
            process.terminate()
            process.join()


    #instantiate the client
