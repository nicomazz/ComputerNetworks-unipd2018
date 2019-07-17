/*
   a) mando più ping incrementando di volta in volta il payload dell'icmp. Setto il flag Don't fragment del pacchetto ip.
   Appena vedo che ad una certa dimensione del payload non ricevo più la echo response, questo significa che il pacchetto è stato scartato
   in quanto la dimensione è troppo grande e tale pacchetto non può essere frammentata. 
   la dimesione mtu dell'ultimo ping del quale si ha una echo reply è la dimensione richiesta.
   b)
      1. aggiungo un parametro in crea_icmp_echo che specifica la dimensione del payload
      2. metto la creazione e l'invio del ping in un ciclo for che incrementa ad ogni iterazione la dimensione del payload, di 4 byte alla volta.
      3. setto il flag Don't Fragment nel pacchetto IP
      4. stampo ad ogni iterazione la mtu
      5. se dopo 3 tentativi non ricevo alcun echo reply esco dal ciclo for e stampo la mtu dell'ultimo pacchetto del quale si ha ricevuto la echo reply.
   c) dalle rilevazioni misuro che la mtu più piccola attraverso cui deve transitare il pacchetto IP è: 1500 bytes, verso l'indirizzo 147.162.2.100.
      Questa dimensione è la massima dimensione di un pacchetto IP senza dover essere frammentata durante il percorso di rete.
 */
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

unsigned char miomac[6] = {0xf2, 0x3c, 0x91, 0xdb, 0xc2, 0x98};
unsigned char broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
unsigned char mioip[4] = {88, 80, 187, 84};
unsigned char netmask[4] = {255, 255, 255, 0};
unsigned char gateway[4] = {88, 80, 187, 1};

//unsigned char iptarget[4] = {88,80,187,80};
unsigned char iptarget[4] = {147,162,2,100};
//unsigned char iptarget[4] = {35,185,190,72};


struct sockaddr_ll sll;
struct timeval stop, start;

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

struct icmp_packet
{
   unsigned char type; //8=echo; 0=echo reply
   unsigned char code;
   unsigned short checksum;
   unsigned short id;
   unsigned short seq;
   unsigned char payload[1];
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

void crea_icmp_echo(struct icmp_packet *icmp, unsigned short id, unsigned short seq, unsigned int payload_size) {
   int i;
   icmp->type = 8;
   icmp->code = 0;
   icmp->checksum = htons(0);
   icmp->id = htons(id);
   icmp->seq = htons(seq);
   for (i = 0; i < payload_size; i++)
      icmp->payload[i] = i;
   icmp->checksum = htons(checksum((unsigned char *)icmp, payload_size+8));
}

void crea_ip(struct ip_datagram *ip, int payloadsize, unsigned char proto, unsigned char *ipdest)
{
   ip->ver_ihl = 0x45;
   ip->tos = 0;
   ip->totlen = htons(payloadsize + 20);
   ip->id = htons(0x1177);
   //ip->flag_offs = 0;
   ip->flag_offs = htons(1UL<<14);
   ip->ttl = 128;
   ip->proto = proto;
   ip->checksum = htons(0);
   ip->saddr = *(unsigned int *)mioip;
   ip->daddr = *(unsigned int *)ipdest;
   ip->checksum = htons(checksum((unsigned char *)ip, 20));
};


/* Stampa pacchetto IP */
void stampa_ip( struct ip_datagram* i ){
   unsigned int ihl = ( i->ver_ihl & 0x0F) * 4; // Lunghezza header IP
   unsigned int totlen = htons( i->totlen );    // Lunghezza totale pacchetto
   unsigned int opt_len = ihl-20;         // Lunghezza campo opzioni

   printf( "\n\n ***** PACCHETTO IP *****\n" );
   printf( "Version: %d\n", i->ver_ihl & 0xF0 );
   printf( "IHL (bytes 60max): %d\n", ihl );
   printf( "TOS: %d\n", i->tos );
   printf( "Lunghezza totale: %d\n", totlen );
   printf( "ID: %x\n", htons( i->id ) );
   unsigned char flags = (unsigned char)( htons( i->flag_offs) >>  13);
   printf( "Flags: %d | %d | %d \n", flags & 4, flags & 2, flags & 1 );
   printf( "Fragment Offset: %d\n", htons( i->flag_offs) & 0x1FFF  );
   printf( "TTL: %d\n", i->ttl );
   printf( "Protocol: %d\n", i->proto );
   printf( "Checksum: %x\n", htons( i->checksum ) );

   unsigned char* saddr = ( unsigned char* )&i->saddr;
   unsigned char* daddr = ( unsigned char* )&i->daddr;

   printf( "IP Source: %d.%d.%d.%d\n", saddr[0], saddr[1], saddr[2], saddr[3] );
   printf( "IP Destination: %d.%d.%d.%d\n", daddr[0], daddr[1], daddr[2], daddr[3] );

   if( ihl > 20 ){
      // Stampa opzioni
      printf( "Options: " );
      for(int j=0; j < opt_len ; j++ ){
         printf("%.3d(%.2x) ", i->payload[j], i->payload[j]);
      }
      printf( "\n" );
   }
}

void stampa_icmp_e( struct icmp_packet* i ){
   printf( "\n\n ***** PACCHETTO ICMP *****\n" );
   printf( "Type: %d\n", i->type );
   printf( "Code: %d\n", i->code );
   printf( "Code: 0x%x\n", htons( i->checksum ) );
   printf( "ID: %d\n", htons(i->id) );
   printf( "Sequence: %d\n", htons(i->seq) );
}

int main()
{
   int t, s, i;
   unsigned char mac[6];
   unsigned char buffer[40000];
   struct icmp_packet *icmp;
   struct ip_datagram *ip;
   struct eth_frame *eth;
   int lungh, n, tentativi=0, icmp_payload;

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
   }
   s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
   if (s == -1)
   {
      perror("Socket fallita");
      return 1;
   }
   eth = (struct eth_frame *)buffer;
   ip = (struct ip_datagram *)eth->payload;
   icmp = (struct icmp_packet *)ip->payload;

   for(icmp_payload=20; icmp_payload<30000 && tentativi<3; icmp_payload+=4){
      crea_icmp_echo(icmp, 0x1177, 1, icmp_payload);
      crea_ip(ip, icmp_payload+8, 1, iptarget);
      crea_eth(eth, mac, 0x0800);
      //printf("ICMP/IP/ETH\n");
      //stampa_buffer(buffer, 14 + 20 + 8 + icmp_payload);
      //stampa_ip(ip);
      //stampa_icmp_e(icmp);
      lungh = sizeof(struct sockaddr_ll);
      bzero(&sll, lungh);
      sll.sll_ifindex = 3;
      n = sendto(s, buffer, 14 + 20 + 8 + icmp_payload, 0, (struct sockaddr *)&sll, lungh);
      if (n == -1)
      {
         perror("sendto fallita");
         return 1;
      }
      printf("Inviato icmp_payload=%d Attendo risposta...\n", icmp_payload);

       for(i=0; i<1000; i++){

         n = recvfrom(s, buffer, 30000, 0, (struct sockaddr *)&sll, &lungh);
         //printf("sono qui");
         if (n == -1)
         {
            perror("recvfrom fallita");
            return 1;
         }
         if (eth->type == htons(0x0800))
            if (ip->proto == 1)
               if (icmp->type == 0 && icmp->id==htons(0x1177))
               {
                  printf("ricevuto icmp echo reply. mtu=%d\n", icmp_payload+8+20);
                  //stampa_buffer(buffer, n);
                  break;
               }
      }
      if(i==1000) {
         icmp_payload-=4; //retry
         tentativi++;
      }
   }
   icmp_payload-=4;
   int max_mtu = icmp_payload+8+20;
   printf("max mtu = %d", max_mtu);
}

