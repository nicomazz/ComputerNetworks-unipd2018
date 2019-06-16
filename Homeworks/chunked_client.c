#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
extern int h_errno;
#include <stdio.h>

#define PORT 8001

const char *host = "www.padovanet.it";
char request[10000];
char buffer[100000];
char content[100000];

struct sockaddr_in saddr;
struct hostent *he;

struct header {
   char *k;
   char *v;
} h[100];

int main() {
   int i, j, s, k, n;
   char flag, isChunked;
   char ch;

   // send server req
   s = socket(AF_INET, SOCK_STREAM, 0);

   he = gethostbyname(host);
   if (he == NULL) {
      printf("Error resolving host name.\n");
      return 1;
   }

   saddr.sin_family = AF_INET;
   saddr.sin_port = htons(80);
   saddr.sin_addr.s_addr = *(unsigned int *)he->h_addr;

   if (-1 == connect(s, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in))) {
      printf("Error connecting to host.\n");
      return 1;
   }

   sprintf(request, "GET / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n", host);
   write(s, request, strlen(request));

   // parse headers
   k = 0;
   j = 0;
   flag = 0;
   h[0].k = buffer;
   while (read(s, buffer + j, 1)) {
      if (buffer[j] == '\n' && buffer[j - 1] == '\r') {  // new header key found
         buffer[j - 1] = 0;                              // add string terminator for prev header / request
         if (h[k].k[0] == 0)                             // double CRLF found -> no more headers
            break;
         h[++k].k = buffer + j + 1;  // point to the header key
         flag = 0;
      }
      if (buffer[j] == ':' && !flag) {
         flag = 1;  // avoid recognizing header value's ':' as separator
         buffer[j] = 0;
         h[k].v = buffer + j + 1;
      }
      j++;
   }

   // check if trasmission is chunked
   for (i = 0; i < k; i++) {
      //printf("%s --> %s\n", h[i].k, h[i].v);
      if (!strcmp(h[i].k, "Transfer-Encoding") && strcasestr(h[i].v, "chunked"))
         isChunked = 1;
   }

   if (isChunked) {
      int total_content_size = 0;

      long int chunk_size = 0, i = 0;  // i will store total content size
      n = 0;
      // read first chunk size
      char *chunk_size_p = buffer + j;
      while (read(s, buffer + j, 1)) {
         if (buffer[j] == ':')  // replace separators with string terminators
            buffer[j] = 0;
         if (buffer[j] == '\n' && buffer[j - 1] == '\r')
            break;
         j++;
      }
      chunk_size = strtol(chunk_size_p, NULL, 16);  // convert from b16 to long

      while (chunk_size > 0) {
         printf("chunk size: %d\n", chunk_size);
         total_content_size += chunk_size;

         // print chunck data
         for (i = 0; i < chunk_size + 2; i++) {
            if (!read(s, &ch, 1)) {
               printf("Error reading\n");
               return 1;
            }
            printf("%c", ch);
         }

         // read successive chunk size
         j = 0;
         char *chunk_size_p = buffer;
         while (read(s, buffer + j, 1)) {
            if (buffer[j] == ':')  // replace separators with string terminators
               buffer[j] = 0;
            if (buffer[j] == '\n' && buffer[j - 1] == '\r')
               break;
            j++;
         }
         chunk_size = strtol(chunk_size_p, NULL, 16);
      }

      //printf("Total content size: %d\n", total_content_size);

   } else {
      printf("Server response is not chunked.\n");
   }

   close(s);

   return 0;
}
