#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include<netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char ** args) {

    char buff[100] = "PING", buffs[100];
    int n, client, pckt_loss = 0, portno;
    float rtt = 0.0, t_sec = 0.0, t_usec = 0.0, min_rtt = 100.0, max_rtt = 0.0, sum = 0.0;
    struct sockaddr_in scket;
    struct timeval send_time, rcv_time, t;
    struct hostent * host;
    socklen_t len;

    // Checking enough number of arguments is passed
    if (argc < 3) {
        fprintf(stderr, "usage: %s <hostname> <port>\n", args[0]);
        exit(0);
    }

    // Gets the IP address of the host which is the first argument
    host = gethostbyname(args[1]);
    
    if (host == NULL) {
        fprintf(stderr, "Error, no such host\n");
        exit(EXIT_FAILURE);
    }

    //Gets the port number which is the second argument
    portno = atoi(args[2]);

    //Creating the socket file descriptor
    client = socket(AF_INET, SOCK_DGRAM, 0);
    if (client < 0) {
        fprintf(stderr, "Error, opening sockets\n");
    }

    bzero( & scket, sizeof(scket));

    //Providing server address information
    scket.sin_port = htons(portno);
    scket.sin_family = AF_INET;
    scket.sin_addr = * ((struct in_addr * ) host -> h_addr);
    
    //send 10 automated PING messages to the server 
    for (int i = 0; i < 10; i++) {

        t_usec = t_sec = 0.0;
        
        //Initialising the weighting time to 1 second
        t.tv_sec = 1;
        
        fd_set socks;
        FD_ZERO( & socks); // clears the structure	
        FD_SET(client, & socks); //	to check file descriptor client

        //Get time to sending information to server
        if (gettimeofday( & send_time, NULL)) {
            printf("time failed\n");
            exit(1);
        }

        //send datagram to server socket
        sendto(client, buff, strlen(buff), 0, (struct sockaddr * ) & scket, sizeof(scket));
        printf("%d: Sent... ", i + 1);

        // time-limited select blocking, where timeout passed to return even if no file descriptors are ready
        if (select(client + 1, & socks, NULL, NULL, & t) > 0) {

            //check file descriptor is ready
            if (FD_ISSET(client, & socks)) {
            	
                n = recvfrom(client, buffs, 100, 0, (struct sockaddr * ) & scket, & len);

                // Get time when acknowledgement recieved from server
                if (gettimeofday( & rcv_time, NULL)) {
                    printf("time failed\n");
                    exit(1);
                }
                buffs[n] = '\0';

                //calculating round trip time 
                t_sec +=  rcv_time.tv_sec - send_time.tv_sec;
                t_usec +=  (rcv_time.tv_usec - send_time.tv_usec)/1000.0 ;
                rtt = (t_usec + t_sec);

                sum += rtt;

                // Finding minimum rtt and maximum rtt of the recieved packets
                min_rtt = min_rtt > rtt ? rtt : min_rtt;
                max_rtt = max_rtt > rtt ? max_rtt : rtt;
                printf(" RTT = %f ms\n", rtt);
            }

        } else {

            // counting the dropped packets
            pckt_loss++;
            printf("Timed out\n");
        }
    }

    int rcvd = 10 - pckt_loss;

    printf("10 pkts xmited, %d pkts rcvd, %d%% pkt loss\n", rcvd, pckt_loss * 100 / 10);
    printf("min %f ms, max: %f ms,avg: %f\n", min_rtt, max_rtt, sum / rcvd);
    
    //closing socket
    close(client);
}
