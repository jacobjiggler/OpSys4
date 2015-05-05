#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>

#define BUFFER_SIZE 1024
#define PORT 8765
#define FRAME_SIZE 1024
#define FRAMES = 32
#define FRAMES_PER_FILE 4

//declare mutex locks up here

//struct to hold page info
struct page {
  char filename[1000];
  int pageNum;

  //Will hold the number in which it was edited
  int lastEdited;
};

void *connection_handler(void *socket_desc)
{
  //(PARSE OUT COMMAND OR RETURN ERROR)
    //if !string compare(client_message(first 4 char) "DIR\n")
      //call dir function


    //elseif !string compare(client_message(first 6 char) "STORE ")
      //if \n in client message
        //save index of \n
      //else
        //printf("ERROR: Incorrect Syntax For COMMAND\n");
        //return
      //if there is a digit before \n
        //store index of first digit in string of digits
        //convert num to int and save as bytes
      //else
        //printf("ERROR: Incorrect Syntax For COMMAND\n");
        //return
      //save filename(dont forget about the space between filename and bytes)
      //either create and call a store function or write it here


    //else if !string compare(client_message(first 5 char) "READ ")
      //if \n in client message
        //save index of \n
      //else
        //printf("ERROR: Incorrect Syntax For COMMAND\n");
        //return
      //if there is a digit before \n
        //store index of first digit in string of digits
        //convert num to int and save as length
      //else
        //printf("ERROR: Incorrect Syntax For COMMAND\n");
        //return
      //if there is a digit before index of previous digits
        //store index of first digit in string of digits
        //convert num to int and save as byteoffset
      //else
        //printf("ERROR: Incorrect Syntax For COMMAND\n");
        //return
      //save filename (dont foget the space)
      //either create and call a READ function or write it here


    //else if !string compare(client_message(first 5 char) "DELETE ")
      //if \n in client message
        //save index of \n
      //else
        //printf("ERROR: Incorrect Syntax For COMMAND\n");
        //return
      //if there is a char before \n
        //save filename
      //else
        //printf("ERROR: Incorrect Syntax For COMMAND\n");
        //return
      //call delete function


    //else
      //printf("ERROR: Incorrect Syntax For COMMAND\n");
      //return






    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size = 0;
    char client_message[2000];

    //Send some messages to the client

    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //end of string marker
		client_message[read_size] = '\0';

		printf("Rcvd: %s", client_message);

		//Send the message back to client
        write(sock , "AWK\n" , strlen("AWK\n"));

		puts("Sent: ACK");
		fflush(stdout);


		//clear the message buffer
		memset(client_message, 0, 2000);
    }

    if(read_size == 0)
    {
        puts("Client closed it's socket....terminating");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    return 0;
}

int main(int argc , char *argv[])
{
  //declare memory array here(32 slots with 1024 bits)
    // allocate 32,768 with calloc
    //then turn into array of char of that size
  //declare page table here (32 slots filled with custom struct page)

	int socket_desc , client_sock, c;
	struct sockaddr_in server, client;
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	/* bind to a specific (OS-assigned) port number */
    if ( bind( socket_desc, (struct sockaddr *)&server, sizeof( server ) ) == -1 )
    {
		perror( "bind() failed" );
		return EXIT_FAILURE;
    }
	puts("bind done");

	listen(socket_desc, 5);
	c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{

	    if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("failed to create thread");
            return 1;
        }
        char str[INET_ADDRSTRLEN];
        printf("Received incoming connection from %s \n", inet_ntop(AF_INET, &client.sin_addr.s_addr, str, INET_ADDRSTRLEN));

	}
	if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
	return 0;
}
