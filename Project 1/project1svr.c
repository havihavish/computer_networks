#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/in.h>

int main(int argc, char ** args) {

    char buff[100], buffc[100] = "PONG";
    int servfd, portno;
    struct sockaddr_in scket, clisckt;

    //Checking enough number of arguments is passed
    if (argc < 2) {
        fprintf(stderr, "usage: %s <port>\n", args[0]);
        exit(0);
    }

    //Gets the port number which is the first argument
    portno = atoi(args[1]);

    bzero( & scket, sizeof(scket));
    bzero( & clisckt, sizeof(clisckt));

    //Create a UDP Socket
    servfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (servfd < 0) {
        fprintf(stderr, "Error, opening sockets\n");
    }

    //Providing server address information
    scket.sin_addr.s_addr = htonl(INADDR_ANY);
    scket.sin_port = htons(portno);
    scket.sin_family = AF_INET;

    //Bind the server address to socket
    if (bind(servfd, (struct sockaddr * ) & scket, sizeof(scket)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    };

    printf("[server]: ready to accept data...\n");

    socklen_t len = sizeof(clisckt);
    while (1) {
        int n = recvfrom(servfd, buff, 50, 0, (struct sockaddr * ) & clisckt, & len);

        //simulate 30% packet loss 
        int ran = rand() % 10;
        if (ran < 3) {
            printf("[client]: %s\n", buff);
            printf("[server]: dropped packet\n");

        } else {
            buff[n] = '\0';
            printf("[client]: %s\n", buff);
            sendto(servfd, buffc, 5, 0, (struct sockaddr * ) & clisckt, len);
        }

    }
}
