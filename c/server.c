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
#include <ctype.h>

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


    //Get the socket descriptor
    int sock = *(int*)socket_desc;


    //Send some messages to the client

    FILE *command = fdopen(sock, "r");
    write(sock , "HALLO!\n" , strlen("HALLO!\n"));



    //(PARSE OUT COMMAND OR RETURN ERROR)(USE FGETS)
    while(1){
      char temp[PATH_MAX + 1];
      if (fgets(temp, PATH_MAX+ 1, command) != NULL){
        char dest[6];
        strncpy(dest,temp,6);
        if (strcmp(temp, "DIR\n")==0){
          printf("Command DIR Recognized\n");
          //call dir function

          DIR *dir;
          struct dirent *ent;
          char dirname[] = ".storage";
          if ((dir = opendir (dirname)) != NULL) {
            /* print all the files and directories within directory */
            int count = 0;
            while ((ent = readdir (dir)) != NULL) {
              if (!strcmp(ent->d_name,"..") || !strcmp(ent->d_name,".")){
                continue;
              }
              count++;
            }
            char output[1000];
            sprintf(output, "%d\n", count);
            closedir (dir);
            dir = opendir (dirname);
            while ((ent = readdir (dir)) != NULL) {
              if (!strcmp(ent->d_name,"..") || !strcmp(ent->d_name,".")){
                continue;
              }
              strcat(output, ent->d_name);
              strcat(output, "\n");
            }
            write(sock , output , strlen(output));
            closedir (dir);

          } else {
            /* could not open directory */
            printf ("couldn't open directory \n");
          }
        }

        else if (!strcmp(dest,"STORE ")){
          //for now
          int length = strlen(temp);
          char file_name [100];
          char bytes_size [100];
          int f_pos = 0;
          int b_pos = 0;
          int pos = 6;
          while(temp[pos] != ' '){
            file_name[f_pos] = temp[pos];
            //putchar(file_name[f_pos]);
            f_pos++;
            pos++;
          }
          pos++;
          while(temp[pos] != '\n'){
            bytes_size[b_pos] = temp[pos];
            b_pos++;
            pos++;
          }
          printf("f_pos %d\n", f_pos);
          printf("b_pos %d\n", b_pos);
          file_name[f_pos] = '\0';
          bytes_size[b_pos] = '\0';

          //puts(file_name);
    			//puts(bytes_size);
    			int numBytes = atoi(bytes_size);          //Check if file exists
          char file_path[1000];
          strcat(file_path, ".storage/");
          strcat(file_path, file_name);
          if( access( file_path, F_OK ) != -1 ) {
              // file exists
              perror("FILE EXISTS\n");
          } else {
              // file doesn't exist



              //save as blank and then f-lock it

              //start writing to file
              FILE * fptr = fopen(file_path, "w");
              if(fptr == NULL){
                perror("Error opening file for writing\n");
                continue;
              }
              else{
                //Create folder if it does not exist already
                mkdir(".storage", 0777);
                printf("Command Store Recognized \n");
                char file_line[BUFFER_SIZE];
                int itr = 0;
                int runthrough = numBytes / BUFFER_SIZE;
                while(itr < runthrough){
                  fread(file_line, sizeof(char), BUFFER_SIZE / sizeof(char), command);
                  printf("%s\n",file_line);
                  fwrite(file_line, sizeof(char), BUFFER_SIZE, fptr);
                  puts("devided once");
                  itr++;
                }
                fread(file_line, sizeof(char), (numBytes % BUFFER_SIZE) / sizeof(char), command);
                printf("%s\n",file_line);
                fwrite(file_line, sizeof(char), (numBytes % BUFFER_SIZE), fptr);
                puts("asdfsdf");
              }
              fclose(fptr);


      		  write(sock , "FILE Read\n" , strlen("FILE Read"));
            puts("sent: file read");

          //if there is a digit before end
            //store index of first digit in string of digits
            //convert num to int and save as bytes
          //else
            //printf("ERROR: Incorrect Syntax For COMMAND\n");
            //return
          //save filename(dont forget about the space between filename and bytes)
          //either create and call a store function or write it here

        }
        //else if !string compare(client_message(first 5 char) "READ ")
          //if there is a digit before end
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
          //if there is a char before \n
            //save filename
          //else
            //printf("ERROR: Incorrect Syntax For COMMAND\n");
            //return
          //call delete function



      }
      else{
        puts(temp);
        printf("ERROR: Incorrect Syntax For COMMAND\n");
        write(sock , "ERROR: Incorrect Syntax For COMMAND\n" , strlen("ERROR: Incorrect Syntax For COMMAND\n"));
      }
    }
      else {
        puts("Client closed it's socket....terminating");
        fflush(stdout);
        return 0;
      }
      write(sock , "AWK\n" , strlen("AWK\n"));

  		puts("Sent: ACK");
  		fflush(stdout);


  		//clear the message buffer
  		memset(temp, 0, 2000);
    }

    return 0;
}

int main(int argc , char *argv[])
{
  //declare memory array here(32 slots with 1024 bits)
    // allocate 32,768 with calloc
    //then turn into array of char of that size
    char* test = calloc(32,1024);

  //declare page table here (32 slots filled with custom struct page)
  struct page pageTable[32];
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
