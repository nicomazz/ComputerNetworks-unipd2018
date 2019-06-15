#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 8004

struct sockaddr_in server, client;

struct header {
   char* k;
   char* v;
} h[100];

char request[10000];
char response[10000];

int main() {
   int i,j,k,s,cs,n,ch;
   int flag;
   int len;
   char *method, *path, *ver;
   char *credentials;
   s = socket(AF_INET, SOCK_STREAM, 0);
   if(s==-1) { printf("Error opening socket"); exit(1); }

   server.sin_family = AF_INET;
   server.sin_port = htons(PORT);
   if(-1 == bind(s, (struct sockaddr*)&server, sizeof(struct sockaddr))) { printf("Error binding socket."); exit(1); }

   if(-1 == listen(s, 10)) { printf("Error listening on socket."); exit(1); }
   len = sizeof(client);
   while(1) {
      cs = accept(s, (struct sockaddr*)&client, &len);

      //PARSE HEADERS
      j = 0;
      k = 0;
      h[0].k = request;
      flag = 0;
      while(ch = read(cs, request+j, 1)) {
         //printf("%c", request[j]);
         if(request[j] == '\n' && request[j-1] == '\r') {
            request[j-1] = 0;
            if(h[k].k[0] == 0)
               break;

            flag = 0;
            h[++k].k = request + j + 1;

         }
         if(request[j] == ':' && !flag) {
            flag = 1;
            request[j] = 0;
            h[k].v = request + j + 1;
         }
         j++;
      }
      printf("=========HEADERS==========\n");
      for(i = 0; i < k; i++) {
         printf("%s --> %s\n", h[i].k, h[i].v);
      }

      //parse request
      //ex GET / HTTP/1.1
      i = 0;
      method = &h[0].k[0];
      for(; h[0].k[i] != ' '; i++);
      h[0].k[i] = 0;
      i++;
      path = &h[0].k[i];
      for(; h[0].k[i] != ' '; i++);
      h[0].k[i] = 0;
      i++;
      ver = &h[0].k[i];

      printf("\n\nmethod: %s \npath: %s \nver: %s\n",  method, path, ver);

      //find auth header
      flag = 0;
      for(i=0;i<k;i++) {
         if(!strcmp("Authorization", h[i].k)) {
            flag = 1;
            break;
         }
      }

      //get credentials
      if(flag) {
         //printf("%s", h[i].v);
         j = 0;
         for(;h[i].v[j] == ' '; j++);
         for(;h[i].v[j] != ' '; j++);
         for(;h[i].v[j] == ' '; j++);
         credentials = &h[i].v[j];
         printf("\n\nCREDENTIALS: %s\n\n", credentials);

      }

      if(!flag || strcmp("QWxhZGRpbjpvcGVuIHNlc2FtZQ==", credentials)) {
         sprintf(response, "HTTP/1.1 401 Unauthorized\r\nConnection:close\r\nWWW-Authenticate: Basic realm=\"WallyWorld\"\r\n\r\n");
         write(cs, response, strlen(response));
      } else {
         sprintf(response, "HTTP/1.1 200 OK\r\n\r\nbella\r\n\r\n");
         write(cs, response, strlen(response));
      }
      close(cs);
   }

   return 0;
}
