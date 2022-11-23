HOST=$(shell hostname)

# main target to generate both the server and client programs
# same with man pdfs
main: client.o server.o
	g++ -O -o client client.o
	g++ -O -o server server.o
	groff -Tpdf -man ./server.man > server.pdf
	groff -Tpdf -man ./client.man > client.pdf

client.o: client.cpp
	g++ -c client.cpp -o client.o

server.o: server.cpp
	g++ -c server.cpp -o server.o

clean:
	rm ./*o ./client ./server ./$(HOST)* ./Server ./server.pdf ./client.pdf