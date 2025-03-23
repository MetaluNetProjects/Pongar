#ifndef _LWIPOPTS_COMMON_H
#define _LWIPOPTS_COMMON_H

#include "pico_logger.h"

// 1. Full TCP/mbedtls debug logs
// #include "lwipopts_fulltcpdebug.h"

// 2. No LWIP/mbedtls debug logs.
#include "lwipopts_debug_off.h"

// Send debug traces over UDP to remote host
#define LWIP_PLATFORM_DIAG(x) do {trace x;} while(0)

#define LWIP_PLATFORM_ASSERT(x) do {trace("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__);} while(0)
// Common settings used in most of the pico_w examples
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html for details)

#define PICO_PANIC_FUNCTION pico_logger_panic

// allow override in some examples
#ifndef NO_SYS
#define NO_SYS                      1
#endif
// allow override in some examples
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 0
#endif

// Using malloc to expand as needed
#define MEM_USE_POOLS               0
#define MEM_LIBC_MALLOC             1
#define MEMP_MEM_MALLOC             1
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    (64*1024)
#define TCP_OVERSIZE                1

// General tuning due to low memory
#define TCP_TMR_INTERVAL   25
#define TCP_FAST_INTERVAL   50
#define TCP_SLOW_INTERVAL   100
#define TCP_FIN_WAIT_TIMEOUT   3000
#define TCP_SYN_RCVD_TIMEOUT   3000

#define MEMP_NUM_TCP_SEG            0
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              0
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_MSS                     1460

/* TCP WND must be at least 16 kb to match TLS record size
   or you will get a warning "altcp_tls: TCP_WND is smaller than the RX decrypion buffer, connection RX might stall!" */
#define TCP_WND                     (32 * TCP_MSS)
#define TCP_SND_BUF                 (32 * TCP_MSS)

#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETCONN                0
#define MEM_STATS                   1
#define SYS_STATS                   1
#define MEMP_STATS                  1
#define LINK_STATS                  1
// #define ETH_PAD_SIZE                2
#define LWIP_CHKSUM_ALGORITHM       3
#define LWIP_DHCP                   1
#define LWIP_IPV4                   1
#define LWIP_IPV6                   0
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DNS                    1
#define LWIP_TCP_KEEPALIVE          1
#define LWIP_NETIF_TX_SINGLE_PBUF   1
#define DHCP_DOES_ARP_CHECK         0
#define LWIP_DHCP_DOES_ACD_CHECK    0
#define LWIP_STATS                  1
#define LWIP_STATS_DISPLAY          1

#define MEM_OVERFLOW_CHECK 1
#define LWIP_MEM_ILLEGAL_FREE panic

#define PICO_PRINTF_ALWAYS_INCLUDED 0

#define LWIP_MBEDTLSDIR PICO_MBEDTLS_PATH

#endif /* __LWIPOPTS_H__ */
