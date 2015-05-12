//testing of partially correct commands(store without any data afterwards)
//READ
//DELETE
//Page Table Operations
//assure offset and length are < size of file





#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
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
#include <time.h>

#define BUFFER_SIZE 1024
#define PORT 8765
#define FRAME_SIZE 1024
#define FRAMES = 32
#define FRAMES_PER_FILE 4

//declare mutex locks up here



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
        printf("[thread %lu] Rcvd: %s\n",(unsigned long)pthread_self(), temp);

        char *dest;
        dest = strtok(temp, " ");
        if (strcmp(temp, "DIR\n") ==0){
          printf("Command DIR Recognized\n");
          //call dir function

          DIR *dir;
          struct dirent *ent;
          char * dirname = ".storage";
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
            closedir(dir);
            dir = opendir (dirname);
            while ((ent = readdir (dir)) != NULL) {
              if (!strcmp(ent->d_name,"..") || !strcmp(ent->d_name,".")){
                continue;
              }
              strcat(output, ent->d_name);
              strcat(output, "\n");
            }
            write(sock , output , strlen(output));
            closedir(dir);
            write(sock , "AWK\n" , strlen("AWK\n"));
            puts("Sent: ACK");
          } else {
            /* could not open directory */
            printf ("couldn't open directory \n");
          }
        }

        else if (!strcmp(dest,"STORE")){
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
    			int numBytes = atoi(bytes_size);
          char file_path[1000];
          memset(file_path,0,9);
          strcat(file_path, ".storage/");
          strcat(file_path, file_name);

          if( access( file_path, F_OK ) != -1 ) {
              // file exists
              write(sock , "Error: FILE EXISTS\n" , strlen("Error: FILE EXISTS\n"));
              perror("Error: FILE EXISTS\n");

          } else {


              //start writing to file
              FILE * fptr = fopen(file_path, "w");


              if(fptr == NULL){
                write(sock , "Error: opening file for writing\n" , strlen("Error opening file for writing\n"));
                perror("Error: opening file for writing\n");
                printf("ERROR: %s\n", strerror(errno));
                continue;
              }
              else{
                flock(fileno(fptr), LOCK_EX);
                printf("Command Store Recognized \n");
                char file_line[BUFFER_SIZE];
                int itr = 0;
                int runthrough = numBytes / BUFFER_SIZE;
                while(itr < runthrough){
                  fread(file_line, sizeof(char), BUFFER_SIZE / sizeof(char), command);
                  //printf("%s\n",file_line);
                  fwrite(file_line, sizeof(char), BUFFER_SIZE / sizeof(char), fptr);
                  puts("devided once");
                  itr++;
                }
                fread(file_line, sizeof(char), (numBytes % BUFFER_SIZE) / sizeof(char), command);
                printf("%s\n",file_line);
                fwrite(file_line, sizeof(char), (numBytes % BUFFER_SIZE)/ sizeof(char), fptr);
              }
              fclose(fptr);
              flock(fileno(fptr), LOCK_UN);


            printf("[thread %lu] Transferred file (%d bytes)\n",(unsigned long)pthread_self(), numBytes);
            write(sock , "AWK\n" , strlen("AWK\n"));
            puts("Sent: ACK");



        }



        }
        else if(!strcmp(dest,"READ")){
          puts("Received READ");
          char file_name [100];
          char bytes_size [100];
          char length [100];
          int f_pos = 0;
          int b_pos = 0;
          int l_pos = 0;
          int pos = 5;
          while(temp[pos] != ' '){
            file_name[f_pos] = temp[pos];
            //putchar(file_name[f_pos]);
            f_pos++;
            pos++;
          }
          pos++;
          while(temp[pos] != ' '){
            bytes_size[b_pos] = temp[pos];
            b_pos++;
            pos++;
          }
          pos++;
          while(temp[pos] != '\n'){
            length[l_pos] = temp[pos];
            l_pos++;
            pos++;
          }
          printf("f_pos %d\n", f_pos);
          printf("b_pos %d\n", b_pos);
          printf("l_pos %d\n", l_pos);

          file_name[f_pos] = '\0';
          bytes_size[b_pos] = '\0';
          length[l_pos] = '\0';
    			int byteOffset = atoi(bytes_size);
          int readLength = atoi(length);
          char file_path[1000];
          memset(file_path,0,9);
          strcat(file_path, ".storage/");
          strcat(file_path, file_name);
          //read function
          //add locks
          //wrote this when super tired need to recheck work

          //check if exists
          //if it doesnt
            //[thread 134559232] Sent: ERROR NO SUCH FILE
          //if it does
            //multilock with read only
            //get filesize
            //difference = offset % 1024
            //firstPageSize = 1024 - difference
            //if firstPageSize > length
              //firstPageSize = length
            //struct page* pages[4];
            //int index
            //use preexisting page if possible to write beginning bytes
              //update last edited
              //index = 0
              //write message of last FirstPageSize bytes to client(needs a bunch of code between these 2 lines)
              //print stuff
            //else
              //update last edited
              //do page check and set page[0] then write bytes
              //pages[0] = right page
              //index = 1;
              //strcpy the whole 1024 bytes into memory
              //write message of last FirstPageSize bytes to client(needs a bunch of code between these 2 lines)
              //print stuff



            //byteOffset = byteOffset + firstPageSize
            //bytesRead = firstPageSize
            //while(index < 4 && bytesRead < length){
              //check if 1024 > (length - bytesRead)
              //if it is{
                //bytesToWrite = length - bytesRead
              //else
                //bytesToWrite = 1024
              //check if in preexisting page and use that if you can
                //update bytesRead and offset
                //print stuff
              //else
                //if index < 4
                  //do page check and assign new page
                  //pages[index%4] = right page
                //else
                  //rewrite page already in pages
                //strcpy those bytes into memory
                //write message of those bytes to client
                //update bytesRead
                //updateoffset
                //print stuff
                //index++
            //}


      }
      else if(!strcmp(dest,"DELETE")){
        puts("Received DELETE");
          //if it exists
            //add flock
            //search all of pagetable for it.
            //unallocate any with it
            //lock them before you unallocate them.
            //delete the actual file

          //else
            //printf("ERROR: File doesn't exist\n");
            //return
        }




      else{
        printf("contents %s\n", temp);
        printf("ERROR: Incorrect Syntax For COMMAND\n");
        write(sock , "ERROR: Incorrect Syntax For COMMAND\n" , strlen("ERROR: Incorrect Syntax For COMMAND\n"));
      }
    }
      else {
        puts("Client closed it's socket....terminating");
        fflush(stdout);
        return 0;
      }

  		fflush(stdout);


  		//clear the message buffer
  		memset(temp, 0, 2000);
    }

    return 0;
}
//struct to hold page info
struct page {
  char filename[1000];
  int pageNum;
  //Will hold the number in which it was edited
  time_t lastEdited;
};

int findLeastRecentyUsed (struct page* pageTable){
  //RETURNS -1 as error case
  const int tableSize = 32;
  time_t oldestTime;
  int oldestIndex;
  localtime(&oldestTime);
  int i;
  for(i =1; i< tableSize; i++){
    if(pagetable[i]==NULL) return -1
    if (pageTable[i].lastEdited < oldestTime){
      oldestTime = pageTable[i].lastEdited;
      oldestIndex =i;
    }
  }
  return oldestIndex;
}
int checkForPage(char filename, int offset){
  //check if a page exists with same offset
  //if it exists
    //update lastEdited
    //return its index
  // else return -1

  return -1;
}
int transferPage(int index, char * filename, int pageNum, char * buffer){

  return -1
}
int main(int argc , char *argv[])
{
  //declare memory array here(32 slots with 1024 bits)
    // allocate 32,768 with calloc
    //then turn into array of char of that size
    char* memory = calloc(32,1024);

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
  //Create folder if it does not exist already
  mkdir(".storage", 0777);
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
