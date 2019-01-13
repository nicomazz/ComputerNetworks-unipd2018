#include <stdio.h>
#include <strings.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> /* See NOTES */

#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include <arpa/inet.h>

struct timeval tcptime, timezero;    //???
struct sockaddr_ll sll;              //Indirizzo ethernet mio
unsigned int delta_sec, delta_usec;  //???
int primo;                           //???

unsigned char dstip[4] = {88, 80, 187, 80};                      //IP destinazione
unsigned char miomac[6] = {0xf2, 0x3c, 0x91, 0xdb, 0xc2, 0x98};  //ifconfig
unsigned char mioip[4] = {88, 80, 187, 84};
unsigned char gateway[4] = {88, 80, 187, 1};  //comando linux: route -n
unsigned char netmask[4] = {255, 255, 255, 0};
unsigned char broadcastmac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};  //MAC con tutti 1, pacchetto indirizzato a tutti

int progr = 0;  //???

//Segmento TCP
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

// Frame Ethernet
struct eth_frame {
   unsigned char dst[6];
   unsigned char src[6];
   unsigned short type;  //2byte
   char payload[1500];   //ARP o IP
};

// Datagramma IP
struct ip_datagram {
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
   unsigned char payload[1];  //???
};

//Funzione checksum per pacchetto IP
unsigned short int checksum(char *b, int l) {
   unsigned short *p;

   int i;
   unsigned short int tot = 0;
   unsigned short int prec;
   /*Assunzione: l pari */
   p = (unsigned short *)b;

   for (i = 0; i < l / 2; i++) {
      prec = tot;
      tot += htons(p[i]);
      if (tot < prec) tot = tot + 1;
   }
   return (0xFFFF - tot);
}

//Port 0-65536
struct tcp_status {
   unsigned int forwzero, backzero;
   struct timeval timezero;
} st[65536];

//Crea pacchetto IP
void creaip(struct ip_datagram *ip, unsigned int dstip, int payload_len, unsigned char proto) {
   ip->ver_ihl = 0x45;  //primi 4 bit: versione=4, ultimi 4 bit:IHL=5word(32bit*5=20byte)
   ip->tos = 0x0;
   ip->totlen = htons(payload_len + 20);
   ip->id = progr++;  //???
   ip->flag_offs = htons(0);
   ip->ttl = 64;  //Può attraversare massimo 64 router
   ip->proto = proto;
   ip->checksum = 0;
   ip->saddr = *(unsigned int *)mioip;
   ip->daddr = dstip;
   ip->checksum = htons(checksum((unsigned char *)ip, 20));
};

// MAIN
int main(int argc, char **argv) {
   unsigned int ackzero, seqzero, forwzero, backzero;  //Contatori per
   unsigned char buffer[1600];
   unsigned char buffer_1[100000];
   unsigned char buffer_2[100000];
   int len_1 = 0, len_2 = 0;
   unsigned short other_port;  // Variabile di supporto per salvare la porta di sorgente/dest
   int i, l, s, lunghezza;     //???

   struct eth_frame *eth;    //Frame ethernet
   struct ip_datagram *ip;   // Datagramma IP che contiene anche TCP
   struct tcp_segment *tcp;  //Segmento TCP

   //Costruisco un frame ethernet con pacchetto IP-TCP all'interno
   eth = (struct eth_frame *)buffer;
   ip = (struct ip_datagram *)eth->payload;
   tcp = (struct tcp_segment *)ip->payload;

   //APRIRE COMUNICAZIONE
   /*socket restituisce un INT che è un file descriptor
     ovvero l'indice della tabella con tutto ciò che serve per gestire la comunicazione*/
   s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  //AF_INET=IPv4, SOCK_RAW=non elaborato, htons(ETH_P_ALL)

   //Preparo l'indirizzo
   lunghezza = sizeof(struct sockaddr_ll);
   bzero(&sll, sizeof(struct sockaddr_ll));
   sll.sll_ifindex = 3;

   primo = 1;
   while (1) {
      //Ricevo
      l = recvfrom(s, buffer, 1600, 0, (struct sockaddr *)&sll, &lunghezza);

      //Inserisco in tcptime quando ho ricevuto il pacchetto
      gettimeofday(&tcptime, NULL);

      //Se il frame Ethernet ricevuto contiene un pacchetto IP
      if (eth->type == htons(0x0800)){

         if (ip->proto == 1)
            printf("ICMP\n");
         //Se il protocollo all'interno del payload IP è TCP=6
         if (ip->proto == 6) {

            if (tcp->d_port == htons(80) && primo){
               other_port=htons(tcp->s_port);
               timezero=tcptime;
               struct in_addr ip_addr;
               ip_addr.s_addr = ip->saddr;
               printf("The IP address is %s\n", inet_ntoa(ip_addr));;
            }

            //Se hanno come sorgente o destinazione il port80
            if ((tcp->s_port == htons(80)) || (tcp->d_port == htons(80))) {
               //Se la portSorgente=80 in other_port inserisco la portDestinazione sennò il contrario
               unsigned short other_p = (tcp->s_port == htons(80)) ? htons(tcp->d_port) : htons(tcp->s_port);
               if (other_p != other_port) continue; // not the first connection

               //Se il SYN è a 1
               if (tcp->flags & 0x02) {
                  st[other_port].timezero = tcptime;  //Salvo quando ho visto pasare il primo SYN
                  //Salvo il contatore dei byte del segmento in base alla direzione (entrata/uscita)
                  if (tcp->d_port == htons(80)) st[other_port].forwzero = htonl(tcp->seq);
                  if (tcp->s_port == htons(80)) st[other_port].backzero = htonl(tcp->seq);
               }
               //Se la portDestinazine=80 in seqzero inserisco forwzero dell'altra porta altrimenti backzero
               seqzero = (tcp->d_port == htons(80)) ? st[other_port].forwzero : st[other_port].backzero;
               //Se la portSorgente=80 in ackzero inserisco forwzero dell'altra porta altrimenti backzero
               ackzero = (tcp->s_port == htons(80)) ? st[other_port].forwzero : st[other_port].backzero;

               int offset = ntohl(tcp->seq) - seqzero;
               int ip_header_size = (ip->ver_ihl & 0x4) * 4;
               int tcp_header_size = (tcp->d_offs_res >> 4) * 4;
               unsigned char* data_start = (unsigned char *)tcp + tcp_header_size;
               unsigned char* target_buffer = (tcp->d_port == htons(80)) ? buffer_1 : buffer_2;
               size_t data_len = ntohs(ip->totlen) - tcp_header_size - ip_header_size;
               printf("offset:%d, data len: %d, tcp_header: %d, ip_ header: %d\n",offset, data_len, tcp_header_size,ip_header_size);
               memcpy(target_buffer+offset,data_start,data_len);
               if (tcp->d_port == htons(80)) {
                  if (offset+data_len > len_1) len_1 = offset+data_len;
               } else if (offset+data_len > len_2) len_2 = offset+data_len;


               //Vedo la differenza in secondi tra il tempo 0(primo pacchetto) e tempo attuale
               delta_sec = tcptime.tv_sec - st[other_port].timezero.tv_sec;

               //Differenza microsecondi
               if (tcptime.tv_usec < st[other_port].timezero.tv_usec) {
                  delta_sec--;
                  delta_usec = 1000000 + tcptime.tv_usec - st[other_port].timezero.tv_usec;
               } else
                  delta_usec = tcptime.tv_usec - st[other_port].timezero.tv_usec;


               printf("%.5d->%.5d %.2x seq: %.10u ack: %.10u %4.2f\n", 
                     htons(tcp->s_port),
                     htons(tcp->d_port),
                     tcp->flags,
                     htonl(tcp->seq) - seqzero,
                     htonl(tcp->ack) - ackzero,
                     (htonl(tcp->ack) - ackzero) / (double)(delta_sec * 1000000 + delta_usec));

               if (tcp->flags & 0x01){ // FIN a 1               
                  printf("connessione terminata!\ncontenuto buffer 1:\n---\n");
                  for (int i = 0; i < len_1; i++)
                     printf("%c", buffer_1[i]);
                  printf("\n---\n\ncontenuto buffer 2:\n---\n");
                  for (int i = 0; i < len_2; i++)
                     printf("%c", buffer_2[i]);
                  printf("\n");
                  return 0;
               }
            }
         }
      }
   }  //Fine while

}  //Fine main
