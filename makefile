main: client.o server.o
	g++ -o client client.o
	g++ -o server server.o

client.o: client.cpp
	g++ -c client.cpp -o client.o

server.o: server.cpp
	g++ -c server.cpp -o server.o

clean:
	rm ./*o ./client ./server