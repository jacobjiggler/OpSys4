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
#include <math.h>

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
  time_t lastEdited;
};


//declare memory array here(32 slots with 1024 bits)
  // allocate 32,768 with calloc
  //then turn into array of char of that size
char memory[32][1024];

//declare page table here (32 slots filled with custom struct page)
struct page pageTable[32];

int findLeastRecentyUsed (struct page* pageItr){
  const int tableSize = 32;
  time_t oldestTime;
  int oldestIndex =0;
  localtime(&oldestTime);
  int i=0;
  for(i=0; i< tableSize; i++){
    if(pageTable[i].pageNum == -1)
      return i;
    if (pageItr[i].lastEdited < oldestTime){
      oldestTime = pageItr[i].lastEdited;
      oldestIndex =i;
    }
  }
  return oldestIndex;
}
int checkForFileInPageTable(struct page* pageTable, char * filename){
  //check if a page exists with same offset
  const int tableSize = 32;
  int i;
  for(i =0; i< tableSize; i++){
    if (!strcmp(pageTable[i].filename,filename)){
      localtime(&pageTable[i].lastEdited);
      return i;
    }
  }
  return -1;
}
int transferPage(int index, char * filepath, int pageNum){
  //add locks


  FILE* fptr = fopen(filepath, "r");
  if (fptr==NULL){
    perror("ERROR Could not open file for reading\n");
  }
  else{
    int startByte = 1024 * index;
    fseek(fptr, startByte,SEEK_SET);
    //char* = memory[index*1024];

    fread(memory[index], sizeof(char), 1024, fptr);
    puts(memory[index]);
    //save the memory
    //update pagetable
    //remove locks
    return index;
  }
  //remove locks
  return -1;
}

int writeToClient(int index, int offset, int numBytes, int sock){
  //lock
  //code
  //print stuff;

  char file_content[numBytes];
  char output[numBytes+100];
  int i;
  for(i = 0; i < numBytes; i++){
  	file_content[i] = memory[index][offset%1024+i];
  }
  snprintf(output, sizeof(output), "AWK %d\n%s", numBytes, file_content);
  write(sock , output , strlen(output));

  return -1;
}




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
        char *dest;
        dest = strtok(temp, " ");
        printf("[thread %lu] Rcvd: %s\n",(unsigned long)pthread_self(), temp);


        if (strcmp(temp, "DIR\n") ==0 || strcmp(temp, "DIR\r\n") ==0){
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
		  FILE * file = fopen(file_path, "r");
          //read function
          //add locks
          //wrote this when super tired need to recheck work
		  if( access( file_path, F_OK ) == -1 ) {
            // file exists
            puts(file_path);
			      printf("[thread %lu] Sent: ERROR NO SUCH FILE\n",(unsigned long)pthread_self());
            write(sock , "Error: NO SUCH FILE\n" , strlen("Error: NO SUCH FILE\n"));
            perror("Error: NO SUCH FILE\n");

          }
		  else{
			if (flock(fileno(file), LOCK_SH)!=0){
				puts("file lock not achieved");
				continue;
			}
			struct stat st;
			stat(file_path, &st);
			int size = st.st_size;
			int difference = byteOffset % 1024;
			int firstPageSize = 1024 - difference;
			if(firstPageSize > readLength){
				firstPageSize = readLength;
			}
			struct page* frame[4];
			int index;
      int pageNum = checkForFileInPageTable(pageTable, file_name);
			if(pageNum != -1){
        //use preexisting page if possible to write to beginning bytes
        localtime(&pageTable[pageNum].lastEdited);
        index = 0;
        writeToClient(pageNum, byteOffset, firstPageSize, sock);
			}
			else{
        pageNum = findLeastRecentyUsed(pageTable);
        frame[0] = &pageTable[pageNum];
        transferPage(pageNum, file_path, 0);
        writeToClient(pageNum, byteOffset, firstPageSize, sock);
        localtime(&pageTable[pageNum].lastEdited);
        index = 1;
}

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
		  flock(fileno(file), LOCK_UN);
		  }

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
				//returnwhat
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




int main(int argc , char *argv[])
{

	int i;
	for(i = 0; i < 32; i++){
		pageTable[i].filename[0] = '\0';
		pageTable[i].pageNum = -1;
	}

  //char memory = calloc(32,1024);

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
