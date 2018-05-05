import socket
import time

msgFromClient       = "SET;a;10;"
bytesToSend         = str.encode(msgFromClient)
serverAddressPort   = ("10.194.79.80", 4040)
bufferSize          = 1024



def send():
# Create a UDP socket at client side
    start= time.time()
    UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    # Send to server using created UDP socket
    UDPClientSocket.sendto(bytesToSend, serverAddressPort)
    msgFromServer = UDPClientSocket.recvfrom(bufferSize)
    end=time.time()
    msg = "Message from Server {}".format(msgFromServer[0])
    return (end - start)


l = []
for i in range(10):
    l.append(send())

print(l)
