#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

#include "lwipopts_common.h"

#define LWIP_ALTCP               1
#define LWIP_ALTCP_TLS           1
#define LWIP_ALTCP_TLS_MBEDTLS   1
#define ALTCP_MBEDTLS_USE_SESSION_TICKETS 1
#define ALTCP_MBEDTLS_USE_SESSION_CACHE 1

#endif

