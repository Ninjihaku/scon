all:
	g++ -c WindowsSockets.cpp -o WindowsSockets.o
	g++ -c main.cpp -o main.o
	g++ WindowsSockets.o main.o -o extip -static-libgcc -static-libstdc++ -static -lm -lws2_32