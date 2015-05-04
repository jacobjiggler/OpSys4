import sys, copy, multiprocessing, socket,time

class Client:
    def __init__ (self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(("localhost", 9000))
        data = "some data"
        sock.sendall(data)

        result = sock.recv(1024)
        print result
        sock.close()
if __name__ == '__main__':
    Client()
