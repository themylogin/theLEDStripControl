/**
 * \addtogroup apps
 * @{
 */

/**
 * \defgroup helloworld Hello, world
 * @{
 *
 * A small example showing how to write applications with
 * \ref psock "protosockets".
 */

/**
 * \file
 *         Header file for an example of how to write uIP applications
 *         with protosockets.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#pragma once

#include "psock.h"

typedef struct {
	struct psock p;
	uint16_t pwmRgb[3];
} uip_tcp_appstate_t;

void server_handle_connection(void);
#ifndef UIP_APPCALL
	#define UIP_APPCALL server_handle_connection
#endif /* UIP_APPCALL */

/** @} */
/** @} */
