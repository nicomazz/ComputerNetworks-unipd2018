#ifndef TCP_DEFINES_H
#define TCP_DEFINES_H

/* 
 * Copied from /usr/include/linux/tcp.h
 */


/*
 * TCP general constants
 */
#ifndef TCP_MSS_DEFAULT
#  define TCP_MSS_DEFAULT		 536U	/* IPv4 (RFC1122, RFC2581) */
#endif

#ifndef TCP_MSS_DESIRED
#  define TCP_MSS_DESIRED		1220U	/* IPv6 (tunneled), EDNS0 (RFC3226) */
#endif


/* TCP socket options */
#ifndef TCP_NODELAY
#  define TCP_NODELAY		1	/* Turn off Nagle's algorithm. */
#endif

#ifndef TCP_MAXSEG
#  define TCP_MAXSEG		2	/* Limit MSS */
#endif

#ifndef TCP_CORK
#  define TCP_CORK		3	/* Never send partially complete segments */
#endif

#ifndef TCP_KEEPIDLE
#  define TCP_KEEPIDLE		4	/* Start keeplives after this period */
#endif

#ifndef TCP_KEEPINTVL
#  define TCP_KEEPINTVL		5	/* Interval between keepalives */
#endif

#ifndef TCP_KEEPCNT
#  define TCP_KEEPCNT		6	/* Number of keepalives before death */
#endif

#ifndef TCP_SYNCNT
#  define TCP_SYNCNT		7	/* Number of SYN retransmits */
#endif

#ifndef TCP_LINGER2
#  define TCP_LINGER2		8	/* Life time of orphaned FIN-WAIT-2 state */
#endif

#ifndef TCP_DEFER_ACCEPT
#  define TCP_DEFER_ACCEPT	9	/* Wake up listener only when data arrive */
#endif

#ifndef TCP_WINDOW_CLAMP
#  define TCP_WINDOW_CLAMP	10	/* Bound advertised window */
#endif

#ifndef TCP_INFO
#  define TCP_INFO		11	/* Information about this connection. */
#endif

#ifndef TCP_QUICKACK
#  define TCP_QUICKACK		12	/* Block/reenable quick acks */
#endif

#ifndef TCP_CONGESTION
#  define TCP_CONGESTION		13	/* Congestion control algorithm */
#endif

#ifndef TCP_MD5SIG
#  define TCP_MD5SIG		14	/* TCP MD5 Signature (RFC2385) */
#endif

#ifndef TCP_THIN_LINEAR_TIMEOUTS
#  define TCP_THIN_LINEAR_TIMEOUTS 16      /* Use linear timeouts for thin streams*/
#endif

#ifndef TCP_THIN_DUPACK
#  define TCP_THIN_DUPACK         17      /* Fast retrans. after 1 dupack */
#endif

#ifndef TCP_USER_TIMEOUT
#  define TCP_USER_TIMEOUT	18	/* How long for loss retry before timeout */
#endif

#ifndef TCP_REPAIR
#  define TCP_REPAIR		19	/* TCP sock is under repair right now */
#endif

#ifndef TCP_REPAIR_QUEUE
#  define TCP_REPAIR_QUEUE	20
#endif

#ifndef TCP_QUEUE_SEQ
#  define TCP_QUEUE_SEQ		21
#endif

#ifndef TCP_REPAIR_OPTIONS
#  define TCP_REPAIR_OPTIONS	22
#endif

#ifndef TCP_FASTOPEN
#  define TCP_FASTOPEN		23	/* Enable FastOpen on listeners */
#endif

#ifndef TCP_TIMESTAMP
#  define TCP_TIMESTAMP		24
#endif

#ifndef TCP_NOTSENT_LOWAT
#  define TCP_NOTSENT_LOWAT	25	/* limit number of unsent bytes in write queue */
#endif

#ifndef TCP_CC_INFO
#  define TCP_CC_INFO		26	/* Get Congestion Control (optional) info */
#endif

#ifndef TCP_SAVE_SYN
#  define TCP_SAVE_SYN		27	/* Record SYN headers for new connections */
#endif

#ifndef TCP_SAVED_SYN
#  define TCP_SAVED_SYN		28	/* Get SYN headers recorded for connection */
#endif




/* for TCP_INFO socket option */
#ifndef TCPI_OPT_TIMESTAMPS
#  define TCPI_OPT_TIMESTAMPS	1
#endif

#ifndef TCPI_OPT_SACK
#  define TCPI_OPT_SACK		2
#endif

#ifndef TCPI_OPT_WSCALE
#  define TCPI_OPT_WSCALE		4
#endif

#ifndef TCPI_OPT_ECN
#  define TCPI_OPT_ECN		8 /* ECN was negociated at TCP session init */
#endif

#ifndef TCPI_OPT_ECN_SEEN
#  define TCPI_OPT_ECN_SEEN	16 /* we received at least one packet with ECT */
#endif

#ifndef TCPI_OPT_SYN_DATA
#  define TCPI_OPT_SYN_DATA	32 /* SYN-ACK acked data in SYN sent or rcvd */
#endif






#endif
