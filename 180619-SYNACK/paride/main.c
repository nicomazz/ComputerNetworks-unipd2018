#include <arpa/inet.h>
#include<net/if.h>
#include <sys/types.h>          /* See NOTES */
#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <string.h>
#include <strings.h>
#include <assert.h>

#include <stdlib.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include "defines.h"

#include "utils.h"

#include "time.h"

unsigned char broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
unsigned char mymac[6]     = {0xf2, 0x3c, 0x91, 0xdb, 0xc2, 0x98};
unsigned char myip[4]      = {88, 80, 187, 84};
unsigned char gateway[4]   = {88, 80, 187, 1};
unsigned int netmask       = 0x00FFFFFF;


struct tcp_status {
    unsigned int forzero, backzero;
    struct timeval timezero;
} st[65536];



struct tcp_segment {
    unsigned short s_port;
    unsigned short d_port;
    unsigned int seq;
    unsigned int ack;

    unsigned char d_offs_res;
    unsigned char flags;
    unsigned short win;
    unsigned short checksum;
    unsigned short urgp;

    unsigned char payload[1000];
};

typedef struct eth_frame {
    unsigned char dst[6];
    unsigned char src[6];
    unsigned short type;
    char payload[1500];
} eth_frame_t;

typedef struct icmp_packet {
    unsigned char type;
    unsigned char code;
    unsigned short checksum;
    unsigned short id;
    unsigned short seq;
    char payload[128];
} icmp_packet_t;

typedef struct ip_datagram {
    unsigned char ver_ihl;
    unsigned char tos;
    unsigned short totlen;
    unsigned short id;
    unsigned short flags_offs;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short checksum;
    unsigned int src;
    unsigned int dst;
    unsigned char payload[1500];
} ip_datagram_t;

typedef struct arp_packet {
    unsigned short htype;
    unsigned short ptype;
    unsigned char hlen;
    unsigned char plen;
    unsigned short opcode;
    unsigned char hsrc[6];
    unsigned char psrc[4];
    unsigned char hdst[6];
    unsigned char pdst[4];
} arp_packet_t;




/* Prototypes */
void crea_eth(struct eth_frame *e, unsigned char * src, unsigned char*dst,unsigned short type);
void crea_arp(struct arp_packet* a,unsigned char *mymac,unsigned char *myip,unsigned char *dstip);
void crea_ip( struct ip_datagram* ip, int payloadlen,unsigned char payloadtype,unsigned int ipdest );
void crea_icmp_echo(struct icmp_packet * icmp);
unsigned short checksum(unsigned char *buffer,int len);
void stampabytes(unsigned char * buffer, int quanti);
int trovamac(unsigned char *mac_incognito, unsigned char*dstip);

unsigned short
tcp_checksum ( struct ip_datagram *ip,
               unsigned char * buffer,
               int len )
{
    int i;
    unsigned short *p;
    unsigned int tot = 0;

    /*Assunzione: l pari */
    assert(len % 2 == 0);

    unsigned char pseudo_header[12];
    
    *((unsigned int *)   &pseudo_header[0])  = ip->src;
    *((unsigned int *)   &pseudo_header[4])  = ip->dst;
    *((unsigned char *)  &pseudo_header[8])  = 0x0;              // zero field
    *((unsigned char *)  &pseudo_header[9])  = ip->protocol;     // PTCL field
    *((unsigned short *) &pseudo_header[10]) = ip->totlen - 20;  // @NOTE: -20 to remove the sizeof IP_HDR

    printf("pseudoheader");
    stampabytes(pseudo_header,12);
    for ( i = 0; i < (sizeof(pseudo_header) / 2); i++ ) {
        tot = tot + htons(p[i]);
        if (tot & 0x10000) {
            tot = (tot&0xFFFF) + 1;
        }
    }
 

    p = (unsigned short *) buffer;
   

    
    for ( i = 0; i < len / 2; i++ ) {
        tot = tot + htons(p[i]);
        if (tot & 0x10000) {
            tot = (tot&0xFFFF) + 1;
        }
    }

    return (unsigned short)0xFFFF-tot; 
}



void
send_syn_ack_response ( int socket,
                        char buffer[1500])
{
    struct sockaddr_ll sll;
    bzero(&sll, sizeof(struct sockaddr_ll));
    sll.sll_ifindex = if_nametoindex("eth0");
    int len = sizeof(sll);

    struct eth_frame    *eth;
    struct ip_datagram  *ip;
    struct icmp_packet  *icmp;
    struct tcp_segment  *tcp;

    eth  = (struct eth_frame *) buffer;
    ip   = (struct ip_datagram *) eth->payload;
    icmp = (struct icmp_packet*) ip->payload;
    tcp  = (struct tcp_segment *) ip->payload;

    // Swap ethernet source and destination
    for (int i = 0; i < 6; i++ ) {
        unsigned char temp = eth->src[i];
        eth->src[i] = eth->dst[i];
        eth->dst[i] = temp;
    }

    // Swap ip source and destination
    unsigned int temp_addr = ip->src;
    ip->src = ip->dst;
    ip->dst = temp_addr;

    // restore ttl
    ip->ttl = 0xff;

    // Recompute ip checksum
    ip->checksum = 0;
    ip->checksum = htons(checksum((unsigned char *) ip, 20));

    // Modify the TCP Header
    tcp->flags |= 0b010010; // Sets the acknowledgemnt flag and the syn flag
    tcp->seq = random();    // Reply with random seq number
    tcp->ack = htonl(ntohl(tcp->seq) + 1); // Set the ACK to the Client Sequence Number + 1 for synchronization
  
    // Swap TCP source port and destination port
    unsigned short temp_port = tcp->s_port;
    tcp->s_port = tcp->d_port;
    tcp->d_port = temp_port;

    // Recompute the TCP checksum
    tcp->checksum = 0;
    int tcp_checksum_len = (ip->totlen - 20) > 1500 ? 1500 : (ip->totlen - 20);
    tcp->checksum = htons(tcp_checksum(ip, (unsigned char *) tcp, tcp_checksum_len));
 
    
    // Send the SYN_ACK Response
    printf("Sending SYN_ACK Response\n");
    
    stampabytes(buffer,74);
    ssize_t t = sendto(socket, buffer, 74, 0, (struct sockaddr*) &sll, len);
    if ( t == -1 ) {
        perror("Failed to responde with SYN_ACK");
        abort();
    }
}


int
main( int argc, char **argv )
{
    srandom(time(0));
    struct sockaddr_ll sll;
    unsigned char mac_incognito[6];
    unsigned char buffer[1500];
    
    unsigned char      *nexthop;
    
    struct eth_frame   *eth;
    struct ip_datagram *ip;
    struct icmp_packet *icmp;
    struct tcp_segment *tcp;
//unsigned char targetip[4] = {88,80,187,2};
    unsigned char targetip[4] = {147,162,2,100};
    int m,i,s,len,t;


    
    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (s == -1) {
        perror("Fallita apertura socket");
        abort();
    }
    
    eth  = (struct eth_frame *) buffer;
    ip   = (struct ip_datagram *) eth->payload;
    icmp = (struct icmp_packet*) ip->payload;
    tcp  = (struct tcp_segment *) ip->payload;
    
    bzero(&sll, sizeof(struct sockaddr_ll));
    sll.sll_ifindex = if_nametoindex("eth0");
    printf("ifindex = %d\n", sll.sll_ifindex);
    len = sizeof(sll);

    while (1) { 
        t = recvfrom(s, buffer, 1500, 0, (struct sockaddr *) &sll, &len);
        if ( t == -1 ) {
            perror("recfrom faillita");
            abort();
        }

        if (eth->type == htons(ETH_P_IP)) {
            
            if (ip->protocol == IPPROTO_TCP) {
                // valid tcp protocol
                // printf("Protocollo TCP ricevuto\n");
                
                //if (tcp->d_port == htons(19420)) {
                //    printf("tcp port = %d\n", tcp->d_port);
                //}
                if (tcp->d_port == htons(19420)) {
                    
                    //printf("Porta corretta\n");
                    //printf("flags = %d\n", tcp->flags);
                    if (tcp->flags & 0b000010) {   // Check for SYN Flag
                        printf("Got a SYN Request\n");
                        stampabytes(buffer,74);
                        // @NOTE: we should verify that the checksum is indeed correct.
                        send_syn_ack_response(s, buffer);
                    }
                }
            }
        }
    }
}


int trovamac ( unsigned char *mac_incognito,
               unsigned char *dstip )
{
    struct sockaddr_ll sll;
    int len,i,t,m,s;
    char ifname[50];
    unsigned char buffer[1500];
    struct arp_packet *arp;
    struct eth_frame  *eth;
    
    eth = (struct eth_frame *) buffer;
    arp = (struct arp_packet *) eth->payload;
    
    crea_eth(eth, mymac, broadcast, 0x0806);
    crea_arp(arp, mymac, myip, dstip);
    stampabytes(buffer, 14 + sizeof(struct arp_packet));

    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    bzero(&sll, sizeof(struct sockaddr_ll));
    sll.sll_family = AF_PACKET;
//for(i=1;i<4;i++) 
//	printf("%s\n",if_indextoname(i,ifname));

    printf("\n");
    sll.sll_ifindex = if_nametoindex("eth0");
    len = sizeof(sll);

    
    t = sendto(s, buffer, 14 + sizeof(struct arp_packet), 0, (struct sockaddr *) &sll, len);   
    if ( t == -1 ) {
        perror("sendto fallita");
        return 1;
    }
    
    
    for( m=0; m < 1000; m++ ) {
        t = recvfrom(s, buffer, 1500, 0, (struct sockaddr *) &sll, &len);
        if ( t == -1 ){
            perror("recvfrom fallita");
            return 1;
        }


        if ( eth->type == htons(0x0806) ){
            if(arp->opcode == htons(2))
                if (! memcmp(arp->psrc, dstip, 4)) {
                    memcpy(mac_incognito, arp->hsrc, 6);
                    return 0;
                }
        }
    }
    return 1;
}

void crea_eth ( struct eth_frame * e,
                unsigned char * src,
                unsigned char*dst,
                unsigned short type )
{
    int i;
    e->type = htons(type);
    for ( i = 0; i < 6; i++ ) { e->src[i]=src[i]; }
    for ( i = 0; i < 6; i++ ) { e->dst[i]=dst[i]; }
} 

void crea_arp ( struct arp_packet* a,
                unsigned char *mymac,
                unsigned char *myip,
                unsigned char *dstip)
{
    int i;

    a-> htype  = htons(1);
    a-> ptype  = htons(0x0800);
    a-> hlen   = 6;
    a-> plen   = 4;
    a-> opcode = htons(1);
    
    for ( i = 0; i < 6; i++ ) { a-> hsrc[i] = mymac[i]; }
    for ( i = 0; i < 4; i++ ) { a-> psrc[i] = myip[i]; }
    for ( i = 0; i < 6; i++ ) { a-> hdst[i] = 0; }
    for ( i = 0; i < 4; i++ ) { a-> pdst[i] = dstip[i]; }
}

void stampabytes ( unsigned char * buffer,
                   int quanti )
{
    int i;
    for (i = 0; i < quanti; i++) {
        if (! (i & 0x3) ) {
            printf("\n");
        }
        printf("%.2X(%.3d) ", buffer[i], buffer[i]);
    }
    printf("\n");
}


void crea_ip ( struct ip_datagram* ip,
               int payloadlen,
               unsigned char payloadtype,
               unsigned int ipdest )
{

    ip->ver_ihl    = 0x45;      // (4 << 4) | 5;
    ip->tos        = 0;
    ip->totlen     = htons(20 + payloadlen);
    ip->id         = htons(0xABCD);
    ip->flags_offs = htons(0);
    ip->ttl        = 128;
    ip->protocol   = payloadtype;
    ip->src        = *((unsigned int*) myip);
    ip->dst        = ipdest;
    ip->checksum   = htons(0);
    ip->checksum   = htons(checksum((unsigned char*) ip, 20));
};


unsigned short
checksum ( unsigned char * buffer,
           int len )
{
    int i;
    unsigned short *p;
    unsigned int tot = 0;

    /*Assunzione: l pari */
    assert(len % 2 == 0);

    p = (unsigned short *) buffer;
    
    for ( i = 0; i < len / 2; i++ ) {
        tot = tot + htons(p[i]);
        if (tot & 0x10000) {
            tot = (tot&0xFFFF) + 1;
        }
    }
    
    return (unsigned short)0xFFFF-tot; 
}

void
crea_icmp_echo(struct icmp_packet * icmp)
{
    int i;
    
    icmp->type       = 8;
    icmp->code       = 0;
    icmp->checksum   = htons(0);
    icmp->id         = htons(0x1234);
    icmp->seq        = htons(1);
    
    for( i = 0; i < 20; i++ ) { icmp->payload[i] = i; }

    icmp->checksum   = htons(checksum((unsigned char*)icmp, 28));
}

