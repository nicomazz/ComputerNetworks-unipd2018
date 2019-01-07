#include <stdio.h>
#include <strings.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> /* See NOTES */

#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include <arpa/inet.h>
#include <string.h>
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
char * ipToText(unsigned int ip){
	struct in_addr ip_addr;
	ip_addr.s_addr = ip;
	return inet_ntoa(ip_addr);
}

char forward_buffer[100000];
char backward_buffer[100000];
// MAIN
int main(int argc, char **argv) {
	unsigned int ackzero, seqzero, forwzero, backzero;  //Contatori per
	unsigned char buffer[1600];
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

	unsigned short s_p_tg = 0, d_p_tg = 0;
	unsigned int saddr = 0, daddr = 0;
	char syn_preso = 0;
	char syn_ack_preso = 0;
	struct timeval first_tcptime;
	unsigned int sseq_st = 0;
	unsigned int dseq_st = 0;
	int max_forward = 0;
	int max_backward = 0;
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
				// if (tcp->d_port == htons(80) && primo){
				//             other_port=htons(tcp->s_port);
				//              timezero=tcptime;
				// primo=0;
				// }

				//Se hanno come sorgente o destinazione il port80
				if ((tcp->s_port != htons(80)) && (tcp->d_port != htons(80))) continue;
				//Se la portSorgente=80 in other_port inserisco la portDestinazione sennò il contrario
				//	other_port = (tcp->s_port == htons(80)) ? htons(tcp->d_port) : htons(tcp->s_port);

				//risposta al primo pacchetto (syn e ack a 1)
				if (syn_preso && !syn_ack_preso && tcp->flags & 0x12){
					dseq_st = ntohl(tcp->seq);
					syn_ack_preso = 1;
				}
				//Se il SYN è a 1, e ack a 0 (il primo davvero)
				else if (!syn_ack_preso && !syn_preso && tcp->flags & 0x02 && !(tcp->flags & 0x10)) {
					s_p_tg = tcp->s_port;
					d_p_tg = tcp->d_port;
					saddr = ip->saddr;
					daddr = ip->daddr;
					first_tcptime = tcptime;
					sseq_st = ntohl(tcp->seq);
					char sa[20]; char * c = ipToText(saddr); strcpy(sa,c);
					char da[20]; c = ipToText(daddr); strcpy(da,c);
					printf("intercettata una connessione tra %s e %s su porte %d->%d\n",sa,da, ntohs(s_p_tg),ntohs(d_p_tg));
					syn_preso = 1;
				}
				char type = 0;
#define FORWARD 1
#define BACKWARD 2	
				if ( tcp->s_port == s_p_tg && tcp->d_port == d_p_tg && 
						ip-> saddr == saddr && ip->daddr == daddr)type = FORWARD;
				else if (tcp->s_port == d_p_tg && tcp->d_port == s_p_tg &&
						ip->saddr == daddr && ip->daddr == saddr) type = BACKWARD;

				if (!type) {
					//	printf("intercettato pacchetto di un'altra connessione, skippo\n");
					continue;
				}

				if (tcp->flags & 0x01){// chiusura connessione
					printf("Connessione in chiusura. Buffer 1:\n\n");
					for (int i = 0; i < max_forward;i++) printf("%c",forward_buffer[i]);
					printf("\n\n buffer 2:\n");
					for (int i = 0; i < max_backward;i++) printf("%c",backward_buffer[i]);
					return 0;
				}
				seqzero = (type == FORWARD) ? sseq_st:dseq_st;
				ackzero = (type == FORWARD) ? dseq_st:sseq_st;
				//Vedo la differenza in secondi tra il tempo 0(primo pacchetto) e tempo attuale
				delta_sec = tcptime.tv_sec - first_tcptime.tv_sec;

				//Differenza microsecondi
				if (tcptime.tv_usec < first_tcptime.tv_usec) {
					delta_sec--;
					delta_usec = 1000000 + tcptime.tv_usec - first_tcptime.tv_usec;
				} else
					delta_usec = tcptime.tv_usec - first_tcptime.tv_usec;

				printf("%d->%d seq: %u ack: %u  %.6d\n",ntohs(tcp->s_port),ntohs(tcp->d_port), ntohl(tcp->seq)-seqzero, ntohl(tcp->ack)-ackzero,delta_usec);

				int ip_header_size = (4 * (ip->ver_ihl & 0x0F));
			  	int tcp_header_size = 	((tcp->d_offs_res & 0xF0) >> 2); 

				int tcp_data_size = ntohs(ip->totlen) - ip_header_size - tcp_header_size;
				//printf("tcp data size: %d\n",tcp_data_size);

				int offset = ntohl(tcp->seq)-seqzero;
				char * dest = type == FORWARD ? forward_buffer : backward_buffer;
				memcpy(dest+offset,tcp->payload,tcp_data_size);
				if (type == FORWARD) max_forward = offset+tcp_data_size;
				else max_backward = offset+tcp_data_size;
			//	for (int i = 0; i < tcp_data_size; i++)
			//		printf("%c",tcp->payload[i]);
			//	printf("\n");
				//Per ogni pacchetto stampo una riga che dice i dati, stma della banda,...
				//printf("%.4d.%.6d %.5d->%.5d %.2x %.10u %.10u %.5u %4.2f\n", delta_sec, delta_usec, htons(tcp->s_port), htons(tcp->d_port), tcp->flags, htonl(tcp->seq) - seqzero, htonl(tcp->ack) - ackzero, htons(tcp->win), (htonl(tcp->ack) - ackzero) / (double)(delta_sec * 1000000 + delta_usec));

			}
		}
	}  //Fine while

}  //Fine main
