#include <netinet/in.h>
#include <sys/time.h>
#include <map>
#include <cstring>

#include "helper.h"

using namespace std;

int main(int argc, char **argv) {
    //https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
    int clients = 100;
    int server_fd, new_socket, valread, client_sockets[clients], max_sd, activity;
    struct sockaddr_in address;
    fd_set readfds;
    timeval timeout;
    int opt = 1;
    
    if (argc != 2) {
        cout << "incorrect number of calling arguments" << endl;
        exit(0);
    }
    int port = stoi(argv[1]);

    //initialise all client_socket[] to 0 so not checked 
    for (int i = 0; i < clients; i++)  
    {  
        client_sockets[i] = 0;  
    }  
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    int addrlen = sizeof(address);
 
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // setting max clients to 3
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    ofstream logfile;

    logfile.open("Server");
    
    logfile << "Using port " << port << endl;

    double startTime = 0, time;
    map<string, int> transactions;
    transactions["total"] = 0;
    // main loop
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;

        //add child sockets to set 
        for (int i = 0 ; i < clients ; i++) {  
            //if valid socket descriptor then add to read list 
            if(client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &readfds);  
            }

            //highest file descriptor number, need it for the select function 
            if(client_sockets[i] > max_sd){
                max_sd = client_sockets[i]; 
            }
        }  

        // wait for client socket io or timeout
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &timeout);  

        if ((activity < 0) && (errno!=EINTR)) {  
            printf("select error");  
        } 
        if (activity == 0) {
            break;
        }

        // incoming connection
        if (FD_ISSET(server_fd, &readfds)) { 
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }

            //add new socket to array of sockets 
            for (int i = 0; i < clients; i++) {  
                //if position is empty 
                if( client_sockets[i] == 0 ) {  
                    client_sockets[i] = new_socket;
                    break;  
                }  
            }  
        }

        char buffer[1024];
        for (int i = 0; i < clients; i++)  
        {  
            if (FD_ISSET(client_sockets[i], &readfds)) {  
                // if client closes then we can reuse socket
                if ((valread = read( client_sockets[i] , buffer, 1024)) == 0) {  
                    //Close the socket and mark as 0 in list for reuse 
                    close( client_sockets[i] );  
                    client_sockets[i] = 0;  
                }  
                else {
                    time = chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
                    // first transaction
                    if (startTime == 0) {
                        startTime = time;
                    }
                    transactions["total"] += 1;
                    string inp = string(buffer);
                    string cmd = inp.substr(0, inp.find(" "));
                    int n = stoi(cmd.substr(1));
                    string host = inp.substr(inp.find(" ") + 1);
                    if (transactions.find(host) == transactions.end()) {
                        transactions[host] = 1;
                    } else {
                        transactions[host] += 1;
                    }
                    memset(buffer, 0, sizeof buffer);
                    logfile << fixed << setprecision(2) << time / 1000 << ": #" << setw(3) << transactions["total"] << " (T" << setw(3) << n << ") from " << host << endl;
                    Trans(n);
                    time = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
                    logfile << time / 1000 << ": #" << setw(3) << transactions["total"] << " (Done) from " << host << endl;
                    string done = "D" + to_string(transactions["total"]);
                    send(client_sockets[i], done.c_str(), done.length(), 0);
                }  
            }  
        }
    }
    double endTime = chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    logfile << "\nSUMMARY" << endl;
    for (auto const& trans: transactions) {
        if (trans.first != "total") {
            logfile << "\t" << trans.second << " transactions from " << trans.first << endl;
        }
    }
    
    logfile << transactions["total"]/((endTime - startTime)/1000 - 30) << " transactions/sec (" << transactions["total"] << "/" << (endTime - startTime)/1000 - 30 << ")" << endl;
    
    // closing the connected socket
    for (int i = 0; i < clients; i++) {  
        close(client_sockets[i]);
    }  
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return(0);
}