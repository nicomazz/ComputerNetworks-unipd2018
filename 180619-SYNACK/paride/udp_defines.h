#ifndef UDP_DEFINES_H
#define UDP_DEFINES_H

/* 
 * Copied from /usr/include/linux/udp.h
 */





/* UDP socket options */
#ifndef UDP_CORK
#  define UDP_CORK	1	/* Never send partially complete segments */
#endif

#ifndef UDP_ENCAP
#  define UDP_ENCAP	100	/* Set the socket to accept encapsulated packets */
#endif

#ifndef UDP_NO_CHECK6_TX
#  define UDP_NO_CHECK6_TX 101	/* Disable sending checksum for UDP6X */
#endif

#ifndef UDP_NO_CHECK6_RX
#  define UDP_NO_CHECK6_RX 102	/* Disable accpeting checksum for UDP6 */
#endif





/* UDP encapsulation types */
#ifndef UDP_ENCAP_ESPINUDP_NON_IKE
#  define UDP_ENCAP_ESPINUDP_NON_IKE	1 /* draft-ietf-ipsec-nat-t-ike-00/01 */
#endif

#ifndef UDP_ENCAP_ESPINUDP
#  define UDP_ENCAP_ESPINUDP	2 /* draft-ietf-ipsec-udp-encaps-06 */
#endif

#ifndef UDP_ENCAP_L2TPINUDP
#  define UDP_ENCAP_L2TPINUDP	3 /* rfc2661 */
#endif






#endif
