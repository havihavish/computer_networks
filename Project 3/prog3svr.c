/***********************************************************
***** NAME	:	HAVISH NALLAPAREDDY                      ***
***** DATE	:	April 22nd, 2020                         ***
***** COURSE:	CSCE 5580 COMPUTER NETWORKS              ***
************************************************************
** In this project, I  wrote a complete C program        ***
** to support the server in a client/server model using  ***
** Linux stream sockets. 								 ***
***********************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <unistd.h>
#include<pthread.h>

char client_message[2000];
char buffer[1024];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int client_count = 1;

// Create a structure which is database to the store the clients
struct client{
	int socketno;
	char name[50];
};

// create 10 clients 
struct client clients[10];

//handle join requests for duplicate clients and other commands like database is full
int join(char *client, int fd){
	for(int i= 0;i<10;i++){
		if(clients[i].name[0] == '\0'){
			strcpy(clients[i].name,client);
			clients[i].socketno = fd;
			return 11;
		}
		if(strcmp(clients[i].name,client) == 0) return i;
		
	}
	return -1;
}

// function to check whether the requested client is present in the database or not
int check_client(int fd){
	
	for(int i= 0;i<10;i++){
		if(clients[i].socketno == fd)
		 return i;
	}
	return -1;
}


// function that handles all the JOIN, LIST, BCST, MESG, QUIT commands 
void * socketThread(void *arg)
{
	int newSocket = *((int *)arg);
	
	//Accepting connection requests if there are less than 10 clients in the database as registered.	
	if(client_count>10){
		
		printf("Client (%d): Database Full. Disconnecting User.\n",newSocket);
		printf("Error: Too Many Clients Connected\n");
	}else{
		printf("Client (%d): Connection Accepted\n",newSocket);
		printf("Client (%d): Connection Handler Assigned\n",newSocket);
	}
	
	if(client_count <= 10)
	for(;;){
		
		//receiving the messaage from the client
  		int ln = recv(newSocket , client_message , 2000 , 0);
  		client_message[ln-2] = '\0'; 
	    
	    //checking which command the client requested.
	    if(strncmp(client_message,"JOIN",4)==0){
	    	
			char scnd[50];
	    	for(int i =5;i<strlen(client_message);i++){
	    		scnd[i-5] = client_message[i];
			}
			scnd[strlen(client_message) - 5] = '\0';
			
	    	int r = join(scnd, newSocket);
	    	int dup = check_client(newSocket);
	    	
	    	// handling excess clients
	    	if( r == -1){
	    		
	    		pthread_mutex_lock(&lock);
  				char *message = malloc(sizeof(client_message)+20);
  				strcpy(message,"Too Many Users. Disconnecting User.\n");
  				strcpy(buffer,message);

		  		free(message);
  				pthread_mutex_unlock(&lock);
  				
  				send(newSocket,buffer,strlen(buffer),0);
			}
			//handling duplicate join requests
			else if(dup >= 0 ){
	    		
				pthread_mutex_lock(&lock);
				char *message = malloc(sizeof(client_message)+20);
  				strcpy(message,"User Already Registered: Username (");
				strcat(message,clients[dup].name);
				strcat(message,"), FD (");
				char fd[10];
				sprintf(fd, "%d", clients[dup].socketno);
				strcat(message,fd);
				strcat(message,")\n");
  				strcpy(buffer,message);
		  		free(message);
  				pthread_mutex_unlock(&lock);
  				send(newSocket,buffer,strlen(buffer),0);
				
			}
						//handling duplicate clients
			else if (r>=0 && r<10){
				
				pthread_mutex_lock(&lock);
  				char *message = malloc(sizeof(client_message)+20);
  				strcpy(message,"User Already Registered: Username (");
				strcat(message,clients[r].name);
				strcat(message,"), FD (");
				char fd[10];
				sprintf(fd, "%d", clients[r].socketno);
				strcat(message,fd);
				strcat(message,")\n");
  				strcpy(buffer,message);
		  		free(message);
  				pthread_mutex_unlock(&lock);
  				send(newSocket,buffer,strlen(buffer),0);
				
			}
			//handling the perfect clients
			else{
							
				pthread_mutex_lock(&lock);
  				char *message = malloc(sizeof(client_message)+20);
  				printf("Client (%d): %s\n",newSocket,client_message);
  				strcpy(message,"JOIN ");
				strcat(message,scnd);
				strcat(message," Request Accepted\n");
  				strcpy(buffer,message);
		  		free(message);
  				pthread_mutex_unlock(&lock);
  				
  				send(newSocket,buffer,strlen(buffer),0);
  				client_count++;
			}
			
		}
		// handling MESG requests from clients
		else if(strncmp(client_message,"MESG",4)==0){
	    	printf("Client (%d): MESG\n",newSocket);
	    	
	    	// checking the client is registered or not
	    	if(check_client(newSocket)==-1){
	    		
				pthread_mutex_lock(&lock);
  				char *message = malloc(sizeof(client_message)+20);
  				strcpy(message,"Unregistered User. Use \"JOIN <username>\" to Register.\n");
  				strcpy(buffer,message);
		  		free(message);
  				pthread_mutex_unlock(&lock);
  				send(newSocket,buffer,strlen(buffer),0);

			}
			//sending the message to requested client
			else{
				char scnd[50];
				int len;
				// extracting the sender name from the total client message
	    		for(int i =5;i<strlen(client_message);i++){
	    			if(client_message[i] == ' '){
	    				len =i+1;
	    				break;
					}
	    			scnd[i-5] = client_message[i];
				}
				scnd[strlen(client_message) - 5] = '\0';
				int notfound = 0;
				//sending message to client name requested.
				for(int i = 0;i<10;i++){
					if(strcmp(clients[i].name,scnd) == 0){
						
						
						char msg[100];
						for(int i = len; i<strlen(client_message);i++){
							msg[i-len] = client_message[i];
						}
						msg[strlen(client_message) - len] = '\0';
						pthread_mutex_lock(&lock);
  						char *message = malloc(sizeof(client_message)+20);
  						strcpy(message,"From ");
  						strcat(message,clients[check_client(newSocket)].name);
  						strcat(message,": ");
  						strcat(message,msg);
  						strcat(message,"\n");
  						buffer[0] = '\0';	
  						strcpy(buffer,message);
  						free(message);
  						pthread_mutex_unlock(&lock);
  						send(clients[i].socketno,buffer,strlen(buffer),0);
  						notfound =1;
						break;
					}					
				}
				// handling clients that are not registered in the database
				if(notfound == 0){
						printf("Unable to Locate Recipient (%s) in Database. Discarding MESG.\n",scnd);
						pthread_mutex_lock(&lock);
  						char *message = malloc(sizeof(client_message)+20);
  						
  						strcpy(message,"Unknown Recipient (");
						strcat(message,scnd);
						strcat(message,"). MESG Discarded.\n");
  						strcpy(buffer,message);
				  		free(message);
		  				pthread_mutex_unlock(&lock);
  						
  						send(newSocket,buffer,strlen(buffer),0);
				}
			}
		}
		// handling BCST message request
		else if(strncmp(client_message,"BCST",4)==0){
			printf("Client (%d): BCST\n",newSocket);
	    	// checking the client is registered or not
	    	if(check_client(newSocket)==-1){

				pthread_mutex_lock(&lock);
				printf("Unable to Locate Client (%d) in Database. Discarding BCST\n",newSocket);
  				char *message = malloc(sizeof(client_message)+20);
  				strcpy(message,"Unregistered User. Use \"JOIN <username>\" to Register.\n");
  				strcpy(buffer,message);
		  		free(message);
  				pthread_mutex_unlock(&lock);
  				send(newSocket,buffer,strlen(buffer),0);

			}
			// sending BCST message to all clients except itself.
			else{
				char msg[100];
				// extracting the bcst message from the client message.
				for(int i = 5; i<strlen(client_message);i++){
					msg[i-5] = client_message[i];
				}
				msg[strlen(client_message) - 5] = '\0';
				for(int i = 0;i<10;i++){
					if(clients[i].name[0] != '\0' && clients[i].socketno != newSocket){
						pthread_mutex_lock(&lock);
  						char *message = malloc(sizeof(client_message)+20);
  						strcpy(message,"From ");
  						strcat(message,clients[check_client(newSocket)].name);
  						strcat(message,": ");
  						strcat(message,msg);
  						strcat(message,"\n");
  						buffer[0] = '\0';
  						strcpy(buffer,message);
				  		free(message);
  						pthread_mutex_unlock(&lock);
  						send(clients[i].socketno,buffer,strlen(buffer),0);
						
					}
				}
			}
		}
		else if(strncmp(client_message,"LIST",4)==0){
			printf("Client (%d): LIST\n",newSocket);
			
			// checking the client is registered or not
			if(check_client(newSocket)==-1){
				
				pthread_mutex_lock(&lock);
				printf("Unable to Locate Client (%d) in Database. Discarding LIST\n",newSocket);
  				char *message = malloc(sizeof(client_message)+20);
  				strcpy(message,"Unregistered User. Use \"JOIN <username>\" to Register.\n");
  				strcpy(buffer,message);
		  		free(message);
  				pthread_mutex_unlock(&lock);
  				send(newSocket,buffer,strlen(buffer),0);

			}
			//Sending the list of clients in the database to requested client.
			else{
				char *message = malloc(sizeof(client_message)+20);
  				strcpy(message,"Username \t FD\n----------------\n");
  				pthread_mutex_lock(&lock);
				for(int i = 0;i<10;i++){
					if(clients[i].name[0] != '\0'){
						
  						strcat(message,clients[i].name);
  						strcat(message,"\t");
  						char fd[10];
						sprintf(fd, "%d", clients[i].socketno);
						strcat(message,fd);
  						strcat(message,"\n");
					}
				}
				strcat(message,"----------------\n");
				buffer[0] = '\0';
				strcpy(buffer,message);
				free(message);
  				pthread_mutex_unlock(&lock);
  				send(newSocket,buffer,strlen(buffer),0);
			}
		}
		//handling QUIT command for both registered and unregistered user.
		else if(strncmp(client_message,"QUIT",4)==0){
			// checking the client is registered or not
			if(check_client(newSocket)==-1){
				printf("Client (%d): QUIT\n",newSocket);
				printf("Unable to Locate Client (%d) in Database. Disconnecting User\n",newSocket);
				break;
			}
			else{
				printf("Client (%d): QUIT\n",newSocket);
				printf("Client (%d): Disconnecting User\n",newSocket);
				client_count--;
				for(int i = 0;i<10;i++){
					if(clients[i].socketno == newSocket){
						clients[i].name[0] = '\0';
						clients[i].socketno = 0;
					}
				}
	    		break;
			}	
		}
		//handling the unknown message received 
		else{

				pthread_mutex_lock(&lock);
				printf("Client (%d): Unrecognizable Message. Discarding UNKNOWN Message.\n",newSocket);
  				char *message = malloc(sizeof(client_message)+20);
  				strcpy(message,"Unknown Message. Discarding UNKNOWN Message.\n");
  				strcpy(buffer,message);
		  		free(message);
  				pthread_mutex_unlock(&lock);
  				
  				send(newSocket,buffer,strlen(buffer),0);
		}
	}
	
	// closing the socket and exiting the thread.
	close(newSocket);
	pthread_exit(NULL);

}
int main(int argc, char ** args){
	
	
  int serverSocket, newSocket,portno;
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  
  // check enough number of arguments passed or not
	if (argc < 2) {
        fprintf(stderr, "usage: %s <port>\n", args[0]);
        exit(0);
    }
    portno = atoi(args[1]);
    
  //Create the socket. 
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  serverAddr.sin_family = AF_INET;
  //Set port number, using htons function to use proper byte order 
  serverAddr.sin_port = htons(portno);
  //Set IP address to localhost 
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
  
  //Bind the address struct to the socket 
  bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  
  
  //initialise null to 10 clients
  for(int j = 0; j<10;j++){
  	clients[j].name[0] = '\0';
  	
  }
  
  //Listen on the socket, with 15 max connection requests queued 
  if(listen(serverSocket,15)==0)
    printf("Waiting for Incoming Connections..\n");
  else
    printf("Error\n");
    
    // create 10 threads
    pthread_t tid[10];
    int i = 0;
    while(1)
    {
        //Accept call creates a new socket for the incoming connection
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
        
        //for each client request creates a thread and assign the client request to it to process
        //by this I allow the main thread can entertain next request
        if( pthread_create(&tid[i++], NULL, socketThread, &newSocket) != 0 )
           printf("Failed to create thread\n");
        if( i >= 10)
        {
          i = 0;
          while(i < 5)
          {
            pthread_join(tid[i++],NULL);
          }
          i = 0;
        }
    }
  return 0;
}
