#include<stdio.h>
#include<winsock2.h>
#include<iostream>
#include<thread>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

int main(int argc , char *argv[])
{
    WSADATA wsa;
    SOCKET s;

    struct sockaddr_in server;

    char message[1024] , server_reply[2000];
    int recv_size, iresult;
    bool run = true;

    //starting up winsock
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }

    printf("Initialised.\n");

    //Create a TCP socket
    if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d" , WSAGetLastError());
    }

    printf("Socket created.\n");

    //remote server IP, IPv4, port
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );

    //Connect to remote server

    if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }

    std::cout << "Connected" << std::endl;

    //Send some data

    while(run) {
        //Lambda thread for receiving data
        std::thread tokimot([&] {
            while(run) {
                //iresult == 0 means no connection
                iresult = recv(s, server_reply, 2000, 0);
                if (iresult == SOCKET_ERROR) {
                    std::cout << "recv failed" << std::endl;
                    return 1;
                }
                //server shutdown sequence
                if (iresult == 0){
                    std::cout << "Server shutdown..." << std::endl;
                    run = false;
                    closesocket(s);
                    WSACleanup();
                    return 0;
                }

                //Add a NULL terminating character to make it a proper string before printing
                server_reply[iresult] = '\0';
                std::cout << server_reply << std::endl;
            }
        });
        //Lambda thread for sending data
        std::thread sender([&] {
            while (run) {
            std::cin.getline(message, 1024);

                if(*message!='q') {
                    iresult = send(s, message, strlen(message), 0);
                    if (iresult < 0) {
                        std::cout << "Send failed" << std::endl;
                        return 1;
                    }
                }
                    //Write letter 'q' to disconnect from server
                else{
                    std::cout << "Disconnecting..." << std::endl;
                    run = false;
                }
            }
        });

        tokimot.join();
        sender.join();
    }
    //Shutdown sequence
    shutdown(s, SD_SEND);
    WSACleanup();
    return 0;
}