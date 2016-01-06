all:
	g++ -c LinuxSockets.cpp -o LinuxSockets.o
	g++ -c main.cpp -o main.o
	g++ LinuxSockets.o main.o -o extip -lnsl -lpthread