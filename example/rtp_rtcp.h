#ifndef _WXH_RTP_HEADER_H_
#define _WXH_RTP_HEADER_H_


#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#ifdef __MACH__
#include <machine/endian.h>
#else
#include <endian.h>
#endif
#endif

#include "inttypes.h"

#define RTP_HEADER_SIZE	12

#ifdef WIN32
#define __BYTE_ORDER  0x1234
#define __BIG_ENDIAN 0x3412
#define __LITTLE_ENDIAN 0x1234
#endif // WIN32


typedef struct rtp_header
{
#if __BYTE_ORDER == __BIG_ENDIAN
	uint16_t version:2;
	uint16_t padding:1;
	uint16_t extension:1;
	uint16_t csrccount:4;
	uint16_t markerbit:1;
	uint16_t type:7;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	uint16_t csrccount:4;
	uint16_t extension:1;
	uint16_t padding:1;
	uint16_t version:2;
	uint16_t type:7;
	uint16_t markerbit:1;
#endif

	uint16_t seq_number;
	uint32_t timestamp;
	uint32_t ssrc;
	uint32_t csrc[16];
} rtp_header;




#endif