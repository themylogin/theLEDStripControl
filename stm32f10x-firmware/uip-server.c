/**
 * \addtogroup uip-server
 * @{
 */

/**
 * \file
 *         An example of how to write uIP applications
 *         with protosockets.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/*
 * This is a short example of how to write uIP applications using
 * protosockets.
 */

/*
 * We define the application state (struct hello_world_state) in the
 * hello-world.h file, so we need to include it here. We also include
 * uip.h (since this cannot be included in hello-world.h) and
 * <string.h>, since we use the memcpy() function in the code.
 */
#include "uip-server.h"
#include "uip.h"

#include "stm32f10x_tim.h"

#include <string.h>

/*---------------------------------------------------------------------------*/
/*
 * In uip-server we have defined the UIP_APPCALL macro to
 * server_handle_connection so that this funcion is uIP's application
 * function. This function is called whenever an uIP event occurs
 * (e.g. when a new connection is established, new data arrives, sent
 * data is acknowledged, data needs to be retransmitted, etc.).
 */
void server_handle_connection()
{
	/*
	* The uip_conn structure has a field called "appstate" that holds
	* the application state of the connection. We make a pointer to
	* this to access it easier.
	*/
	uip_tcp_appstate_t* s = &(uip_conn->appstate);

	/*
	* If a new connection was just established, we should initialize
	* the protosocket in our applications' state structure.
	*/
	if (uip_connected())
	{
		PSOCK_INIT(&s->p, s->pwmRgb, sizeof(s->pwmRgb));
	}

	/*
	* Finally, we run the protosocket function that actually handles
	* the communication. We pass it a pointer to the application state
	* of the current connection.
	*/
	PSOCK_BEGIN(&s->p);

	while (1)
	{
		PSOCK_READBUF(&s->p);

		TIM4->CCR2 = s->pwmRgb[0];
		TIM4->CCR3 = s->pwmRgb[1];
		TIM4->CCR4 = s->pwmRgb[2];
	}

	PSOCK_CLOSE(&s->p);
	PSOCK_END(&s->p);
}
/*---------------------------------------------------------------------------*/
