#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <string.h>
#include <unistd.h>

unsigned char my_mac[6] = { 0xb0,0xc0,0x90,0xa6,0x78,0xfb };
unsigned char broadcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
unsigned char my_ip[4] = { 192, 168, 1, 10 };
unsigned char netmask[4] = { 255,255,255,0};
unsigned char gateway[4] = { 192,168,1,1};
/*
unsigned char my_mac[6] = { 0xf2,0x3c,0x91,0xdb,0xc2,0x98 };
unsigned char broadcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
unsigned char my_ip[4] = { 88,80,187,84 };
unsigned char target_ip[4] = { 88,80,187,80 };
unsigned char netmask[4] = { 255,255,255,0};
unsigned char gateway[4] = { 88,80,187,1};
*/

struct sockaddr_ll sll;

struct eth_frame {       // TOT 14 bytes
  unsigned char dst[6];
  unsigned char src[6];
  unsigned short type;   // 0x0800 -> IP, 0x0806 -> ARP
  unsigned char payload[1];
};

struct arp_packet {
  unsigned short int htype;
  unsigned short int ptype;
  unsigned char hlen;
  unsigned char plen;
  unsigned short int op;
  unsigned char hsrc[6];
  unsigned char psrc[4];
  unsigned char hdst[6];
  unsigned char pdst[4];
};

struct ip_datagram {        // TOT 20 bytes
  unsigned char ver_ihl;    // default 0x45
  unsigned char tos;        // type of service -> 0
  unsigned short totlen;
  unsigned short id;        // id chosen by sender
  unsigned short flag_offs;
  unsigned char ttl;        // packet time to live
  unsigned char proto;      // ICMP -> 1
  unsigned short checksum;  // checksum of header only
  unsigned int saddr;
  unsigned int daddr;
  unsigned char payload[1];
};


struct icmp_unreachable {    // TOT 8 bytes
  unsigned char type;
  unsigned char code;
  unsigned short checksum;
  unsigned int unused;
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
   unsigned char payload[1];
};


unsigned char buffer[5000];
unsigned char response_buffer[5000];
unsigned char temp[1000];

void build_icmp_unreachable(
    struct icmp_unreachable
    *icmp, unsigned char code,
    unsigned char *old_ip_message,
    int old_ip_size
);
unsigned short checksum(unsigned char *b, int n);
void build_eth(struct eth_frame *e, unsigned char *dest, unsigned short type);
void build_ip_datagram(struct ip_datagram *ip, int payloadsize,
                       unsigned char protocol, unsigned char *target_ip);
int arp_req(unsigned char *mac, unsigned char *target);
void build_arp(struct arp_packet *a, unsigned char *dest_ip, unsigned short op);

//Stampa un numero di byte da un buffer
void stampabytes(unsigned char * buffer, int quanti){
	int i;
	for(i=0;i<quanti;i++){
		if (!(i&0x3)) printf("\n");
		printf("%.2X(%.3d) ",buffer[i],buffer[i]);
	}
	printf("\n");
}

/*
1. listen for raw packets (tcp)
2. filtrare i pacchetti in base alla porta tcp scelta
3. generare un messaggio ICMP destination unreachable ed inviarlo al sender
del pacchetto tcp
*/
int main() {
  int i, j, s, len, n;
  unsigned char MY_PORT = 80;

  s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (s==-1) {
    printf("Error creating socket. Do you have enough permissions?\n");
    return 1;
  }

  sll.sll_family = AF_PACKET;
  sll.sll_ifindex = 3;
  len = sizeof(sll);

  struct eth_frame *eth = (struct eth_frame*) buffer;
  struct ip_datagram *ip = (struct ip_datagram*) eth->payload;
  struct tcp_segment *tcp = (struct tcp_segment*) ip->payload;

  while (1) {

    bzero(buffer, 5000);
    n = recvfrom(s, buffer, 5000, 0, (struct sockaddr*)&sll, &len);
    if (n==-1) {
      printf("Error receiving raw packets.\n");
      return 1;
    }

    if (eth->type == htons(0x0800) // ip?
        && ip->proto == 6 // tcp?
        && tcp->d_port == htons(MY_PORT)
    ) {
      printf("Got one!\n");

      printf("\nReceived: \n");
      stampabytes(buffer, n);

      unsigned int source_ip = ip->saddr;
      unsigned char target_mac[6];
      // if target ip is not in LAN then request gateway MAC
      if ((unsigned int)source_ip & (unsigned int)netmask
          != (unsigned int)my_ip & (unsigned int)netmask) {
            n = arp_req(target_mac, gateway);
      } else {
          n = arp_req(target_mac, (unsigned char*)&source_ip);
      }

      if(n != 0) {
        printf("Error finding target Mac\n");
        return 1;
      }

      int ip_header_len = (ip->ver_ihl & 0x0F) *8;
      // temporary store IP header + first 8 bytes, requested
      // by ICMP error message payload
      bzero(temp, 1000);
      memcpy(temp, ip, ip_header_len + 8);

      bzero(response_buffer, 5000);

      struct eth_frame *eth_r = (struct eth_frame*)response_buffer;
      struct ip_datagram *ip_r = (struct ip_datagram*)eth_r->payload;
      struct icmp_unreachable *icmp_u  = (struct icmp_unreachable*)ip_r->payload;


      // build an ICMP unreachable message
      build_eth(eth_r, target_mac, 0x0800);
      // payloadsize: ip_header_len + first 8 bytes + icmp header size (8)
      build_ip_datagram(ip_r, ip_header_len + 8 + 8, 1, (unsigned char*)&source_ip);
      // 3: PORT unreachable code
      build_icmp_unreachable(icmp_u, 3, temp, ip_header_len + 8);

      // message size: 14 (eth) + 20 (ip) + 8 (icmp) + ip_header_len + 8 (first 8 bytes of payload)
      int tot_size = 14 + 20 + 8 + ip_header_len + 8;

      printf("\Sending: \n");
      stampabytes(response_buffer, tot_size);

      n = sendto(s, buffer, tot_size, 0, (struct sockaddr*)&sll, sizeof(sll));
      if(n==-1) {
        printf("Error sending ICMP message.\n");
        return 1;
      }
    }
  }
}


void build_icmp_unreachable(
    struct icmp_unreachable
    *icmp, unsigned char code,
    unsigned char *old_ip_message,
    int old_ip_size
  ) {
  icmp->type = 3;
  icmp->code = code;
  icmp->unused = 0;
  icmp->checksum = 0;
  for (int i = 0; i < old_ip_size; i++)
    icmp->payload[i] = old_ip_message[i];
  icmp->checksum = htons(checksum((unsigned char*)icmp, 8 + old_ip_size));
}

/*
* Build an IP datagram with @param target_ip as IP target and @param protocol
* as protocol.
* Some protocols:
*  1 -> ICMP
*/
void build_ip_datagram(struct ip_datagram *ip, int payloadsize,
                       unsigned char protocol, unsigned char *target_ip) {
  ip->ver_ihl = 0x45;
  ip->tos = 0;
  ip->totlen = htons(20 + payloadsize);
  ip->id = htons(0xABCD);
  ip->flag_offs = htons(0);
  ip->ttl = 64;
  ip->proto = protocol;
  ip->checksum = htons(0);
  ip->saddr = *(unsigned int*) my_ip;
  ip->daddr = *(unsigned int*) target_ip;
  ip->checksum = htons(checksum((unsigned char*)ip, 20));
}


/*
* Build an ethernet packet with @param dest as MAC target and @param type
* as type.
* Some types:
*  0x0806 -> ARP protocol
*  0x0800 -> IP protocol
*/
void build_eth(struct eth_frame *e, unsigned char *dest, unsigned short type) {
  int i;
  e->type = htons(type);
  for(i=0; i<6; i++) {
    e->dst[i] = dest[i];
    e->src[i] = my_mac[i];
  }
}

/*
* Send an ARP request to @param target as target IP and set the response in
* @param mac.
* @return 1 on error, 0 otherwise
*/
int arp_req(unsigned char *mac, unsigned char *target) {
  int i,s, n, len;
  unsigned char buffer[1500];
  struct eth_frame *e = (struct eth_frame *) buffer;
  struct arp_packet *a = (struct arp_packet *) e->payload;

  s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if(s==-1) {
    printf("Arp req: Error creating socker.\n");
    return 1;
  }

  build_arp(a, target, 1); // 1-> arp req
  build_eth(e, broadcast, 0x0806);

  sll.sll_family = AF_PACKET;
  sll.sll_ifindex = 3;
  len = sizeof(sll);

  n = sendto(s,
             buffer,
             14+sizeof(struct arp_packet),
             0,
             (struct sockaddr *)&sll,
             len);

  if(n==-1) {
    printf("Arp req: Error sending arp request.\n");
    return 1;
  }

  printf("Waiting for arp response...\n");

  while (1) {
    n = recvfrom(s, buffer, 1500, 0, (struct sockaddr *)&sll, &len);

    if(n==-1) {
      printf("Arp req: Error receiving packet.\n");
      return 1;
    }

    if(e->type == htons(0x0806)
       && a->op == htons(2)
       && !memcmp(a->psrc, target, 4)) {

      for(i=0; i<6; i++)
        mac[i] = a->hsrc[i];

      break;
    }
  }

  printf("Arp mac response:\n");
  for(i=0; i<6; i++)
    printf("%x", mac[i]);
  printf("\n");

  close(s);
  return 0;
}

/*
* Build an ARP request with @param dest_ip as target IP and @param op
* as operation.
* Some operations:
*  1 -> ARP request
*  2 -> ARP response
*/
void build_arp(struct arp_packet *a, unsigned char *dest_ip, unsigned short op) {
  int i;
  a->htype = htons(1);
  a->ptype = htons(0x0800);
  a->hlen = 6;
  a->plen = 4;
  a->op = htons(op);
  for(i=0; i<6; i++) {
    a->hsrc[i] = my_mac[i];
    a->hdst[i] = 0;
  }
  for(i=0; i<4; i++) {
    a->psrc[i] = my_ip[i];
    a->pdst[i] = dest_ip[i];
  }
}

/*
* Calculate the checksum of the @param buffer according with the directives of
* Jon Postel.
* Right now it works only if @param size is even
*/
unsigned short checksum(unsigned char *b, int n) {
  unsigned short i, prev, tot = 0;
  unsigned short *p = (unsigned short*)b;
  for (i=0; i<n/2; i++) {
    prev = tot;
    tot += htons(p[i]);
    if (tot < prev)
      tot++;
  }
  return (0xFFFF - tot);
}
