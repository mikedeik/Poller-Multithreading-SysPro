all: server client

server:
	g++ -pthread -o poller Server.cpp Master.cpp

client:
	g++ -pthread Client.cpp -o pollSwayer

clear:
	-rm *.o