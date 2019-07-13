#include <stdio.h>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

unsigned char miomac[6] = {0xf2, 0x3c, 0x91, 0xdb, 0xc2, 0x98};
unsigned char broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
unsigned char mioip[4] = {88, 80, 187, 84};
unsigned char netmask[4] = {255, 255, 255, 0};
unsigned char gateway[4] = {88, 80, 187, 1};

//unsigned char iptarget[4] = {88,80,187,80};
unsigned char iptarget[4] = {147,162,2,100};

struct sockaddr_ll sll;

struct eth_frame
{
   unsigned char dst[6];
   unsigned char src[6];
   unsigned short int type; //0x0800=IP, 0x0806=ARP
   unsigned char payload[1];
};

struct arp_packet
{
   unsigned short int htype; //tipo protocollo di rete. Eth=1
   unsigned short int ptype; //IPv4=0x0800
   unsigned char hlen; //lunghezza indirizzi hardware. Ethernet=6
   unsigned char plen; //lunghezza protocollo superiore. IPv4=4
   unsigned short int op; //operazione del mittente: 1=richiesta, 2=risposta
   unsigned char hsrc[6];
   unsigned char psrc[4];
   unsigned char hdst[6];
   unsigned char pdst[4];
};

struct ip_datagram
{
   unsigned char ver_ihl; // first 4 bits: version, second 4 bits: (lenght header)/32
   unsigned char tos; //type of service
   unsigned short totlen; // len header + payload
   unsigned short id; // useful in case of fragmentation
   unsigned short flag_offs; //offset/8 related to the original ip package
   unsigned char ttl;
   unsigned char proto; // TCP = 6, ICMP = 1
   unsigned short checksum; // only header checksum (not of payload). Must be at 0 before the calculation.
   unsigned int saddr; // ip address
   unsigned int daddr; // ip address
   unsigned char payload[1];
};

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

struct tcp_pseudo{
   unsigned int ip_src, ip_dst;
   unsigned char zeroes;
   unsigned char proto;        // ip datagram protocol field (tcp = 6, ip = 1)
   unsigned short entire_len;  // tcp length (header + data)
   unsigned char tcp_segment[20/*to set appropriatly */];  // entire tcp packet pointer
};

void stampa_buffer(unsigned char *b, int quanti);

void crea_eth(struct eth_frame *e, unsigned char *dest, unsigned short type)
{
   int i;
   for (i = 0; i < 6; i++)
   {
      e->dst[i] = dest[i];
      e->src[i] = miomac[i];
   }
   e->type = htons(type);
}

void crea_arp(struct arp_packet *a, unsigned short op, unsigned char *ptarget)
{
   int i;
   a->htype = htons(1);
   a->ptype = htons(0x0800);
   a->hlen = 6;
   a->plen = 4;
   a->op = htons(op);
   for (i = 0; i < 6; i++)
   {
      a->hsrc[i] = miomac[i];
      a->hdst[i] = 0;
   }
   for (i = 0; i < 4; i++)
   {
      a->psrc[i] = mioip[i];
      a->pdst[i] = ptarget[i];
   }
}

int risolvi(unsigned char *target, unsigned char *mac_incognito)
{
   unsigned char buffer[1500];
   struct eth_frame *eth;
   struct arp_packet *arp;
   int i, n, s;
   int lungh;
   s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
   if (s == -1)
   {
      perror("socket fallita");
      return 1;
   }

   eth = (struct eth_frame *)buffer;
   arp = (struct arp_packet *)eth->payload;

   crea_arp(arp, 1, target);
   crea_eth(eth, broadcast, 0x0806);
   //stampa_buffer(buffer, 14+sizeof(struct arp_packet));

   sll.sll_family = AF_PACKET;
   sll.sll_ifindex = 3;
   lungh = sizeof(sll);
   n = sendto(s, buffer, 14 + sizeof(struct arp_packet), 0, (struct sockaddr *)&sll, lungh);
   if (n == -1)
   {
      perror("sendto fallita");
      return 1;
   }

   while (1)
   {
      n = recvfrom(s, buffer, 1500, 0, (struct sockaddr *)&sll, &lungh);
      if (n == -1)
      {
         perror("recvfrom fallita");
         return 1;
      }
      if (eth->type == htons(0x0806))
         if (arp->op == htons(2))
            if (!memcmp(arp->psrc, target, 4))
            {
               for (i = 0; i < 6; i++)
                  mac_incognito[i] = arp->hsrc[i];
               break;
            }
   }
   close(s);
   return 0;
}

void stampa_buffer(unsigned char *b, int quanti) {
   int i;
   for (i = 0; i < quanti; i++) {
      printf("%.3d(%.2x) ", b[i], b[i]);
      if ((i % 4) == 3)
         printf("\n");
   }
}

unsigned short checksum( unsigned char * buffer, int len) {
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

void crea_ip(struct ip_datagram *ip, int payloadsize, unsigned char proto, unsigned char *ipdest)
{
   ip->ver_ihl = 0x45;
   ip->tos = 0;
   ip->totlen = htons(payloadsize + 20);
   ip->id = htons(0xABCD);
   ip->flag_offs = htons(0);
   ip->ttl = 128;
   ip->proto = proto;
   ip->checksum = htons(0);
   ip->saddr = *(unsigned int *)mioip;
   ip->daddr = *(unsigned int *)ipdest;
   ip->checksum = htons(checksum((unsigned char *)ip, 20));
};

void crea_tcp(struct tcp_segment *tcp, unsigned short s_port, unsigned int seq) {
   tcp->s_port = htons(s_port);
   tcp->d_port = htons(80);
   tcp->seq = htonl(seq);        // offset in bytes from the start of the tcp segment in the stream (from initial sequance n)
   tcp->d_offs_res = 80;// first 4 bits: (header len/8)
   tcp->flags = 2;            // check rfc
   tcp->win = htons(0xFFFF);      // usually initially a 0 (?)
   tcp->checksum = 0;
   tcp->urgp = 0;  
} 

int main()
{
   int t, s;
   unsigned char mac[6];
   unsigned char buffer[1500];
   struct tcp_segment *tcp;
   struct ip_datagram *ip;
   struct eth_frame *eth;
   int lungh, n;

   srandom(time(NULL));

   if ((*(unsigned int *)mioip & *(unsigned int *)netmask) == (*(unsigned int *)iptarget & *(unsigned int *)netmask))
   {
      t = risolvi(iptarget, mac);
   }
   else
      t = risolvi(gateway, mac);

   if (t == 0)
   {
      printf("Mac incognito:\n");
      stampa_buffer(mac, 6);
      printf("\n");
   }
   s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
   if (s == -1)
   {
      perror("Socket fallita");
      return 1;
   }
   eth = (struct eth_frame *)buffer;
   ip = (struct ip_datagram *)eth->payload;
   tcp = (struct tcp_segment*)ip->payload;

   unsigned short port = random();
   unsigned short seq = random();

   crea_tcp(tcp, port, seq); 
   crea_ip(ip, 20, 6, iptarget);
   crea_eth(eth, mac, 0x0800);

   int TCP_TOTAL_LEN = 20;
   struct tcp_pseudo pseudo; // size of this: 12
   memcpy(pseudo.tcp_segment,tcp,TCP_TOTAL_LEN);
   pseudo.zeroes = 0;
   pseudo.ip_src = ip->saddr;
   pseudo.ip_dst = ip->daddr;
   pseudo.proto = 6;
   pseudo.entire_len = htons(TCP_TOTAL_LEN); // may vary
   tcp->checksum = htons(checksum((unsigned char*)&pseudo,TCP_TOTAL_LEN+12));

   stampa_buffer(buffer, 14 + 20 + 20);

   lungh = sizeof(struct sockaddr_ll);
   bzero(&sll, lungh);
   sll.sll_ifindex = 3;

   n = sendto(s, buffer, 14 + 20 + 20, 0, (struct sockaddr *)&sll, lungh);
   if (n == -1)
   {
      perror("sendto fallita");
      return 1;
   }
   printf("Attendo risposta...\n");
   while (1)
   {
      n = recvfrom(s, buffer, 1500, 0, (struct sockaddr *)&sll, &lungh);
      if (n == -1)
      {
         perror("recvfrom fallita");
         return 1;
      }
      if (eth->type == htons(0x0800))
         if (ip->proto == 6)
            if (tcp->s_port == htons(80))
            {
               if(tcp->d_port == htons(port))
               {
                  if(tcp->ack == htonl(seq+1))
                     if(tcp->flags == 0x12) {
                        stampa_buffer(buffer, n);
                        break;
                     }
               }
            }
   }
}
