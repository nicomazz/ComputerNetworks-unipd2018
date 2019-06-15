#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PORT 8000

int build_chunck(char *b, int len);

struct header{
  char * n;
  char * v;
}h[100];

FILE * f;

struct sockaddr_in myaddr,remote_addr;

char  filename[100],command[1000];
char request[10000];
char response[10000];
char chunked_buffer[10000];
char temp[10000];

int yes=1;

int main() {
  char *method, *path, *ver;
  int c;
  int duepunti;
  int i,k,j,n,s,s2;
  int length, lungh;

  s = socket(AF_INET, SOCK_STREAM,0);
  if ( s == -1) {
    perror("socket fallita");
    return 1;
  }
  if ( setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 ) {perror("setsockopt fallita"); return 1; }

  myaddr.sin_family=AF_INET;
  myaddr.sin_port=htons(PORT);

  if (bind(s,(struct sockaddr *)&myaddr,sizeof(struct sockaddr_in))==-1){ perror("bind fallita"); return 1; }
  if (listen(s,10)==-1){ perror("Listen Fallita");return 1; }

  lungh=sizeof(struct sockaddr_in);
  while(1){
    s2 = accept(s,(struct sockaddr *) &remote_addr,&lungh);
    // printf("%d\n",s2);
    if (s2 == -1) {perror("accept fallita"); return 1;}

    // parse headers
    j=0;k=0;
    h[0].n=request;
    while(read(s2,request+j,1)){
      // printf("%c(%d)",request[j],request[j]);
      // printf("%c",request[j]);
      if((request[j]=='\n') && (request[j-1]=='\r')){
        printf("LF\n");
        request[j-1]=0;
        duepunti=0;
        if(h[k].n[0]==0) break;
        h[++k].n=request+j+1;
      }
      if(request[j]==':' && (!duepunti)){
        // printf("Due punti\n");
        duepunti=1;
        request[j]=0;
        h[k].v=request+j+1;
      }
      j++;
    }
    if (k==0) {close(s2);continue;}

    //perror("Read\n");
    printf("Command-Line: %s\n",h[0].n);
    printf("===== HEADERS ==============\n");
    for(i=1;i<k;i++){
      printf("%s --> %s\n", h[i].n, h[i].v);
    }

    // parse command line
    method = h[0].n;
    for(i=0;h[0].n[i]!=' ';i++); h[0].n[i]=0;i++;
    for(;h[0].n[i]==' ';i++);
    path = &h[0].n[i];
    for(;h[0].n[i]!=' ';i++); h[0].n[i]=0; i++;
    for(;h[0].n[i]==' ';i++);
    ver = &h[0].n[i];

    printf("\n%s %s %s\n",method,path,ver);

    if (!strncmp("/exec/",path,6)){ //Funzione Gateway
      sprintf(command,"%s > output",path+6);
      printf("Eseguo comando: %s",command);
      system(command);
      sprintf(filename,"output");
    }
    else{
      strcpy(filename,path+1);
    }
    f=fopen(filename,"r");
    if (f == NULL){
      sprintf(response,"HTTP/1.1 404 Not Found\r\nConncection:close\r\n\r\n");
      write(s2,response,strlen(response));
    }
    else {
      sprintf(response,"HTTP/1.1 200 OK\r\nConnection:close\r\nTransfer-Encoding:chunked\r\nContent-Type: text/plain\r\n\r\n");
      write(s2,response,strlen(response));

      i = 0;
      while ( (c = fgetc(f))!=EOF)
        temp[i++] = c;

      int size = 0;

      // send a response made of 2 chunks
      size = build_chunck(temp, i/2);
      write(s2, chunked_buffer, size);

      size = build_chunck(temp+i/2, i - i/2);
      write(s2, chunked_buffer, size);

      // send last chunk
      size = build_chunck(0, 0);
      write(s2, chunked_buffer, size);

      fclose(f);
    }
    close(s2);
  }
  close(s);
}

// returns size of built chunk
int build_chunck(char *b, int len) {
  // put chunk size
  sprintf(chunked_buffer, "%x\r\n", len);
  // put chunk data
  int from = strlen(chunked_buffer);
  int i = 0;
  for (i; i < len; i++)
    chunked_buffer[from+i] = b[i];
  chunked_buffer[from + (i++)] = '\r';
  chunked_buffer[from + (i++)] = '\n';
  chunked_buffer[from + i] = 0;
  return from + i;

}
