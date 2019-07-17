#include <stdio.h>
#include <strings.h>
#include <sys/types.h> /* See NOTES */
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <arpa/inet.h>

struct timeval tcptime, timezero;
struct sockaddr_ll sll;
unsigned int delta_sec, delta_usec;
int primo;
unsigned char dstip[4] = {88, 80, 187, 80};
unsigned char miomac[6] = {0xf2, 0x3c, 0x91, 0xdb, 0xc2, 0x98};
unsigned char mioip[4] = {88, 80, 187, 84};
unsigned char gateway[4] = {88, 80, 187, 1};
unsigned char netmask[4] = {255, 255, 255, 0};
unsigned char broadcastmac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int progr = 0;

struct tcp_segment
{
   unsigned short s_port;
   unsigned short d_port;
   unsigned int seq;
   unsigned int ack;
   unsigned char d_offs_res; // header_length/32
   unsigned char flags; //[4bit reserved] CWR ECE URG ACK PSH RST SYN FIN
   unsigned short win; // window size
   unsigned short checksum; 
   unsigned short urgp;
   unsigned char payload[1000];
};

struct eth_frame
{
   unsigned char dst[6];
   unsigned char src[6];
   unsigned short type;
   char payload[1500];
};

struct ip_datagram
{
   unsigned char ver_ihl;
   unsigned char tos;
   unsigned short totlen;
   unsigned short id;
   unsigned short flag_offs;
   unsigned char ttl;
   unsigned char proto;
   unsigned short checksum;
   unsigned int saddr;
   unsigned int daddr;
   unsigned char payload[1];
};

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

void creaip(struct ip_datagram *ip, unsigned int dstip, int payload_len, unsigned char proto) {
   ip->ver_ihl = 0x45;
   ip->tos = 0x0;
   ip->totlen = htons(payload_len + 20);
   ip->id = progr++;
   ip->flag_offs = htons(0);
   ip->ttl = 64;
   ip->proto = proto;
   ip->checksum = 0;
   ip->saddr = *(unsigned int *)mioip;
   ip->daddr = dstip;
   ip->checksum = htons(checksum((unsigned char *)ip, 20));
};

int main(int argc, char **argv) {
   unsigned int ackzero, seqzero, forwzero, backzero;
   unsigned char buffer[1600];
   unsigned short other_port;
   int i, l, s, lunghezza;
   unsigned int window;
   struct eth_frame *eth;
   struct ip_datagram *ip;
   struct tcp_segment *tcp;
   eth = (struct eth_frame *)buffer;
   ip = (struct ip_datagram *)eth->payload;
   tcp = (struct tcp_segment *)ip->payload;
   s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
   lunghezza = sizeof(struct sockaddr_ll);
   bzero(&sll, sizeof(struct sockaddr_ll));
   sll.sll_ifindex = 3;
   primo = 1;
   while (1)
   {
      l = recvfrom(s, buffer, 1600, 0, (struct sockaddr *)&sll, &lunghezza);
      gettimeofday(&tcptime, NULL);
      if (eth->type == htons(0x0800))
         if (ip->proto == 6)
         {
            window= tcp->win;

            //check tcp options
            unsigned char *p = (unsigned char *)tcp + 20;
            unsigned char *end = (unsigned char *)tcp + ((tcp->d_offs_res) >> 2);
            while (p < end) {
               unsigned char kind = *p++;
               if (kind == 0) {
                  break;
               }
               if (kind == 1) {
                  // No-op option with no length.
                  continue;
               }

               // Assumo che tutte le opzioni abbiano il campo length (anche se la rfc dice che puÃ²non essere cosÃ)
               unsigned char size = *p++;
               if (kind == 3 && ((tcp->flags)&0x2)== 0x2) {
                  printf("windows scale option. SEG.WND=%d, scale=%d, ", window, *p);
                  window = window << *p;
                  printf("new window= %d. \n", window);
               }
               p += (size - 2);
            }

            //printf("window = %d \n", window);
         }
   }
}
