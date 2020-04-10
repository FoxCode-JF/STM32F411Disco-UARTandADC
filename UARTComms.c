#include "stm32f4xx_user_utils.h"
#include "stm32f4xx.h"

#define AF_USART1_TX 		0x07u
#define AF_USART1_RX 		0x70u 
#define USART_MODE_RX_TX 	(USART_CR1_RE | USART_CR1_TE)
#define USART_ENABLE		USART_CR1_UE	

#define USART_WORDLENGTH_8B 0X0000
#define USART_WORDLENGTH_9B USART_CR1_M

#define USART_PARITY_NO		0X0000
#define USART_PARITY_EVEN	USART_CR1_PCE
#define USART_PARITY_ODD	USART_CR1_PCO

#define USART_STOPBITS_1	0X0000
#define USART_STOPBITS_0_5	0X1000
#define USART_STOPBITS_2	0X2000
#define USART_STOPBITS_1_5	0X3000

#define USART_FLOWCONTROL_NONE	0X0000
#define USART_FLOWCONTROL_RTS	USART_CR3_RTSE
#define USART_FLOWCONTROL_CTS	USART_CR3_CTSE


static volatile uint32_t ms_count;

void delay_ms(uint32_t);

int main(void)
{

	/*Enable clock signals for peripherals*/
	RCC->APB2ENR 	|= RCC_APB2ENR_USART1EN;

	gpio_init(GPIOA);
	gpio_init(GPIOD);
	__DSB();

	gpio_pin_cfg(GPIOD, PD12, gpio_mode_output_PP_LS);
	gpio_pin_cfg(GPIOD, PD15, gpio_mode_output_PP_LS);

	GPIOA->AFR[1] 	= AF_USART1_RX |
					  AF_USART1_TX;
	SysTick_Config(1600);

	/*Configure UART*/
	USART1->CR1		= /*USART_CR1_TXEIE |*/ 
					  USART_MODE_RX_TX |
					  USART_WORDLENGTH_8B |
					  USART_PARITY_NO;

	USART1->CR2		= USART_STOPBITS_1;

	USART1->CR3 	= USART_FLOWCONTROL_NONE;

	/*Enable transmission*/
	USART1->CR1 	|= USART_ENABLE;

	/*Read/Write data register */
	

	char c;

	//NVIC_EnableIRQ(USART1_IRQn);

	while(1)
	{
		if(USART1->SR & USART_SR_RXNE)
		{
			
			c = USART1->DR;
		}
		if(c == 'T' || c == 't')
		{
			BB(GPIOD->ODR, PD12) ^=1;
			if(USART1->SR & USART_SR_TXE)
			{
				c = 'R';
				USART1->DR 		= c;	
			}
		}
	}
}

/*void USART1_IRQHandler(void)
{
	BB(GPIOD->ODR, PD15) = 1;
	delay_ms(500);
	BB(GPIOD->ODR, PD15) = 0;
}*/

void SysTick_Handler(void)
{
	ms_count++;
}

void delay_ms(uint32_t time)
{
	ms_count = 0;

	while(ms_count != time);

	if(ms_count == time)
	{
		ms_count = 0;
	}
}