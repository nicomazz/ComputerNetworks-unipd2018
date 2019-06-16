#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> /* See NOTES */

struct eth_frame {
   unsigned char dst[6];
   unsigned char src[6];
   unsigned short type;
   char payload[1500];
};

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
   unsigned char payload[1];
};

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

struct tcp_status {
   unsigned int forwzero;
   unsigned int backzero;
   struct timeval timezero;
} st[65536];

struct sockaddr_ll sll;

char forw_buffer[10000];
int forw_size;
char back_buffer[10000];
int back_size;

int main() {
   unsigned int ackzero, seqzero, forwzero, backzero;
   unsigned char buffer[1600];
   unsigned short other_port;
   int i, l, s, lunghezza;

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

   int first_connection_port = 0;

   while (1) {
      l = recvfrom(s, buffer, 1600, 0, (struct sockaddr *)&sll, &lunghezza);

      if (eth->type == htons(0x0800)                                 // IP?
          && ip->proto == 6                                          // TCP?
          && (tcp->s_port == htons(80) || tcp->d_port == htons(80))  // HTTP traffic?
      ) {
         other_port = (tcp->s_port == htons(80)) ? htons(tcp->d_port) : htons(tcp->s_port);

         if (!first_connection_port) {
            first_connection_port = other_port;
         }

         // skip packet if it doesn't belong to this connection
         if (other_port != first_connection_port)
            continue;

         // sync start of communication
         if (tcp->flags & 0x02) {
            if (tcp->d_port == htons(80)) {
               st[other_port].forwzero = htonl(tcp->seq);
               printf("SYN: %hu\n", tcp->seq);
            }

            if (tcp->s_port == htons(80)) {
               st[other_port].backzero = htonl(tcp->seq);
               printf("ACK: %hu\n", tcp->ack);
            }
         }

         // get starting SEQ, ACK
         seqzero = (tcp->d_port == htons(80)) ? st[other_port].forwzero : st[other_port].backzero;
         ackzero = (tcp->s_port == htons(80)) ? st[other_port].forwzero : st[other_port].backzero;
         unsigned int curr_seq = htonl(tcp->seq);

         unsigned short totlen = ntohs(ip->totlen);
         unsigned short ip_header_size = (ip->ver_ihl & 0x0F) * 4;
         unsigned short tcp_header_size = (tcp->d_offs_res >> 4) * 4;  // logic shift right

         unsigned short payload_size = totlen - ip_header_size - tcp_header_size;

         // copy data to buffer and update tot buffer size
         if (tcp->d_port == htons(80)) {
            for (i = 0; i < payload_size; i++)
               forw_buffer[curr_seq - seqzero + i] = tcp->payload[i];

            if ((curr_seq - seqzero + payload_size) > forw_size)
               forw_size = curr_seq - seqzero + payload_size;

         } else {
            for (i = 0; i < payload_size; i++)
               back_buffer[curr_seq - seqzero + i] = tcp->payload[i];

            if ((curr_seq - seqzero + payload_size) > back_size)
               back_size = curr_seq - seqzero + payload_size;
         }

         // handle end of communication
         if (tcp->flags & 0x1) {
            printf("=============Forwarded data ============= \n\n");
            for (i = 0; i < forw_size; i++)
               printf("%c", forw_buffer[i]);

            printf("\n\n==============Received data===========\n\n");
            for (i = 0; i < back_size; i++)
               printf("%c", back_buffer[i]);

            return 0;
         }
      }
   }
}
