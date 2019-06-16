#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>

/*
unsigned char miomac[6] = { 0xf2,0x3c,0x91,0xdb,0xc2,0x98 };
unsigned char broadcast[6] = { 0xff, 0xff, 0xff, 0xff,0xff,0xff};
unsigned char mioip[4] = {88,80,187,84};
unsigned char netmask[4] = { 255,255,255,0};
unsigned char gateway[4] = { 88,80,187,1};
*/
unsigned char miomac[6] = {0xb0, 0xc0, 0x90, 0xa6, 0x78, 0xfb};
unsigned char broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
unsigned char mioip[4] = {192, 168, 1, 12};
unsigned char netmask[4] = {255, 255, 255, 0};
unsigned char gateway[4] = {192, 168, 1, 1};

//unsigned char iptarget[4] = {88,80,187,80};
unsigned char iptarget[4] = {147, 162, 2, 100};

struct sockaddr_ll sll;
struct timeval stop, start;

struct eth_frame {
   unsigned char dst[6];
   unsigned char src[6];
   unsigned short int type;  // ARP -> 0x0806, IP -> 0x0800
   unsigned char payload[1];
};

struct arp_packet {
   unsigned short int htype;  // 1 -> eth
   unsigned short int ptype;  // 0x0800 -> ip
   unsigned char hlen;
   unsigned char plen;
   unsigned short int op;  // operation: request -> 1, response -> 2
   unsigned char hsrc[6];
   unsigned char psrc[4];
   unsigned char hdst[6];
   unsigned char pdst[4];
};

struct icmp_packet {
   unsigned char type;
   unsigned char code;
   unsigned short checksum;  // checksum of ICMP header + ICMP payload
   unsigned short id;
   unsigned short seq;
   unsigned char payload[1];
};

struct ip_datagram {
   unsigned char ver_ihl;  // usually 0x45
   unsigned char tos;      // 0
   unsigned short totlen;  // 20 (ip header) + payload size
   unsigned short id;      // id chosen by sender
   unsigned short flag_offs;
   unsigned char ttl;        // time to live
   unsigned char proto;      // icmp -> 1, 6 -> TCP
   unsigned short checksum;  // checksum of header only
   unsigned int saddr;
   unsigned int daddr;
   unsigned char payload[1];
};

void stampa_buffer(unsigned char *b, int quanti);

void crea_eth(struct eth_frame *e, unsigned char *dest, unsigned short type) {
   int i;
   for (i = 0; i < 6; i++) {
      e->dst[i] = dest[i];
      e->src[i] = miomac[i];
   }
   e->type = htons(type);
}

void crea_arp(struct arp_packet *a, unsigned short op, unsigned char *ptarget) {
   int i;
   a->htype = htons(1);
   a->ptype = htons(0x0800);
   a->hlen = 6;
   a->plen = 4;
   a->op = htons(op);
   for (i = 0; i < 6; i++) {
      a->hsrc[i] = miomac[i];
      a->hdst[i] = 0;
   }
   for (i = 0; i < 4; i++) {
      a->psrc[i] = mioip[i];
      a->pdst[i] = ptarget[i];
   }
}

int risolvi(unsigned char *target, unsigned char *mac_incognito) {
   unsigned char buffer[1500];
   struct eth_frame *eth;
   struct arp_packet *arp;
   int i, n, s;
   int lungh;

   // init raw socket
   s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
   if (s == -1) {
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
   if (n == -1) {
      perror("sendto fallita");
      return 1;
   }

   while (1) {
      n = recvfrom(s, buffer, 1500, 0, (struct sockaddr *)&sll, &lungh);
      if (n == -1) {
         perror("recvfrom fallita");
         return 1;
      }
      if (eth->type == htons(0x0806))
         if (arp->op == htons(2))
            if (!memcmp(arp->psrc, target, 4)) {
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

unsigned short checksum(unsigned char *b, int n) {
   int i;
   unsigned short prev, tot = 0, *p = (unsigned short *)b;
   for (i = 0; i < n / 2; i++) {
      prev = tot;
      tot += htons(p[i]);
      if (tot < prev) tot++;
   }
   return (0xFFFF - tot);
}

void crea_icmp_echo(struct icmp_packet *icmp, unsigned short id, unsigned short seq) {
   int i;
   icmp->type = 8;
   icmp->code = 0;
   icmp->checksum = htons(0);
   icmp->id = htons(id);
   icmp->seq = htons(seq);
   for (i = 0; i < 20; i++)
      icmp->payload[i] = i;
   icmp->checksum = htons(checksum((unsigned char *)icmp, 28));
}

void crea_ip(struct ip_datagram *ip, int payloadsize, unsigned char proto, unsigned char *ipdest, unsigned char ttl) {
   ip->ver_ihl = 0x45;
   ip->tos = 0;
   ip->totlen = htons(payloadsize + 20);
   ip->id = htons(0xABCD);
   ip->flag_offs = htons(0);
   ip->ttl = ttl;
   ip->proto = proto;
   ip->checksum = htons(0);
   ip->saddr = *(unsigned int *)mioip;
   ip->daddr = *(unsigned int *)ipdest;
   ip->checksum = htons(checksum((unsigned char *)ip, 20));
};

int main() {
   int t, s;
   unsigned char mac[6];
   unsigned char buffer[1500];
   struct icmp_packet *icmp;
   struct ip_datagram *ip;
   struct eth_frame *eth;
   int lungh, n;

   // get target mac
   if ((*(unsigned int *)mioip & *(unsigned int *)netmask) == (*(unsigned int *)iptarget & *(unsigned int *)netmask))
      t = risolvi(iptarget, mac);
   else
      t = risolvi(gateway, mac);
   if (t == 0) {
      printf("Mac incognito:\n");
      stampa_buffer(mac, 6);
   }

   // init raw socket
   s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
   if (s == -1) {
      perror("Socket fallita");
      return 1;
   }

   eth = (struct eth_frame *)buffer;
   ip = (struct ip_datagram *)eth->payload;
   icmp = (struct icmp_packet *)ip->payload;

   // printf("ICMP/IP/ETH:\n");
   // stampa_buffer(buffer,14+20+8+20);

   lungh = sizeof(struct sockaddr_ll);
   bzero(&sll, lungh);
   sll.sll_ifindex = 3;

   // send ICMP echos with increasing ttl and wait for response:
   // if the response is an echo response then we reached the target,
   // otherwise we get a time exceed message
   int ttl = 1;
   int flag_host_reached = 0;

   while (!flag_host_reached) {
      crea_icmp_echo(icmp, 0x1234, 1);
      // 28: ICMP head + ICMP payload
      crea_ip(ip, 28, 1, iptarget, ttl);
      crea_eth(eth, mac, 0x0800);

      // re-write lungh as recvfrom modify it
      lungh = sizeof(struct sockaddr_ll);
      n = sendto(s, buffer, 14 + 20 + 8 + 20, 0, (struct sockaddr *)&sll, lungh);
      if (n == -1) {
         perror("sendto fallita");
         return 1;
      }
      printf("Sent echo message with ttl = %d\n", ttl);

      gettimeofday(&start, NULL);
      while (1) {
         gettimeofday(&stop, NULL);
         if ((stop.tv_usec - start.tv_usec) > 500000) {
            printf("\nTime exceeded. Retrying\n");
            ttl--;
            break;
         }
         n = recvfrom(s, buffer, 1500, 0, (struct sockaddr *)&sll, &lungh);
         if (n == -1) {
            perror("recvfrom fallita");
            return 1;
         }

         // check if it's an echo response
         if (eth->type == htons(0x0800)  // ip?
             && ip->proto == 1           // icmp?
             && icmp->type == 0          // echo resp
             && icmp->code == 0) {
            printf("Echo response received, host reached!\n");
            flag_host_reached = 1;
            break;
         }

         if (eth->type = htons(0x0800) && ip->proto == 1 && icmp->type == 11  // time exceeded message?
                                                                              //&& icmp->code == 0
         ) {
            printf("Found intermediary host n. %d\twith ip: ", ttl);
            for (int i = 0; i < 4; i++)
               printf("%d.", ((unsigned char *)&ip->saddr)[i]);
            printf("\n");

            break;
         }
      }  // end while 1
      ttl++;
   }  // end while not reached
}
