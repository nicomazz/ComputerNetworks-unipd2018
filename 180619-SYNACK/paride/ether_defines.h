#ifndef IF_ETHER_DEFINES_H
#define IF_ETHER_DEFINES_H



/* 
 * Copied from /usr/include/if_ether.h
 */






/*
 *	IEEE 802.3 Ethernet magic constants.  The frame sizes omit the preamble
 *	and FCS/CRC (frame check sequence).
 */

#ifndef ETH_ALEN
#  define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#endif

#ifndef ETH_HLEN
#  define ETH_HLEN	14		/* Total octets in header.	 */
#endif

#ifndef ETH_ZLEN
#  define ETH_ZLEN	60		/* Min. octets in frame sans FCS */
#endif

#ifndef ETH_DATA_LEN
#  define ETH_DATA_LEN	1500		/* Max. octets in payload	 */
#endif

#ifndef ETH_FRAME_LEN
#  define ETH_FRAME_LEN	1514		/* Max. octets in frame sans FCS */
#endif

#ifndef ETH_FCS_LEN
#  define ETH_FCS_LEN	4		/* Octets in the FCS		 */
#endif





/*
 *	These are the defined Ethernet Protocol ID's.
 */

#ifndef ETH_P_LOOP
#  define ETH_P_LOOP	0x0060		/* Ethernet Loopback packet	*/
#endif

#ifndef ETH_P_PUP
#  define ETH_P_PUP	0x0200		/* Xerox PUP packet		*/
#endif

#ifndef ETH_P_PUPAT
#  define ETH_P_PUPAT	0x0201		/* Xerox PUP Addr Trans packet	*/
#endif

#ifndef ETH_P_IP
#  define ETH_P_IP	0x0800		/* Internet Protocol packet	*/
#endif

#ifndef ETH_P_X25
#  define ETH_P_X25	0x0805		/* CCITT X.25			*/
#endif

#ifndef ETH_P_ARP
#  define ETH_P_ARP	0x0806		/* Address Resolution packet	*/
#endif

#ifndef ETH_P_BPQ
#  define	ETH_P_BPQ	0x08FF		/* G8BPQ AX.25 Ethernet Packet	[ NOT AN OFFICIALLY REGISTERED ID ] */
#endif

#ifndef ETH_P_IEEEPUP
#  define ETH_P_IEEEPUP	0x0a00		/* Xerox IEEE802.3 PUP packet */
#endif

#ifndef ETH_P_IEEEPUPAT
#  define ETH_P_IEEEPUPAT	0x0a01		/* Xerox IEEE802.3 PUP Addr Trans packet */
#endif

#ifndef ETH_P_DEC
#  define ETH_P_DEC       0x6000          /* DEC Assigned proto           */
#endif

#ifndef ETH_P_DNA_DL
#  define ETH_P_DNA_DL    0x6001          /* DEC DNA Dump/Load            */
#endif

#ifndef ETH_P_DNA_RC
#  define ETH_P_DNA_RC    0x6002          /* DEC DNA Remote Console       */
#endif

#ifndef ETH_P_DNA_RT
#  define ETH_P_DNA_RT    0x6003          /* DEC DNA Routing              */
#endif

#ifndef ETH_P_LAT
#  define ETH_P_LAT       0x6004          /* DEC LAT                      */
#endif

#ifndef ETH_P_DIAG
#  define ETH_P_DIAG      0x6005          /* DEC Diagnostics              */
#endif

#ifndef ETH_P_CUST
#  define ETH_P_CUST      0x6006          /* DEC Customer use             */
#endif

#ifndef ETH_P_SCA
#  define ETH_P_SCA       0x6007          /* DEC Systems Comms Arch       */
#endif

#ifndef ETH_P_TEB
#  define ETH_P_TEB	0x6558		/* Trans Ether Bridging		*/
#endif

#ifndef ETH_P_RARP
#  define ETH_P_RARP      0x8035		/* Reverse Addr Res packet	*/
#endif

#ifndef ETH_P_ATALK
#  define ETH_P_ATALK	0x809B		/* Appletalk DDP		*/
#endif

#ifndef ETH_P_AARP
#  define ETH_P_AARP	0x80F3		/* Appletalk AARP		*/
#endif

#ifndef ETH_P_8021Q
#  define ETH_P_8021Q	0x8100          /* 802.1Q VLAN Extended Header  */
#endif

#ifndef ETH_P_IPX
#  define ETH_P_IPX	0x8137		/* IPX over DIX			*/
#endif

#ifndef ETH_P_IPV6
#  define ETH_P_IPV6	0x86DD		/* IPv6 over bluebook		*/
#endif

#ifndef ETH_P_PAUSE
#  define ETH_P_PAUSE	0x8808		/* IEEE Pause frames. See 802.3 31B */
#endif

#ifndef ETH_P_SLOW
#  define ETH_P_SLOW	0x8809		/* Slow Protocol. See 802.3ad 43B */
#endif

#ifndef ETH_P_WCCP
#  define ETH_P_WCCP	0x883E		/* Web-cache coordination protocol
#endif

	
#ifndef ETH_P_PPP_DISC				 * defined in draft-wilson-wrec-wccp-v2-00.txt */
#  define ETH_P_PPP_DISC	0x8863		/* PPPoE discovery messages     */
#endif

#ifndef ETH_P_PPP_SES
#  define ETH_P_PPP_SES	0x8864		/* PPPoE session messages	*/
#endif

#ifndef ETH_P_MPLS_UC
#  define ETH_P_MPLS_UC	0x8847		/* MPLS Unicast traffic		*/
#endif

#ifndef ETH_P_MPLS_MC
#  define ETH_P_MPLS_MC	0x8848		/* MPLS Multicast traffic	*/
#endif

#ifndef ETH_P_ATMMPOA
#  define ETH_P_ATMMPOA	0x884c		/* MultiProtocol Over ATM	*/
#endif

#ifndef ETH_P_ATMFATE
#  define ETH_P_ATMFATE	0x8884		/* Frame-based ATM Transport
#endif

					 * over Ethernet
	
#ifndef ETH_P_PAE				 */
#  define ETH_P_PAE	0x888E		/* Port Access Entity (IEEE 802.1X) */
#endif

#ifndef ETH_P_AOE
#  define ETH_P_AOE	0x88A2		/* ATA over Ethernet		*/
#endif

#ifndef ETH_P_TIPC
#  define ETH_P_TIPC	0x88CA		/* TIPC 			*/
#endif

#ifndef ETH_P_1588
#  define ETH_P_1588	0x88F7		/* IEEE 1588 Timesync */
#endif

#ifndef ETH_P_FCOE
#  define ETH_P_FCOE	0x8906		/* Fibre Channel over Ethernet  */
#endif

#ifndef ETH_P_FIP
#  define ETH_P_FIP	0x8914		/* FCoE Initialization Protocol */
#endif

#ifndef ETH_P_EDSA
#  define ETH_P_EDSA	0xDADA		/* Ethertype DSA [ NOT AN OFFICIALLY REGISTERED ID ] */
#endif






/*
 *	Non DIX types. Won't clash for 1500 types.
 */

#ifndef ETH_P_802_3
#  define ETH_P_802_3	0x0001		/* Dummy type for 802.3 frames  */
#endif

#ifndef ETH_P_AX25
#  define ETH_P_AX25	0x0002		/* Dummy protocol id for AX.25  */
#endif

#ifndef ETH_P_ALL
#  define ETH_P_ALL	0x0003		/* Every packet (be careful!!!) */
#endif

#ifndef ETH_P_802_2
#  define ETH_P_802_2	0x0004		/* 802.2 frames 		*/
#endif

#ifndef ETH_P_SNAP
#  define ETH_P_SNAP	0x0005		/* Internal only		*/
#endif

#ifndef ETH_P_DDCMP
#  define ETH_P_DDCMP     0x0006          /* DEC DDCMP: Internal only     */
#endif

#ifndef ETH_P_WAN_PPP
#  define ETH_P_WAN_PPP   0x0007          /* Dummy type for WAN PPP frames*/
#endif

#ifndef ETH_P_PPP_MP
#  define ETH_P_PPP_MP    0x0008          /* Dummy type for PPP MP frames */
#endif

#ifndef ETH_P_LOCALTALK
#  define ETH_P_LOCALTALK 0x0009		/* Localtalk pseudo type 	*/
#endif

#ifndef ETH_P_CAN
#  define ETH_P_CAN	0x000C		/* Controller Area Network      */
#endif

#ifndef ETH_P_PPPTALK
#  define ETH_P_PPPTALK	0x0010		/* Dummy type for Atalk over PPP*/
#endif

#ifndef ETH_P_TR_802_2
#  define ETH_P_TR_802_2	0x0011		/* 802.2 frames 		*/
#endif

#ifndef ETH_P_MOBITEX
#  define ETH_P_MOBITEX	0x0015		/* Mobitex (kaz@cafe.net)	*/
#endif

#ifndef ETH_P_CONTROL
#  define ETH_P_CONTROL	0x0016		/* Card specific control frames */
#endif

#ifndef ETH_P_IRDA
#  define ETH_P_IRDA	0x0017		/* Linux-IrDA			*/
#endif

#ifndef ETH_P_ECONET
#  define ETH_P_ECONET	0x0018		/* Acorn Econet			*/
#endif

#ifndef ETH_P_HDLC
#  define ETH_P_HDLC	0x0019		/* HDLC frames			*/
#endif

#ifndef ETH_P_ARCNET
#  define ETH_P_ARCNET	0x001A		/* 1A for ArcNet :-)            */
#endif

#ifndef ETH_P_DSA
#  define ETH_P_DSA	0x001B		/* Distributed Switch Arch.	*/
#endif

#ifndef ETH_P_TRAILER
#  define ETH_P_TRAILER	0x001C		/* Trailer switch tagging	*/
#endif

#ifndef ETH_P_PHONET
#  define ETH_P_PHONET	0x00F5		/* Nokia Phonet frames          */
#endif

#ifndef ETH_P_IEEE802154
#  define ETH_P_IEEE802154 0x00F6	/* IEEE802.15.4 frame		*/
#endif



#endif
