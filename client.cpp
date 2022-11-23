#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "helper.h"

using namespace std;

int main(int argc, char **argv) {
    //https://www.geeksforgeeks.org/socket-programming-cc/
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };

    if (argc != 3) {
        cout << "incorrect number of calling arguments" << endl;
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

    ofstream logfile;

    char hostname[128];
    gethostname(hostname, 128);
    string host = string(hostname) + "." + to_string(getpid());
    logfile.open(host);
    
    logfile << "Using port " << port << endl;
    logfile << "Using server address " << ipaddr << endl;
    logfile << "Host " << host << endl;

    double now;
    int transacNum = 0;
    // main loop
    string cmd;
    while (getline(cin, cmd)) {
        // https://www.epochconverter.com/
        int n = stoi(cmd.substr(1));
        if (cmd[0] == 'S') {
            logfile << "Sleep " << n << " units" << endl;
            Sleep(n);
        } else {
            now = chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
            logfile << fixed << setprecision(2) << now / 1000 << ": Send (T" << setw(3) << n << ")" << endl;
            cmd += " " + host;
            cmd[cmd.size()] = '\0';
            send(sock, cmd.c_str(), cmd.size(), 0);
            transacNum += 1;
            valread = read(sock, buffer, 1024);
            now = chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
            logfile << now / 1000 << ": Recv (D" << setw(3) << string(buffer).substr(1) << ")" << endl;
        }
    }

    logfile << "Sent " << transacNum << " transactions" << endl;

    // close logfile
    logfile.close();

    // closing the connected socket
    close(client_fd);
    return(0);
}