# Computer networks - unipd 2018

Tips and code to easily pass the computer networks practical exam ("Reti di calcolatori") in Padua.
You can find everything that is in this readme through `man`, and the various RFCs. I've done it just for a fast reference.

<details><summary>TOC</summary>
<p>

<!-- MarkdownTOC autolink="true" -->

- [Useful RFCs](#useful-rfcs)
- [Useful links](#useful-links)
- [Something to understand before trying the past exams](#something-to-understand-before-trying-the-past-exams)
   - [Data types and endianess](#data-types-and-endianess)
   - [Ethernet frame](#ethernet-frame)
   - [IP datagram](#ip-datagram)
   - [TCP segment](#tcp-segment)
   - [Checksum calculation](#checksum-calculation)
   - [Convert int IP address in string](#convert-int-ip-address-in-string)
- [Editor for the exam](#editor-for-the-exam)
- [Past exams](#past-exams)
   - [19 June 2018 \(ping.c\)](#19-june-2018-pingc)
   - [20 June 2018 \(ping.c\)](#20-june-2018-pingc)
   - [20 June 2016](#20-june-2016)
   - [1 \(tcp16.c\)](#1-tcp16c)
   - [2 \(wp16.c\)](#2-wp16c)
   - [3 \(ws18.c\)](#3-ws18c)
   - [15 July 2016 \(ping.c\)](#15-july-2016-pingc)
   - [24 July 2015 \(wc18.c\)](#24-july-2015-wc18c)
   - [26 June 2014](#26-june-2014)
- [Misc](#misc)
   - [HTTP-date](#http-date)
   - [rewind](#rewind)
   - [Read a file and forward it](#read-a-file-and-forward-it)
   - [Read from a socket and forward to another socket \(like a proxy\)](#read-from-a-socket-and-forward-to-another-socket-like-a-proxy)
   - [Check if the target ip is in our network](#check-if-the-target-ip-is-in-our-network)
   - [gethostbyname](#gethostbyname)
   - [socket creation, binding for listening and writing \(like http requests\)](#socket-creation-binding-for-listening-and-writing-like-http-requests)
   - [RAW socket creation, binding for listening and writing \(to deal with tcp/icmp/arp requests directly\)](#raw-socket-creation-binding-for-listening-and-writing-to-deal-with-tcpicmparp-requests-directly)
   - [How to printf the various things](#how-to-printf-the-various-things)

<!-- /MarkdownTOC -->
</p>
</details>


## Useful RFCs
- [RFC791](https://tools.ietf.org/html/rfc791) IP
- [RFC792](https://tools.ietf.org/html/rfc792) ICMP
- [RFC793](https://tools.ietf.org/html/rfc793) TCP
- [RFC826](https://tools.ietf.org/html/rfc826) ARP
- [RFC1945](https://tools.ietf.org/html/rfc1945) HTTP1
- [RFC2616](https://tools.ietf.org/html/rfc2616) HTTP1.1


## Useful links
- [Here](https://www.stefanoivancich.com/?p=1291) you can find a summary of the most important things to know to pass the exam. 
Anyway, maybe is better to study more in depth every topic.
- [C socket programming online book](http://alas.matf.bg.ac.rs/manuals/lspe/mode=1.html). There is more than needed, but still interesting.


## Something to understand before trying the past exams

An Ethernet frame (data link layer) contains an IP datagram (network layer) that can contains one of the following { tcp_segment (transport layer), icmp_packet } (for the purpose of this exam). An easy way to realize that is:

```c
eth = (struct eth_frame *)buffer;
ip = (struct ip_datagram *)eth->payload;

tcp = (struct tcp_segment *)ip->payload;
// or
icmp = (struct icmp_packet *)ip->payload; 
```
#### Data types and endianess
<details>
<summary>Data types and endianess</summary>

(depends on the architecture, but you can assume that the following is true for this exam)

- `unsigned char` : 1 byte
- `unsigned short`: 2 bytes
- `unsiged int` : 4 bytes

To transfer on the network is used Big endian. Most of the intel's cpus are little endian. To convert use this 2 functions that automatically understand if a conversion is needed:
-  `htonl(x)` or `htons(x)` to convert x from **H**ost **to** **N**etwork endianess, **l** if you have to convert a 4 bytes variable, **s** a 2 bytes one.
- `ntohl(x)` or `ntohs(x)` for the opposite. (You may notice that the implementation of htonx and ntohx is the same)
- if a variable is 1 byte long we don't have endianess problems (obviously)
 </details>

 
#### Ethernet frame
<details><summary>Ethernet frame</summary>
<p>

![Ethernet frame](https://upload.wikimedia.org/wikipedia/commons/thumb/4/42/Ethernet_frame.svg/800px-Ethernet_frame.svg.png)

```c
// Frame Ethernet
struct eth_frame {
   unsigned char dst[6]; // mac address
   unsigned char src[6]; // mac address
   unsigned short type;  // 0x0800 = ip, 0x0806 = arp
   char payload[1500];   //ARP or IP
 };
```
Thanks to the `type` we can understand where to forward it on the next level (2 examples are ip or arp)

</p>
</details>


#### IP datagram
<details><summary>IP datagram</summary>
<p>

![Ip datagram](http://www.danzig.jct.ac.il/tcp-ip-lab/ibm-tutorial/3376f11.gif)

Header length: check second half of `ver_ihl` attribute. Example: if it's '5', then the header length is **4** * 5 = 20 bytes.  
//todo add image
```c
// Datagramma IP
struct ip_datagram{
   unsigned char ver_ihl;    // first 4 bits: version, second 4 bits: (lenght header)/8
   unsigned char tos;        //type of service 
   unsigned short totlen;    // len header + payload
   unsigned short id;        // useful in case of fragmentation
   unsigned short flags_offs;//offset/8 related to the original ip package
   unsigned char ttl;
   unsigned char protocol;   // TCP = 6, ICMP = 1
   unsigned short checksum;  // only header checksum (not of payload). Must be at 0 before the calculation.
   unsigned int src;         // ip address
   unsigned int dst;         // ip address
   unsigned char payload[1500];
};
```

</p>
</details>


#### TCP segment

<details><summary>TCP segment</summary>
<p>

![tcp segment](https://i.ibb.co/WpSwRXL/Screen-Shot-2019-01-07-at-22-15-38.png)

Header (as defined here) length: `20`
```c
struct tcp_segment {
   unsigned short s_port;
   unsigned short d_port;
   unsigned int seq;        // offset in bytes from the start of the tcp segment in the stream (from initial sequance n)
   unsigned int ack;        // useful only if ACK flag is 1. Next seq that sender expect
   unsigned char d_offs_res;// first 4 bits: (header len/8)
   unsigned char flags;            // check rfc
   unsigned short win;      // usually initially a 0 (?)
   unsigned short checksum; // use tcp_pseudo to calculate it. Must be at 0 before the calculation.
   unsigned short urgp;            
   unsigned char payload[1000];
};
```
To calculate the checksum of a TCP segment is useful to define an additional structure (check on the relative RFC). Size of it, without the tcp_segment part
```c
struct tcp_pseudo{
   unsigned int ip_src, ip_dst;
   unsigned char zeroes;
   unsigned char proto;        // ip datagram protocol field (tcp = 6, ip = 1)
   unsigned short entire_len;  // tcp length (header + data)
   unsigned char tcp_segment[20/*to set appropriatly */];  // entire tcp packet pointer
};
```
To calculate the size of the entire tcp segment (or of the icmp), or more in general of the ip payload:
```c
unsigned short ip_total_len = ntohs(ip->totlen);
unsigned short ip_header_dim = (ip->ver_ihl & 0x0F) * 4;
int ip_payload_len = ip_total_len-ip_header_dim;
```
</p>
</details>


#### Checksum calculation

<details><summary>Checksum calculation</summary>
<p>

We can use this function both for the IP datagram and the TCP segment,
but we must take care about the `len` parameter.

- [ ] todo: take care about minimum size for tcp, and odd/even corner case

```c
unsigned short checksum( unsigned char * buffer, int len){
   int i;
   unsigned short *p;
   unsigned int tot=0;
   p = (unsigned short *) buffer;
   for(i=0;i<len/2;i++){
      tot = tot + htons(p[i]);
      if (tot&0x10000) tot = (tot&0xFFFF)+1;
   }
   return (unsigned short)0xFFFF-tot;
}
```
The 2 cases are: 
- IP: `ip->checksum=htons(checksum((unsigned char*) ip, 20));`
`
- TCP: 
```c
int TCP_TOTAL_LEN = 20;
struct tcp_pseudo pseudo; // size of this: 12
memcpy(pseudo.tcp_segment,tcp,TCP_TOTAL_LEN); 
pseudo.zeroes = 0;
pseudo.ip_src = ip->src;
pseudo.ip_dst = ip->dst;
pseudo.proto = 6;
pseudo.entire_len = htons(TCP_TOTAL_LEN); // may vary
tcp->checksum = htons(checksum((unsigned char*)&pseudo,TCP_TOTAL_LEN+12));
```


</p>
</details>

#### Convert int IP address in string

<details><summary>Convert int IP address in string</summary>
<p>

```c
#include <arpa/inet.h>

void print_ip(unsigned int ip){
   struct in_addr ip_addr;
   ip_addr.s_addr = ip;
   printf("%s\n",inet_ntoa(ip_addr));
}

```

</p>
</details>

## Editor for the exam
I advice VIM. And please, indent your code.
- `:wq` to save and quit. 
- Press `esc` 2 times if you don't understand what is happening
- `/query` to search for "query", `n` and `N` to search prev/next result

<details><summary>Put this in ~/.vimrc to save time:</summary>
<p>

```
" auto reformat when you pres F7
map <F7> mzgg=G`z

" F8 to save and compile creating np executable
map <F8> :w <CR> :!gcc % -o np -g <CR>
" F9 to execute
map <F9> :!./np <CR>

" make your code look nicer
set tabstop=3
set shiftwidth=3
set softtabstop=0 expandtab
set incsearch
set cindent

" Ctrl+shift+up/down to swap the line up or doen
nnoremap <C-S-Up> <Up>"add"ap<Up>
nnoremap <C-S-Down> "add"ap

" ctrl+h to hilight the last search
nnoremap <C-h> :set hlsearch!<CR>

set number
set cursorline
set mouse=a

set foldmethod=indent               
set foldlevelstart=99

let mapleader="\<space>"
nnoremap <leader>b :make <CR> :cw <CR>
```
</p>
</details>

#### Quickfix and makefile (aka how not waste time)

First of all create a `makefile` in the directory with the files to compile, like this:
```makefile
np : ws18.c
        gcc -o np ws18.c
```
Pay attention to put a tab before "gcc", and not spaces (if you have expandtab enabled in vim, use `ctrl=v tab`).
Here `np` is what you want generate (the executable), and `ws18.c` the file to compile. In the line below there is the command to call each time you write `:make` in vim.
Then, with the `.vimrc` provided above, press `space` (release it) and `b` (**b**uild).
The command will be excuted, and you will see in the bottom of your code the list of errors. 
You can fastly jump in the correct line by pressing enter in each entry.
To move between the top and bottom split press `CTRL+W` `W`. To close the bottom view (quickfix) `:q`, or `:cw`.

## Past exams
You can find the complete exam statement in the site at the beginning of this readme.
The complete code is in the folders.
 
<details>
<summary>Past exams</summary>   

#### 19 June 2018 (ping.c)
Implement TCP three way handshake (ACK+SYN).

**Tips**:
You can check with wireshark if your TCP checksum is correct or not.

- [ ] Is the option field to include?

---
#### 20 June 2018 (ping.c)
Implement echo reply only for icmp requests of a certain size

**Tips**:
You can calculate the size of an icmp message in this way:

```c
unsigned short dimension = ntohs(ip->totlen);
unsigned short header_dim = (ip->ver_ihl & 0x0F) * 4;
int icmp_dimension = dimension-header_dim;
```

---

#### 20 June 2016

#### 1 (tcp16.c)
Intercept the first received connection, and print sequence and acknowledge numbers of them. Then reconstruct the 2 streams in 2 different buffers, and print their content.

**Tips**: 
To intercept the end of the connection, just check if a package contains the FIN bit at 1 (after having filtered all the packages, maintaining only the ones belonging to the first connection).
Use the tcp sequence field to copy the contnet at the right offset in the 2 buffers.
DON'T DUPLICATE CODE.

---

#### 2 (wp16.c)
Modify the proxy to allow the request only from a pool of IP addresses, and allow only the transfer of files with text or html.

**Tips**:
Is better to first receive the response from the server in a buffer, then copy this content to another buffer to extract headers as always.
This because the header extraction procedure modifies the buffer.
If the condition of the Content-type is fullfilled then just forward the contnet of the initial buffer.

---

#### 3 (ws18.c)
Send HTTP response with a chunked body.

**Tips**: 
Add `Content-Type: text/plain\r\nTransfer-Encoding: chunked\r\n` to HTTP headers.
Then, to build each chunk to send, you can use something like:
<details>
<summary>Code to build a chunk</summary>

```c
int build_chunk(char * s, int len){
   sprintf(chunk_buffer,"%x\r\n",len); // size in hex
   // debug   printf("%d in hex: %s",len,chunk_buffer);
   int from = strlen(chunk_buffer);
   int i = 0;
   for (;i < len; i++)
      chunk_buffer[from+i] = s[i];
   chunk_buffer[from+(i++)] = '\r';
   chunk_buffer[from+(i++)] = '\n';
   chunk_buffer[i+from] = 0;
   return i+from;
}
```

</details>

---


#### 15 July 2016 (ping.c)
Implement an ICMP "Destination unreachable" that say that the port is unavailable

**Tips**: 
you have to send the package in response to a tcp connection. `icmp->type = 3`, `icmp->code=3`.
And remember to copy in the payload the content of the icmp original payload.

---

#### 24 July 2015 (wc18.c)
Implement the `Last-Modified` header of HTTP/1.0

**Tips**: 
Some useful time conversion functions in the misc section. It could also have been done without the need of these conversions.
The HTTP date format is `%a, %d %b %Y %H:%M:%S %Z`

---

#### 26 June 2014
1: content length (was already implemented)
2: trace (??)

- [ ] How does www.webtrace.com work?

---

</details>


# Misc

#### HTTP-date
The HTTP date format is `%a, %d %b %Y %H:%M:%S %Z`
<details>
<summary>Some useful functions to deal with HTTP time</summary>   

```c
char date_buf[1000];

char* getNowHttpDate(){
   time_t now = time(0);
   struct tm tm = *gmtime(&now);
   strftime(date_buf, sizeof date_buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
   printf("Time is: [%s]\n", date_buf);
   return date_buf;
}
// parse time and convert it to millisecond from epoch
time_t httpTimeToEpoch(char * time){
   struct tm tm;
   char buf[255];
   memset(&tm, 0, sizeof(struct tm));
   strptime(time,"%a, %d %b %Y %H:%M:%S %Z", &tm);
   return mktime(&tm);
}
// returns 1 if d1 < d2
unsigned char compareHttpDates(char * d1, char * d2){
   return httpTimeToEpoch(d1) < httpTimeToEpoch(d2);
}
unsigned char expired(char * uri, char * last_modified){
   char * complete_name = uriToCachedFile(uri);
   FILE * fp = fopen(complete_name,"r");
   if (fp == NULL) return 1;
   //read the first line
   char * line = 0; size_t len = 0;
   getline(&line,&len,fp);
   if (compareHttpDates(last_modified,line)) return 0;
   return 1;
   //todo read First line and compare
}
```

</details>


#### rewind 
`rewind(FILE*)` set the cursor at the beginning

#### Read a file and forward it
<details>
<summary>Read a file and forward it</summary>   

```c
FILE *fin;
if ((fin = fopen(uri + 1, "rt")) == NULL) { // the t is useless
   printf("File %s non aperto\n", uri + 1);
   sprintf(response, "HTTP/1.1 404 File not found\r\n\r\n<html>File non trovato</html>");
   t = write(s2, response, strlen(response));
   if (t == -1) {
      perror("write fallita");
      return -1;
      }
} else {
   content_length = 0;
   while ((c = fgetc(fin)) != EOF) content_length++; // get file lenght
   sprintf(response, "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: %d\r\n\r\n", content_length);
   printf("Response: %s\n", response);
   //send header
   t = write(s2, response, strlen(response));
   //rewind the file
   rewind(fin);
   //re-read the file, char per char
   while ((c = fgetc(fin)) != EOF) {
      //printf("%c", c);
      //sending the file, char per char
      if (write(s2, (unsigned char *)&c, 1) != 1) {
         perror("Write fallita");
      }
   }
   fclose(fin);
}
```

</details>


#### Read from a socket and forward to another socket (like a proxy)
```c
char car;
while (read(s3, &car, 1)) {
   write(s2, &car, 1);
   //   printf("%c",car);
}
```

#### Check if the target ip is in our network
```c
unsigned char targetip[4] = {147, 162, 2, 100};
unsigned int netmask = 0x00FFFFFF;
if ((*((unsigned int *)targetip) & netmask) == (*((unsigned int *)myip) & netmask))
      nexthop = targetip;
   else
      nexthop = gateway;
```

#### gethostbyname

from hostname (like www.google.it) to ip address
```c
/**
struct hostent {
   char  *h_name;            // official name of host 
   char **h_aliases;         // alias list 
   int    h_addrtype;        // host address type 
   int    h_length;          // length of address 
   char **h_addr_list;       // list of addresses 
}
#define h_addr h_addr_list[0] // for backward compatibility 
*/
struct hostent *he;
he = gethostbyname(hostname);
printf("Indirizzo di %s : %d.%d.%d.%d\n", hostname,
       (unsigned char)(he->h_addr[0]), (unsigned char)(he->h_addr[1]),
       (unsigned char)(he->h_addr[2]), (unsigned char)(he->h_addr[3]));
```

#### socket creation, binding for listening and writing (like http requests)
For listening:

<details>
<summary>Socket creation and options setting</summary>   

```c
int s = socket(AF_INET, // domain: ipv4
/*
SOCK_STREAM     Provides sequenced, reliable, two-way, connection-based  byte  streams.   An  out-of-band  data
                       transmission mechanism may be supported.
SOCK_DGRAM      Supports datagrams (connectionless, unreliable messages of a fixed maximum length).
SOCK_RAW        Provides raw network protocol access.
*/
   SOCK_STREAM,         // type: stream
   0);                  // protocol (0=ip), check /etc/protocols
if (s == -1) {
   perror("Socket Fallita");
   return 1;
}
// https://stackoverflow.com/questions/3229860/what-is-the-meaning-of-so-reuseaddr-setsockopt-option-linux
// SO_REUSEADDR allows your server to bind to an address which is in a TIME_WAIT state.
int yes = 1;
if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
   perror("setsockopt");
   return 1;
}
```

</details>

<details>
<summary>Binding to a local port</summary>   

```c
struct sockaddr_in indirizzo;
indirizzo.sin_family = AF_INET;
indirizzo.sin_port = htons(8987);
indirizzo.sin_addr.s_addr = 0;

t = bind(s, (struct sockaddr *)&indirizzo, sizeof(struct sockaddr_in));
if (t == -1) {
   perror("Bind fallita");
   return 1;
}
t = listen(s, 
// backlog defines the maximum length for the queue of pending connections.
   10);
if (t == -1) {
   perror("Listen Fallita");
   return 1;
}
```

</details>

<details>
<summary>Accepting a connection, and reading the actual content of the buffer</summary>   

```c
int lunghezza = sizeof(struct sockaddr_in);
// the remote address will be placed in indirizzo_remoto
s2 = accept(s, (struct sockaddr *)&indirizzo_remoto, &lunghezza);
if (s2 == -1) {
   perror("Accept Fallita");
   return 1;
}
// now we can read in this way:
char buffer[10000];
int i;
for (i = 0; (t = read(s2, buffer+i, 1)) > 0; i++); // ps. it's not a good way
// if the previous read returned -1
if (t == -1) {
   perror("Read Fallita");
   return 1;
}
```

</details>

At the end, remember to close all the sockets with `close(s)` (where s in the socket you want to close)


#### RAW socket creation, binding for listening and writing (to deal with tcp/icmp/arp requests directly)

```c
int s = socket(
//AF_PACKET       Low level packet interface       packet(7)
   AF_PACKET, 
//SOCK_RAW        Provides raw network protocol access.
   SOCK_RAW, 
// When protocol is set to htons(ETH_P_ALL), then all protocols are received.
   htons(ETH_P_ALL));

unsigned char buffer[1500];
bzero(&sll, sizeof(struct sockaddr_ll));
struct sockaddr_ll sll;
sll.sll_ifindex = if_nametoindex("eth0");
len = sizeof(sll);
int t = sendto(s, //socket
   buffer, //things to send
   14 + 20 + 28, // len datagram
   0, //flags
   (struct sockaddr *)&sll, // destination addr
   len // dest addr len
);

// to receive
t = recvfrom(s, buffer, 1500, 0, (struct sockaddr *)&sll, &len);
if (t == -1) {
   perror("recvfrom fallita");
   return 1;
}

```

#### How to printf the various things
Not really useful, but..
```c
// es. tcp.c
printf("%.4d.  // delta_sec (unsigned int)
   %.6d        // delta_usec
   %.5d->%.5d  // ports (unsigned short)
   %.2x        // tcp flags (unsigned char) in hex: es: "12"
   %.10u       // seq (unsigned int)
   %.10u       // ack 
   %.5u        //tcp win   
   %4.2f\n", delta_sec, delta_usec, htons(tcp->s_port), htons(tcp->d_port), tcp->flags, htonl(tcp->seq) - seqzero, htonl(tcp->ack) - ackzero, htons(tcp->win), (htonl(tcp->ack) - ackzero) / (double)(delta_sec * 1000000 + delta_usec));

```

