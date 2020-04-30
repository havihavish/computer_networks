/***********************************************************
***** NAME	:	HAVISH NALLAPAREDDY                      ***
***** DATE	:	MARCH 26TH, 2020                         ***
***** COURSE:	CSCE 5580 COMPUTER NETWORKS              ***
************************************************************
** This programyou will implement network port scanner in **
** C that will report the status of services and their    **
** associated ports for the TCP and UDP protocols for a   **
** given port range.                                      **
***********************************************************/

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

// function for printing the open ports in any protocol
void openport(int port, char * protocol) {
  struct servent * appl_name;
  char * name;
  
  //getting server poer information
  appl_name = getservbyport(htons(port), protocol);
  if (!appl_name) {
    printf("port %d  \t open   : scv name unavail\n", port);
  } else {
    name = appl_name -> s_name;
    printf("port %d  \t open   : %s\n", port, name);
  }
}

// function for printing the closed ports in both the protocols
void closeport(int port, char * protocol) {
  struct servent * appl_name;
  char * name;
  //getting server poer information
  appl_name = getservbyport(htons(port), protocol);
  if (!appl_name) {
    printf("port %d \t closed : scv name unavail\n", port);
  } else {
    name = appl_name -> s_name;
    printf("port %d \t closed : %s\n", port, name);
  }
}
int main(int argc, char ** args) {

  char buff[100] = "PING", buffs[100];
  int n, client, pckt_loss = 0,open = 1, portlow, porthigh;
  
  struct timeval t;
  struct hostent * host;
  socklen_t len;
  char * protocol;

  // Checking enough number of arguments is passed
  if (argc < 5) {
    fprintf(stderr, "usage: %s <hostname> <protocol> <portlow> <porthigh>\n", args[0]);
    exit(0);
  }

  fprintf(stderr, "scanning host=%s, protocol=%s ports: %s -> %s\n", args[1], args[2], args[3], args[4]);

  // Gets the IP address of the host which is the first argument
  host = gethostbyname(args[1]);
  if (host == NULL) {
    fprintf(stderr, "error: host %s not exist\n", args[1]);
    exit(EXIT_FAILURE);
  }
  
  //checking valid protocol entered or not
  protocol = args[2];
  if (strcmp(protocol, "tcp") != 0 && strcmp(protocol, "udp") != 0) {
    fprintf(stderr, "invalid protocol: %s Specify \"tcp\" or \"udp\"\n", protocol);
    exit(EXIT_FAILURE);
  }

  //Gets the port number which is the second argument
  portlow = atoi(args[3]);
  porthigh = atoi(args[4]);

  // checking ports status for UDP protocol
  if (strcmp(protocol, "udp") == 0) {
    //Creating the socket file descriptor
    for (int i = portlow; i <= porthigh; i++) {

      struct sockaddr_in scket;
      struct servent * appl_name;
      char * name;
      
	  //Creating the socket file descriptor for UDP 
	  client = socket(AF_INET, SOCK_DGRAM, 0);
	  if (client < 0) {
        fprintf(stderr, "Error, opening sockets\n");
        exit(0);
      }

      bzero( & scket, sizeof(scket));
      
      //Providing server address information
      scket.sin_port = htons(i);
      scket.sin_family = AF_INET;
      scket.sin_addr = * ((struct in_addr * ) host -> h_addr);

      //Initialising the weighting time to 0.1 second
	  t.tv_sec = 0.1;
      fd_set socks;
      FD_ZERO( & socks);
      FD_SET(client, & socks);
      
      //send datagram to server socket
	  if (sendto(client, buff, strlen(buff), 0, (struct sockaddr * ) & scket, sizeof(scket)) != -1) {

        // time-limited select blocking, where timeout passed to return even if no file descriptors are ready
		if (select(client + 1, & socks, NULL, NULL, & t) > 0) {
          if (FD_ISSET(client, & socks)) {

            n = recvfrom(client, buffs, 100, 0, (struct sockaddr * ) & scket, & len);
            buffs[n] = '\0';
            openport(i, protocol);
          } else {
          	//printing closed ports that do not set the file descriptor
            closeport(i, protocol);
          }

        }
		//printing status of open and closed ports 
		else {
        	appl_name = getservbyport(htons(i), protocol);
        	if (!appl_name) {
    			printf("port %d  \t open   : scv name unavail\n", i);
  			} else {
    			name = appl_name -> s_name;
    			if (open) {
              		printf("port %d  \t open   : %s\n", i, name);
              		open = 0;
            	} else {
              		printf("port %d  \t closed : %s\n", i, name);
              		open = 1;
            	}
  			}
        }

      } else {
        closeport(i, protocol);
      }

      //closing the socket
	  close(client);
    }
  } 
  // checking ports status for TCP protocol
  else {
    for (int i = portlow; i <= porthigh; i++) {
      struct sockaddr_in scket;
      
      
      //Creating the socket file descriptor for TCP
      client = socket(AF_INET, SOCK_STREAM, 0);
      if (client < 0) {
        fprintf(stderr, "Error, opening sockets\n");
        exit(0);
      }

      bzero( & scket, sizeof(scket));
      
      //Providing server address information
      scket.sin_port = htons(i);
      scket.sin_family = AF_INET;
      scket.sin_addr = * ((struct in_addr * ) host -> h_addr);

      // connect the client socket to server socket 
      if (connect(client, (struct sockaddr * ) & scket, sizeof(scket)) == 0) {
      	
		// printing the open socket when successful connect occurs
        openport(i, protocol);
      }
      
      //closing the socket
	  close(client);
    }
  }
  // assign IP, PORT 
}
