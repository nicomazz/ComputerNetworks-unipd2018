
#include <arpa/inet.h>
#include<net/if.h>
#include <sys/types.h>          /* See NOTES */
#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <string.h>
#include <strings.h>

unsigned char broadcast[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned char mymac[6]={0xf2,0x3c,0x91,0xdb,0xc2,0x98};
unsigned char myip[4]={88,80,187,84};
unsigned char gateway[4]={88,80,187,1};
unsigned int netmask = 0x00FFFFFF;

unsigned char mac_cache[6];

struct pseudoheader{
unsigned int s_addr;
unsigned int d_addr;
unsigned char zero;
unsigned char proto;
unsigned short tcp_len;
};


struct tcp_segment{
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

struct eth_frame {
unsigned char dst[6];
unsigned char src[6];
unsigned short type;
char payload[15000];
};
struct icmp_packet{
unsigned char type;
unsigned char code;
unsigned short checksum;
unsigned short id;
unsigned short seq;
char payload[128];
};
struct ip_datagram{
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
unsigned char payload[15000];
};

struct arp_packet{
unsigned short htype;
unsigned short ptype;
unsigned char hlen;
unsigned char plen;
unsigned short opcode;
unsigned char hsrc[6];
unsigned char psrc[4];
unsigned char hdst[6];
unsigned char pdst[4];
};
void crea_tcp(struct tcp_segment * tcp, unsigned short srcport, unsigned short dstport, struct pseudoheader* pshdr);
void crea_eth(struct eth_frame *e, unsigned char * src, unsigned char*dst,unsigned short type);
void crea_arp(struct arp_packet* a,unsigned char *mymac,unsigned char *myip,unsigned char *dstip);
void crea_ip( struct pseudoheader * pshdr, struct ip_datagram* ip, int payloadlen,unsigned char payloadtype,unsigned int ipdest );
void crea_icmp_echo(struct icmp_packet * icmp);
unsigned short checksum(unsigned char *pshdr, unsigned char *buffer,int len);
void stampabytes(unsigned char * buffer, int quanti);
int trovamac(unsigned char *mac_incognito, unsigned char*dstip);

int main(){
struct sockaddr_ll sll;
unsigned char mac_incognito[6];
unsigned char buffer[15000];
unsigned char * nexthop;
struct eth_frame * eth;
struct ip_datagram * ip;
struct tcp_segment * tcp;
struct icmp_packet * icmp;
struct pseudoheader pshdr;
//unsigned char targetip[4]={88,80,187,2};
//unsigned char targetip[4]={147,162,2,100};
//unsigned char targetip[4]={216,58,213,68};
//unsigned char targetip[4]={185,2,4,58};
unsigned short int savedport=0;
unsigned int savedip;

//unsigned char targetip[4]={216,58,204,68};
unsigned char targetip[4]={147,162,2,199};
int m,i,s,len,t;
s = socket(AF_PACKET, SOCK_RAW,htons(ETH_P_ALL));
eth = (struct eth_frame *) buffer;
ip = (struct ip_datagram *) eth->payload;
tcp = (struct tcp_segment*) ip->payload;
icmp = (struct icmp_packet*) ip->payload;
for(m=0;m<100000;m++){
t = recvfrom(s, buffer, 15000, 0, (struct sockaddr *) &sll, &len);
if (t == -1 ){
 perror("recvfrom fallita");
return 1;
}
if (eth->type == htons(0x0800)){
        if((ip->protocol == 1 )){
                if((icmp->type == 0)){
                	//stampabytes(buffer,t);
                	//return 0;
			}
	}
	else if(ip->protocol == 6) 
                if((tcp->d_port == htons(29600)) || (tcp->d_port == htons(savedport))){
                	if(!savedport && (tcp->flags &  0x02 )&& (tcp->d_port == htons(29600))){
				printf("Inizio connessione da client\n");
				savedport =htons(tcp->s_port);
				savedip = htonl(ip->src);  
			}	
			//tcp->flags=0x12;
                	if(tcp->d_port == htons(29600)){
				printf("In ingresso da client:\n");
                		stampabytes((unsigned char *)ip,t-14);
				if (htons(tcp->s_port!=htons(savedport))) continue;
				pshdr.d_addr= ip->dst = *(int *)targetip; 
				tcp->d_port = htons(80);
				tcp->s_port=htons(savedport);
				
			} else {
				printf("In ingresso da google\n");
                		stampabytes((unsigned char *)ip,t-14);
				pshdr.d_addr= ip->dst = htonl(savedip);
				//tcp->d_port = htons(savedport);
				tcp->s_port=htons(29600);
			}	
			pshdr.tcp_len=htons(htons(ip->totlen)-20);
			if(htons(ip->totlen) > 15000){ printf("troppo lungo!\n"); return 1;}
			pshdr.zero=0;
			pshdr.proto=6;
			ip->ttl =64;
			pshdr.s_addr= ip->src  = *((unsigned int*)myip);	
			ip->checksum=0;
			ip->checksum = htons(checksum(NULL,(unsigned char*)ip,20));
			
			//tcp->ack= htonl(htonl(tcp->seq)+1);
			//tcp->seq= htonl(0x9999999);
			tcp->checksum=htons(0);
			tcp->checksum = htons(checksum((unsigned char *)&pshdr,(unsigned char *)tcp, htons(ip->totlen)-20));
			//printf("\n++++ pseudo header +++\n");
			//stampabytes((unsigned char* )&pshdr,12);
			//printf("\n++++ SYN + ACK  +++\n");
			printf("In uscita:\n");
			stampabytes((unsigned char* ) ip,t-14);
        		nexthop = gateway;
			if (mac_cache[0]==0){
				if(!trovamac(mac_incognito,nexthop)){
        				stampabytes(mac_incognito,6);
					memcpy(mac_cache,mac_incognito,6);
					}
				else
        				printf("trovamac fallita\n");
				}
			else 
				memcpy(mac_incognito,mac_cache,6);

			crea_eth(eth,mymac,mac_incognito,0x0800);
			bzero(&sll,sizeof(struct sockaddr_ll));
			sll.sll_ifindex = if_nametoindex("eth0");
			printf("ifindex = %d\n",sll.sll_ifindex);
			len = sizeof(sll);
			t = sendto(s, buffer, 14+20+htons(pshdr.tcp_len), 0, (struct sockaddr *) &sll, len);
			if (t == -1 ){
 				perror("sendto fallita");
				return 1;
				}

			
       	 }
	 /* else if(tcp->d_port == htons(39600)){
			printf("Ritornato!!!!\n");
			stampabytes((unsigned char* ) ip,t-14);
			return 1;
		} */
	}		
}
}


int trovamac(unsigned char *mac_incognito, unsigned char*dstip)
{
struct sockaddr_ll sll;
int len,i,t,m,s;
char ifname[50];
unsigned char buffer[1500];
struct arp_packet * arp;
struct eth_frame * eth;
eth = (struct eth_frame *) buffer;
arp = (struct arp_packet *) eth->payload;
crea_eth(eth,mymac,broadcast,0x0806);
crea_arp(arp,mymac,myip,dstip);
//stampabytes(buffer,14+sizeof(struct arp_packet));
s = socket(AF_PACKET, SOCK_RAW,htons(ETH_P_ALL));
bzero(&sll,sizeof(struct sockaddr_ll));
sll.sll_family = AF_PACKET;
//for(i=1;i<4;i++) 
//	printf("%s\n",if_indextoname(i,ifname));

printf("\n");
sll.sll_ifindex = if_nametoindex("eth0");
len = sizeof(sll);
t = sendto(s, buffer, 14+sizeof(struct arp_packet), 0, (struct sockaddr *) &sll, len);
if (t == -1 ){
	 perror("sendto fallita");
	return 1;
}
for(m=0;m<1000;m++){
t = recvfrom(s, buffer, 1500, 0, (struct sockaddr *) &sll, &len);
if (t == -1 ){
 perror("recvfrom fallita");
return 1;
}
if (eth->type == htons(0x0806)){
	if(arp->opcode == htons(2))
		if(!memcmp(arp->psrc,dstip,4)){
		memcpy(mac_incognito,arp->hsrc,6);
		return 0;
		}
	}
}
return 1;
}

void crea_eth(struct eth_frame * e, unsigned char * src, unsigned char*dst,unsigned short type)
{
int i;
e->type = htons(type);
for(i=0;i<6;i++) e->src[i]=src[i];
for(i=0;i<6;i++) e->dst[i]=dst[i];
} 

void crea_arp(struct arp_packet* a,unsigned char *mymac,unsigned char *myip,unsigned char *dstip){
int i;
a-> htype = htons(1);
a-> ptype = htons(0x0800);
a-> hlen = 6;
a-> plen = 4;
a-> opcode =htons(1);
for(i=0;i<6;i++) a-> hsrc[i]=mymac[i];
for(i=0;i<4;i++) a-> psrc[i]=myip[i];
for(i=0;i<6;i++) a-> hdst[i]=0;
for(i=0;i<4;i++) a-> pdst[i]=dstip[i];
}

void stampabytes(unsigned char * buffer, int quanti)
{
int i;
for(i=0;i<quanti;i++){
        if (!(i&0x3)) printf("\n");
        if ((buffer[i]<'0') || (buffer[i]>'z')) printf("%.2X(%.3d  ) ",buffer[i],buffer[i]);
        else printf("%.2X(%.3d %c) ",buffer[i],buffer[i],buffer[i]);
        }
printf("\n");
}

void crea_ip(struct pseudoheader *pshdr, struct ip_datagram* ip, int payloadlen,unsigned char payloadtype,unsigned int ipdest )
{

ip->ver_ihl = 0x45; //(4<<4) | 5;
ip->tos= 0;
pshdr->tcp_len=htons(payloadlen);
pshdr->zero=0;
ip->totlen=htons(20+payloadlen);
ip->id=htons(0xABCD);
ip->flags_offs=htons(0);
ip->ttl=128;
pshdr->proto=ip->protocol=payloadtype;
pshdr->s_addr=ip->src=*((unsigned int*)myip);
pshdr->d_addr=ip->dst=ipdest;
ip->checksum=htons(0);
ip->checksum=htons(checksum((unsigned char*) NULL,(unsigned char*) ip, 20));
};


unsigned short checksum(unsigned char * pshdr,  unsigned char * buffer, int len){
int i;
unsigned short *p,*q;
unsigned int tot=0;
unsigned char tmp,torestore=0;
p = (unsigned short *) buffer;
q = (unsigned short *) pshdr;
if (len&1){ 
	tmp=buffer[len]; 
	buffer[len]=0;
	len = len + 1;
	torestore=1;
}
if (q!=NULL)
	for(i=0;i<12/2;i++){
		tot = tot + htons(q[i]);
		if (tot&0x10000) tot = (tot&0xFFFF)+1;
		}
for(i=0;i<len/2;i++) {
	tot = tot + htons(p[i]);
	if (tot&0x10000) tot = (tot&0xFFFF)+1;
	}
if(torestore) buffer[len-1]=tmp;
return (unsigned short)0xFFFF-tot; 
}

void crea_tcp(struct tcp_segment * tcp, unsigned short srcport, unsigned short dstport, struct pseudoheader* pshdr)
{
tcp-> s_port=htons(srcport);
tcp-> d_port=htons(dstport);
tcp-> seq=htonl(0xCCCCDDDD);
tcp-> ack=htons(0);
tcp-> d_offs_res=0x50;
tcp-> flags=0x02;
tcp-> win=htons(0);
tcp-> checksum=htons(0);
tcp-> urgp=htons(0);
tcp->checksum = htons(checksum((unsigned char *)pshdr,(unsigned char *)tcp,20));

}
void crea_icmp_echo(struct icmp_packet * icmp){
int i;
icmp->type=8;
icmp->code=0;
icmp->checksum=htons(0);
icmp->id=htons(0x1234);
icmp->seq = htons(1);
for(i=0;i<20;i++) icmp->payload[i]=i;
icmp->checksum = htons(checksum((unsigned char*) NULL,(unsigned char*)icmp,28));
}

