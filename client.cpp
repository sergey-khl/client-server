#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

#include "helper.h"

using namespace std;

int main(int argc, char **argv) {
    //https://www.geeksforgeeks.org/socket-programming-cc/
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    string hello = "Hello from client";
    char buffer[1024] = { 0 };

    if (argc != 3) {
        exit(0);
    }
    int port = stoi(argv[1]);
    char *ipaddr = argv[2];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
 
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, ipaddr, &serv_addr.sin_addr) <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr,sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // main loop
    string cmd;
    while (getline(cin, cmd)) {
        if (cmd[0] == 'S') {
            Sleep(stoi(cmd.substr(1)));
        } else {
            send(sock, cmd.c_str(), cmd.size(), 0);
            valread = read(sock, buffer, 1024);
            printf("%s\n", buffer);
        }
    }

    // closing the connected socket
    close(client_fd);
    return(0);
}