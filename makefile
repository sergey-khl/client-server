main: client.o server.o
	g++ -O -o client client.o
	g++ -O -o server server.o

client.o: client.cpp
	g++ -c client.cpp -o client.o

server.o: server.cpp
	g++ -c server.cpp -o server.o


clean:
	rm ./*o ./client ./server ./oobuntoo* ./Server