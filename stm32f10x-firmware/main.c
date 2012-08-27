/**
  ******************************************************************************
  * @file TIM/PWM_Output/main.c
  * @author  MCD Application Team
  * @version V3.1.2
  * @date    09/28/2009
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

#include "uip.h"
#include "uip_arp.h"
#include "enc28j60.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup TIM_PWM_Output
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
ErrorStatus HSEStartUpStatus;

/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration(void);
void GPIO_Configuration(void);
void TIM_Configuration(void);
void Network_Configuration(void);

void Network_Process();

/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	/* System Clocks Configuration */
	RCC_Configuration();

	/* GPIO Configuration */
	GPIO_Configuration();

	/* Timer Configuration */
	TIM_Configuration();

	/* Network Configuration */
	Network_Configuration();

	while (1)
	{
		Network_Process();
	}
}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void RCC_Configuration(void)
{
	/* Setup the microcontroller system. Initialize the Embedded Flash Interface,
	 initialize the PLL and update the SystemFrequency variable. */
	SystemInit();

	/* PCLK1 = HCLK/4 */
	RCC_PCLK1Config(RCC_HCLK_Div4);

	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	/* GPIOA and GPIOB clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
						   RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

}

/**
  * @brief  Configure the TIM4 Output Channels.
  * @param  None
  * @retval None
  */
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* GPIOA Configuration: TIM4 Channel2, 3 and 4 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief   Configure TIM4
 * @param   None
 * @retval  None
 */
void TIM_Configuration(void)
{
	/* -----------------------------------------------------------------------
	TIM4 Configuration: generate 4 PWM signals with 4 different duty cycles:
	TIM4CLK = 72 MHz, Prescaler = 18, TIM4 counter clock = 4 MHz
	TIM4 ARR Register = 3999 => TIM4 Frequency = TIM4 counter clock/(ARR + 1)
	TIM4 Frequency = 1 KHz.
	----------------------------------------------------------------------- */

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 3999;
	TIM_TimeBaseStructure.TIM_Prescaler = 18;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 4000;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM4, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

	/* PWM1 Mode configuration: Channel2 */
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 4000;

	TIM_OC2Init(TIM4, &TIM_OCInitStructure);

	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

	/* PWM1 Mode configuration: Channel3 */
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 4000;

	TIM_OC3Init(TIM4, &TIM_OCInitStructure);

	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);

	/* PWM1 Mode configuration: Channel4 */
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 4000;

	TIM_OC4Init(TIM4, &TIM_OCInitStructure);

	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM4, ENABLE);

	/* TIM4 enable counter */
	TIM_Cmd(TIM4, ENABLE);
}

/**
 * @brief   Configure Network
 * @param   None
 * @retval  None
 */
void Network_Configuration(void)
{
	struct uip_eth_addr mac = {{ 0xeb, 0xa0, 0x00, 0x00, 0x00, 0x00 }};
	enc28j60_init(mac.addr);

	uip_init();
	uip_arp_init();
	uip_setethaddr(mac);

	uip_ipaddr_t ipaddr;
	uip_ipaddr(ipaddr, 192, 168, 0, 100);
	uip_sethostaddr(ipaddr);
	uip_ipaddr(ipaddr, 255, 255, 255, 0);
	uip_setnetmask(ipaddr);
	uip_ipaddr(ipaddr, 192, 168, 0, 1);
	uip_setdraddr(ipaddr);

	uip_listen(HTONS(4000));
}

/**
 * @brief   Process Network
 * @param   None
 * @retval  None
 */
void Network_Process(void)
{
	static uint8_t arp_counter = 0;

	int i;
	for (i = 0; i < UIP_CONNS; i++)
	{
		uip_periodic(i);
		if (uip_len > 0)
		{
			uip_arp_out();
			enc28j60_send_packet((uint8_t *)uip_buf, uip_len);
		}
	}

	#if UIP_UDP
		for (i = 0; i < UIP_UDP_CONNS; i++)
		{
			uip_udp_periodic(i);
			if (uip_len > 0)
			{
				uip_arp_out();
				network_send();
			}
		}
	#endif /* UIP_UDP */

	if (++arp_counter >= 50)
	{
		arp_counter = 0;
		uip_arp_timer();
	}

	uip_len = enc28j60_recv_packet((uint8_t *)uip_buf, UIP_BUFSIZE);
	if (uip_len > 0)
	{
		if (((struct uip_eth_hdr *)&uip_buf[0])->type == htons(UIP_ETHTYPE_IP))
		{
			uip_arp_ipin();
			uip_input();
			if (uip_len > 0)
			{
				uip_arp_out();
				enc28j60_send_packet((uint8_t *)uip_buf, uip_len);
			}
		}
		else if (((struct uip_eth_hdr *)&uip_buf[0])->type == htons(UIP_ETHTYPE_ARP))
		{
			uip_arp_arpin();
			if (uip_len > 0)
			{
				enc28j60_send_packet((uint8_t *)uip_buf, uip_len);
			}
		}
	}
}

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
