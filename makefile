all: server client

server:
	g++ -pthread -o poller_server Server.cpp Master.cpp

client:
	g++ -pthread Client.cpp -o client.o

clear:
	-rm *.o