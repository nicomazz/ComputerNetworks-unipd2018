#ifndef IN_DEFINES_H
#define IN_DEFINES_H

/*
 * Copied from /usr/include/linux/in.h
 */


/* Standard well-defined IP protocols.  */

#ifndef IPPROTO_IP
#  define IPPROTO_IP		0    /* Dummy protocol for TCP		*/
#endif
#ifndef IPPROTO_ICMP
#  define IPPROTO_ICMP		1    /* Internet Control Message Protocol	*/
#endif
#ifndef IPPROTO_IGMP
#  define IPPROTO_IGMP		2    /* Internet Group Management Protocol	*/
#endif
#ifndef IPPROTO_IPIP
#  define IPPROTO_IPIP		4    /* IPIP tunnels (older KA9Q tunnels use 94) */
#endif
#ifndef IPPROTO_TCP
#  define IPPROTO_TCP		6    /* Transmission Control Protocol	*/
#endif
#ifndef IPPROTO_EGP
#  define IPPROTO_EGP		8    /* Exterior Gateway Protocol		*/
#endif
#ifndef IPPROTO_PUP
#  define IPPROTO_PUP		12    /* PUP protocol				*/
#endif
#ifndef IPPROTO_UDP
#  define IPPROTO_UDP		17    /* User Datagram Protocol		*/
#endif
#ifndef IPPROTO_IDP
#  define IPPROTO_IDP		22    /* XNS IDP protocol			*/
#endif
#ifndef IPPROTO_TP
#  define IPPROTO_TP		29    /* SO Transport Protocol Class 4	*/
#endif
#ifndef IPPROTO_DCCP
#  define IPPROTO_DCCP		33    /* Datagram Congestion Control Protocol */
#endif
#ifndef IPPROTO_IPV6
#  define IPPROTO_IPV6		41    /* IPv6-in-IPv4 tunnelling		*/
#endif
#ifndef IPPROTO_RSVP
#  define IPPROTO_RSVP		46    /* RSVP Protocol			*/
#endif
#ifndef IPPROTO_GRE
#  define IPPROTO_GRE		47    /* Cisco GRE tunnels (rfc 1701,1702)	*/
#endif
#ifndef IPPROTO_ESP
#  define IPPROTO_ESP		50    /* Encapsulation Security Payload protocol */
#endif
#ifndef IPPROTO_AH
#  define IPPROTO_AH		51    /* Authentication Header protocol	*/
#endif
#ifndef IPPROTO_MTP
#  define IPPROTO_MTP	     92    /* Multicast Transport Protocol		*/
#endif
#ifndef IPPROTO_BEETPH
#  define IPPROTO_BEETPH		94    /* IP option pseudo header for BEET	*/
#endif
#ifndef IPPROTO_ENCAP
#  define IPPROTO_ENCAP		98    /* Encapsulation Header			*/
#endif
#ifndef IPPROTO_PIM
#  define IPPROTO_PIM		103    /* Protocol Independent Multicast	*/
#endif
#ifndef IPPROTO_COMP
#  define IPPROTO_COMP		108    /* Compression Header Protocol		*/
#endif
#ifndef IPPROTO_SCTP
#  define IPPROTO_SCTP		132    /* Stream Control Transport Protocol	*/
#endif
#ifndef IPPROTO_UDPLITE
#  define IPPROTO_UDPLITE		136    /* UDP-Lite (RFC 3828)			*/
#endif
#ifndef IPPROTO_MPLS
#  define IPPROTO_MPLS		137    /* MPLS in IP (RFC 4023)		*/
#endif
#ifndef IPPROTO_RAW
#  define IPPROTO_RAW		255    /* Raw IP packets			*/
#endif

  


#ifndef IP_TOS
#  define IP_TOS		1
#endif

#ifndef IP_TTL
#  define IP_TTL		2
#endif

#ifndef IP_HDRINCL
#  define IP_HDRINCL	3
#endif

#ifndef IP_OPTIONS
#  define IP_OPTIONS	4
#endif

#ifndef IP_ROUTER_ALERT
#  define IP_ROUTER_ALERT	5
#endif

#ifndef IP_RECVOPTS
#  define IP_RECVOPTS	6
#endif

#ifndef IP_RETOPTS
#  define IP_RETOPTS	7
#endif

#ifndef IP_PKTINFO
#  define IP_PKTINFO	8
#endif

#ifndef IP_PKTOPTIONS
#  define IP_PKTOPTIONS	9
#endif

#ifndef IP_MTU_DISCOVER
#  define IP_MTU_DISCOVER	10
#endif

#ifndef IP_RECVERR
#  define IP_RECVERR	11
#endif

#ifndef IP_RECVTTL
#  define IP_RECVTTL	12
#endif

#ifndef IP_RECVTOS
#  define	IP_RECVTOS	13
#endif

#ifndef IP_MTU
#  define IP_MTU		14
#endif

#ifndef IP_FREEBIND
#  define IP_FREEBIND	15
#endif

#ifndef IP_IPSEC_POLICY
#  define IP_IPSEC_POLICY	16
#endif

#ifndef IP_XFRM_POLICY
#  define IP_XFRM_POLICY	17
#endif

#ifndef IP_PASSSEC
#  define IP_PASSSEC	18
#endif

#ifndef IP_TRANSPARENT
#  define IP_TRANSPARENT	19
#endif




/* BSD compatibility */
#ifndef IP_RECVRETOPTS
#  define IP_RECVRETOPTS	IP_RETOPTS
#endif




/* TProxy original addresses */
#ifndef IP_ORIGDSTADDR
#  define IP_ORIGDSTADDR       20
#endif

#ifndef IP_RECVORIGDSTADDR
#  define IP_RECVORIGDSTADDR   IP_ORIGDSTADDR
#endif


#ifndef IP_MINTTL
#  define IP_MINTTL       21
#endif

#ifndef IP_NODEFRAG
#  define IP_NODEFRAG     22
#endif

#ifndef IP_CHECKSUM
#  define IP_CHECKSUM	23
#endif

#ifndef IP_BIND_ADDRESS_NO_PORT
#  define IP_BIND_ADDRESS_NO_PORT	24
#endif




/* IP_MTU_DISCOVER values */

#ifndef IP_PMTUDISC_DONT
#  define IP_PMTUDISC_DONT		0	/* Never send DF frames */
#endif

#ifndef IP_PMTUDISC_WANT
#  define IP_PMTUDISC_WANT		1	/* Use per route hints	*/
#endif

#ifndef IP_PMTUDISC_DO
#  define IP_PMTUDISC_DO			2	/* Always DF		*/
#endif

#ifndef IP_PMTUDISC_PROBE
#  define IP_PMTUDISC_PROBE		3       /* Ignore dst pmtu      */
#endif


/* Always use interface mtu (ignores dst pmtu) but don't set DF flag.
 * Also incoming ICMP frag_needed notifications will be ignored on
 * this socket to prevent accepting spoofed ones.
 */
#ifndef IP_PMTUDISC_INTERFACE
#  define IP_PMTUDISC_INTERFACE		4
#endif

/* weaker version of IP_PMTUDISC_INTERFACE, which allos packets to get
 * fragmented if they exeed the interface mtu
 */
#ifndef IP_PMTUDISC_OMIT
#  define IP_PMTUDISC_OMIT		5
#endif


#ifndef IP_MULTICAST_IF
#  define IP_MULTICAST_IF			32
#endif

#ifndef IP_MULTICAST_TTL
#  define IP_MULTICAST_TTL 		33
#endif

#ifndef IP_MULTICAST_LOOP
#  define IP_MULTICAST_LOOP 		34
#endif

#ifndef IP_ADD_MEMBERSHIP
#  define IP_ADD_MEMBERSHIP		35
#endif

#ifndef IP_DROP_MEMBERSHIP
#  define IP_DROP_MEMBERSHIP		36
#endif

#ifndef IP_UNBLOCK_SOURCE
#  define IP_UNBLOCK_SOURCE		37
#endif

#ifndef IP_BLOCK_SOURCE
#  define IP_BLOCK_SOURCE			38
#endif

#ifndef IP_ADD_SOURCE_MEMBERSHIP
#  define IP_ADD_SOURCE_MEMBERSHIP	39
#endif

#ifndef IP_DROP_SOURCE_MEMBERSHIP
#  define IP_DROP_SOURCE_MEMBERSHIP	40
#endif

#ifndef IP_MSFILTER
#  define IP_MSFILTER			41
#endif

#ifndef MCAST_JOIN_GROUP
#  define MCAST_JOIN_GROUP		42
#endif

#ifndef MCAST_BLOCK_SOURCE
#  define MCAST_BLOCK_SOURCE		43
#endif

#ifndef MCAST_UNBLOCK_SOURCE
#  define MCAST_UNBLOCK_SOURCE		44
#endif

#ifndef MCAST_LEAVE_GROUP
#  define MCAST_LEAVE_GROUP		45
#endif

#ifndef MCAST_JOIN_SOURCE_GROUP
#  define MCAST_JOIN_SOURCE_GROUP		46
#endif

#ifndef MCAST_LEAVE_SOURCE_GROUP
#  define MCAST_LEAVE_SOURCE_GROUP	47
#endif

#ifndef MCAST_MSFILTER
#  define MCAST_MSFILTER			48
#endif

#ifndef IP_MULTICAST_ALL
#  define IP_MULTICAST_ALL		49
#endif

#ifndef IP_UNICAST_IF
#  define IP_UNICAST_IF			50
#endif


#ifndef MCAST_EXCLUDE
#  define MCAST_EXCLUDE	0
#endif

#ifndef MCAST_INCLUDE
#  define MCAST_INCLUDE	1
#endif


/* These need to appear somewhere around here */
#ifndef IP_DEFAULT_MULTICAST_TTL
#  define IP_DEFAULT_MULTICAST_TTL        1
#endif

#ifndef IP_DEFAULT_MULTICAST_LOOP
#  define IP_DEFAULT_MULTICAST_LOOP       1
#endif




#endif
