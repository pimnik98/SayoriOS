#pragma once

#include "common.h"
#include "net/cards.h"

#define ETHERNET_TYPE_IPV4		0x0800	///< Internet Protocol v4
#define ETHERNET_TYPE_ARP		0x0806	///< Address Resolution Protocol
#define ETHERNET_TYPE_WoL		0x0842	///< Wake-on-LAN
#define ETHERNET_TYPE_SRP		0x22EA	///< Stream Reservation Protocol
#define ETHERNET_TYPE_AVTP		0x22F0	///< Audio Video Transport Protocol
#define ETHERNET_TYPE_IETF		0x22F3	///< IETF TRILL Protocol
#define ETHERNET_TYPE_DECMOP	0x6002	///< DEC MOP RC
#define ETHERNET_TYPE_DECNET	0x6003	///< DECnet Phase Iv, DNA Routing
#define ETHERNET_TYPE_DECLAT	0x6004	///< DEC LAT
#define ETHERNET_TYPE_RASP		0x8035	///< Reverse Address Resolution Protocol
#define ETHERNET_TYPE_ATE		0x809B	///< AppleTalk (Ethertalk)
#define ETHERNET_TYPE_ATARP		0x80F3	///< AppleTalk Address Resolution Protocol
#define ETHERNET_TYPE_VLAN		0x8100	///< VLAN-tagged frame (IEEE 802.1Q)
#define ETHERNET_TYPE_SLPP		0x8102	///< Simple Loop Prevention Protocol
#define ETHERNET_TYPE_VLACP		0x8103	///< Virtual Link Aggregation Control Protocol
#define ETHERNET_TYPE_IPX		0x8137	////< IPX
#define ETHERNET_TYPE_QNX		0x8204	///< QNX Qnet
#define ETHERNET_TYPE_IPV6		0x86DD	///< Internet Protocol v6
#define ETHERNET_TYPE_EFC		0x8808	///< Ethernet flow control
#define ETHERNET_TYPE_ESP		0x8809
///< Ethernet Slow Protocols
#define ETHERNET_TYPE_COBRA		0x8819	///< CobraNet
#define ETHERNET_TYPE_MPLSU		0x8847	///< MPLS unicast
#define ETHERNET_TYPE_MPLSM		0x8848	///< MPLS multicast
#define ETHERNET_TYPE_PPPoEDS	0x8863	////< PPPoE Discovery State
#define ETHERNET_TYPE_PPPoESS	0x8864	///< PPPoE Session Stage
#define ETHERNET_TYPE_HP1MME	0x887B	///< HomePlug 1.0 MME
#define ETHERNET_TYPE_EAPoLAN	0x888E	///< EAP over LAN (IEEE 802.1X)
#define ETHERNET_TYPE_PROFINET	0x8892	///< PROFINET Protocol
#define ETHERNET_TYPE_HYPERSCSI	0x889A	///< HyperSCSI (SCSI over Ethernet)
#define ETHERNET_TYPE_ATAoE		0x88A2	///< ATA over Ethernet
#define ETHERNET_TYPE_ECAT		0x88A4	///< EtherCAT Protocol
#define ETHERNET_TYPE_SVLAN		0x88A8	///< Service VLAN tag identifier
#define ETHERNET_TYPE_POWLINK	0x88AB	///< Ethernet Powerlink
#define ETHERNET_TYPE_GOOSE		0x88B8	////< Generic Object Oriented Substatuion event
#define ETHERNET_TYPE_GSE		0x88B9	///< Generic Substation Events
#define ETHERNET_TYPE_SV		0x88BA	///< Sampled Value Transmission
#define ETHERNET_TYPE_MTRMN		0x88BA	///< MikroTik RoMON (unofficial)
#define ETHERNET_TYPE_LLDP		0x88CC	///< Link Layer Discovery Protocol
#define ETHERNET_TYPE_SCSIII	0x88CD	///< SECOS III
#define ETHERNET_TYPE_HPGPHY	0x88E1	///< HomePlug Green PHY
#define ETHERNET_TYPE_MRP		0x88E3	///< Media Redundancy Protocol
#define ETHERNET_TYPE_MACSEC	0x88E5	///< IEEE 802.1AE MAC Security
#define ETHERNET_TYPE_PBB		0x88E7	///< Provider Backbone Bridges
#define ETHERNET_TYPE_PTP		0x88F7	///< Precison Time Protocol
#define ETHERNET_TYPE_NCSI		0x88F8	///< NC-SI
#define ETHERNET_TYPE_PRP		0x88FB	///< Parallel Redundancy Protocol
#define ETHERNET_TYPE_CFM		0x8902	///< IEEE 802.1ag Connectivy Fault Management
#define ETHERNET_TYPE_FCoE		0x8906	///< Fible Channel over Ethenet
#define ETHERNET_TYPE_FCoEI		0x8914	///< FCoE Initialization Protocol
#define ETHERNET_TYPE_RoCE		0x8915	///< RDMA over Converted Ethernet
#define ETHERNET_TYPE_TTE		0x891D	///< TTEthernet Protocol Control Flame
#define ETHERNET_TYPE_1905		0x893A	///< 1905.1 IEEE Protocol
#define ETHERNET_TYPE_HSR		0x892F	///< High-availability Seamless Redundancy
#define ETHERNET_TYPE_ECTP		0x9000	///< ethernet Configuration Testing Protocol
#define ETHERNET_TYPE_RT		0xF1C1	///< Redundancy Tag

#define ETH_IPv6_HEAD_ICMPv6	0x3a	///< ICMPv6 (IPv6 заголовок)
#define ETH_IPv6_HEAD_UDP		0x11	///< UDP (IPv6 заголовок) 

#define ETH_IPv4_HEAD_ICMPv4	1		///< ICMPv4 (IPv4 заголовок)
#define ETH_IPv4_HEAD_UDP		0x11	///< UDP (IPv4 заголовок) еще как вариант 17
#define ETH_IPv4_HEAD_TCP		6		///< TCP (IPv4 заголовок)

#define ETH_ICMPv4_TYPE_PING	8		///< Нас пингуют через ICMPv4 > IPv4
#define ETH_ICMPv6_TYPE_PING	128		///< Нас пингуют через ICMPv6 > IPv6

#define HARDWARE_TYPE_ETHERNET 0x01

typedef struct ethernet_frame {
  uint8_t dest_mac[6];
  uint8_t src_mac[6];
  uint16_t type;
  uint8_t data[];
} __attribute__((packed)) ethernet_frame_t;	///< 14+

typedef struct {
  uint8_t Type;
  uint8_t Size;
  uint8_t MAC[6];
} __attribute__((packed)) ETH_IPv6_OPT_PKG;	///< 8 байт

typedef struct {
	uint8_t Version;
	uint8_t Flow[3];
	uint16_t PayLoad;
	uint8_t NextHead;
	uint8_t HopLimit;
	uint16_t Source[8];
	uint16_t Destination[8];
} __attribute__((packed)) ETH_IPv6_PKG;	///< 40 байт + 8 байт


typedef struct {
	uint8_t Type;
	uint8_t Code;
	uint16_t CheckSum;
	uint32_t Reserved;
	ETH_IPv6_OPT_PKG Opt;
} __attribute__((packed)) ETH_ICMPv6_PKG;	///< 16 байт

typedef struct {
	uint8_t HeaderLength : 4;
	uint8_t Version : 4;
	uint8_t DSF;
	uint16_t TotalLength;
	uint16_t ID;
	uint16_t Flags;
	uint8_t TimeLife;
	uint8_t Protocol;
	uint16_t Checksum;
	uint8_t Source[4];
	uint8_t Destination[4];
} __attribute__((packed)) ETH_IPv4_PKG;	///< 20 байт // NDRAEY: PKT is not PKG!


typedef struct {
  uint16_t SourcePort;
  uint16_t DestinationPort;
  uint16_t Length;
  uint16_t CheckSum;
} __attribute__((packed)) ETH_UDP_PKG;	///< 8 байт


typedef struct {
  uint8_t Type;
  uint8_t Code;
  uint16_t CheckSum;
  uint16_t IDBE;
  uint16_t SNBE;
  uint8_t Timestamp[8];
} __attribute__((packed)) ETH_ICMPv4_PKG;	///< 16 байт


void ethernet_send_packet(netcard_entry_t* card, uint8_t* dest_mac, uint8_t* data, size_t len, uint16_t type);
void ethernet_handle_packet(netcard_entry_t *card, ethernet_frame_t *packet, size_t len);