#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int read_chunk_size(int s);
struct sockaddr_in server;
char request[200];
char response[300000];

unsigned char ip[4]={216,58,212,99}; //google
//unsigned char ip[4]={17,172,224,111}; //apple

struct header {
   char *k;
   char *v;
} h[100];

int main() {
   int i,j,k,n,s, flag;
   char* content_p;
   int chunk_size;
   int content_size = 0;
   char is_trasmission_chunked = 0;

   //connect and send request
   server.sin_family = AF_INET;
   server.sin_port = htons(80);
   server.sin_addr.s_addr = *(unsigned int *)ip;
   s = socket(AF_INET, SOCK_STREAM, 0);
   if(s == -1) {
      printf("Errore socket");
      return 1;
   }
   if(connect(s,(struct sockaddr*)&server,sizeof(struct sockaddr_in))==-1) {
      printf("Errore connessione");
      return 1;
   }
   sprintf(request, "GET / HTTP/1.1\r\nHost:www.google.it\r\n\r\n");
   write(s, request, strlen(request));

   //PARSE HEADERS
   k = 0;
   j=0;
   h[0].k = response;
   //flag used to correctly parse date header
   flag = 0;
   while(read(s, response+j, 1)) {
      if(response[j] == '\n' && response[j-1] == '\r') {
         response[j-1] = 0;
         flag = 0;
         //no more headers
         if(h[k].k[0] == 0) break;

         h[++k].k = response + j +1;
      }
      if(response[j] == ':' && (!flag)) {
         flag = 1;
         h[k].v = response+j+1;
         response[j] = 0;
      }
      j++;

   }

   printf("\n\n========HEADERS============\n\n");
   for(i=0;i<k;i++) {
      if(!strcmp(h[i].k, "Transfer-Encoding") && !strcmp(h[i].v, " chunked"))
        is_trasmission_chunked = 1;
   }


   //GET CHUNCKED CONTENT
   /*
   Model:
   get_chunk_size()
   while(chunk_size > 0)
      consume_data()
      get_chunk_size()
   */
   if(is_trasmission_chunked) {
     content_p = (char*)calloc(20, sizeof(char));
     chunk_size = read_chunk_size(s);

     while (chunk_size > 0) {
       //consume chunk data
       content_p = realloc(content_p, content_size + chunk_size);
       if(content_p == NULL) return 1;

       for(i=0; i < chunk_size; i++) {
         if(!read(s, content_p + content_size++, 1)) return 1;
       }

       //consume CRLF
       for(i=0; i<2; i++)
          if(!read(s, &n, 1)) return 1;

       //read next chunk len
       chunk_size = read_chunk_size(s);
     }

     //print data
     for(i=0; i < content_size; i++)
     printf("%c", content_p[i]);
     free(content_p);
   }



}

/*
Given a socked fd that will send a chunk header, consume the header and return
chunk size.

param s: socket file descriptor
*/
int read_chunk_size(int s) {
  char* chunk_header = malloc(1000 * sizeof(char));
  int i = 0;

  while (read(s, chunk_header + i, 1)) {
    if(chunk_header[i] == ';')
      chunk_header[i] = 0;
    if(chunk_header[i] == '\n' && chunk_header[i-1] == '\r') {
      chunk_header[i-1] = 0;
      return (int)strtol(chunk_header, NULL, 16);
    }
    i++;
  }
}
