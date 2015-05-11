#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#define MAXBUF 10*1024

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    int len;

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("creating socket failed");
    }
    puts("Socket created sucessfully");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8765 );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    //keep communicating with server
    while((len = recv(sock, server_reply, sizeof(server_reply), 0)) > 0)
    {
        printf("%.*s", len, server_reply);

        printf("Enter message : ");
        if (fgets(message, sizeof(message), stdin) == NULL)
            break;
		char substr[100];
		int pos = 0;
		while (pos < 5){
			substr[pos] = message[pos];
			pos++;
		}
		puts(substr);
		if (strcmp(substr, "STORE")==0)
		{
			puts("SUCCESS");
			int file_send = 1;
			char file_name [100];
			char bytes_size [100];
			int f_pos = 0;
			int b_pos = 0;
			pos = 6;
			while(message[pos] != ' '){
				file_name[f_pos] = message[pos];
				f_pos++;
				pos++;
			}
			pos++;
			while(message[pos] != '\n'){
				bytes_size[b_pos] = message[pos];
				b_pos++;
				pos++;				
			}
			printf("%d\n", f_pos);
			printf("%d\n", b_pos);
			file_name[f_pos] = '\0';
			file_name[b_pos] = '\0';
			FILE *fp = fopen("a.txt", "rb");
			if(fp == NULL){
				puts("error opening file:");
			}
			puts(file_name);
			puts(bytes_size);
			int n = 0; //total bytes read/written
			int n_bytes = atoi(bytes_size);
			char buffer[n_bytes];
			while(!feof(fp)){
				int read = fread(buffer, n_bytes, 1, fp);
				if(read < 0){
					puts("error reading file");
				}
				//puts(buffer);
				//write(sock ,buffer,read);
				n+=read;
				strcat(message, buffer);
				//puts(message);
			}
			fclose(fp);	
		}

		
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
    }

    close(sock);

    return 0;
}