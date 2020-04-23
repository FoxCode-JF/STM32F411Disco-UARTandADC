#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "UARTComms.h"
#include "stm32f4xx_user_utils.h"


//extern volatile uint32_t ms_count;

void delay_ms(uint32_t time);


int main(void)
{

	/*********Enable clock signals for peripherals*********/

	RCC->AHB1ENR 	|= RCC_AHB1ENR_DMA2EN;
	RCC->APB2ENR 	|= RCC_APB2ENR_USART1EN;

	GPIO_Init(GPIOA, ENABLE);
	GPIO_Init(GPIOD, ENABLE);
	__DSB();

	GPIO_PinCfg(GPIOD, PD12, gpio_mode_output_PP_LS);
	GPIO_PinCfg(GPIOD, PD13, gpio_mode_output_PP_LS);
	GPIO_PinCfg(GPIOD, PD14, gpio_mode_output_PP_LS);
	GPIO_PinCfg(GPIOD, PD15, gpio_mode_output_PP_LS);

	GPIO_PinCfg(GPIOA, PA9, gpio_mode_AF9_PP_LS);
	GPIO_PinCfg(GPIOA, PA10, gpio_mode_AF10_PP_LS);

	GPIOA->AFR[1] 	= AF_USART1_RX_PA10 |
					  AF_USART1_TX_PA9;

	SysTick_Config(16000);
				  
						  

	/******************Configure UART******************/

	USART1->CR1			= USART_CR1_TXEIE 		| 
						  USART_MODE_RX_TX 		|
						  USART_WORDLENGTH_8B 	|
						  USART_PARITY_NO;

	USART1->CR2			= USART_STOPBITS_1;
	USART1->CR3 		= USART_FLOWCONTROL_NONE;

	/*Baud rate*/
	/*
		BAUD_RATE = f_ck/(8*(2 - OVER8)*USARTDIV)

		For f_ck = 16MHz:
		USARTDIV = (16M/(8*2)*9600) ~= 104.17 
		Converted to hex:
		104 = 0x68
		0,17 ~= 1/16 + 1/8 ~= 0,1875 = 0x0.3
		BRR register value to write is 0x683	 
	 */

	USART1->BRR 	= 0x683;

	/*Enable Direct Memory Access*/
	USART1->CR3 	= USART_CR3_DMAT | USART_CR3_DMAR;


	/*************Configure DMA Controller*************/

	/*Transmission stream config (USART1_TX)
	 *
	 *	DMA1, Channel 4, Stream 7
	 */
	
	DMA2_Stream7->CR 	= DMA_SxCR_CHSEL_2 	| 	// select channel 4
						  DMA_SxCR_PL_1		| 	// Priority level medium
						  DMA_SxCR_MINC 	| 	// Memory increment mode
						  DMA_SxCR_TCIE 	; 	// Transfer Complete Interrupt Enable

	DMA2_Stream7->PAR 	= (uint32_t)&USART1->DR;

	/*Receiving stream config (USART1_RX)*/
	/*
	 *	DMA1, Channel 4, Stream 5
	 */
						  
	DMA2_Stream5->CR 	= DMA_SxCR_CHSEL_2 	|	// select channel 4
						  DMA_SxCR_PL_1		| 	// Priority level medium
						  DMA_SxCR_MINC 	| 	// Memory increment mode
						  DMA_SxCR_TCIE 	; 	// Transfer Complete Interrupt Enable

	DMA2_Stream5->PAR 	= (uint32_t)&USART1->DR;



	/*Enable transmission*/


	/*************Configure Interrupts*************/

	DMA2->HIFCR			= DMA_HIFCR_CTCIF7 	|
						  DMA_HIFCR_CTCIF5;
	delay_ms(100);

	//NVIC_EnableIRQ(USART1_IRQn);
	NVIC_EnableIRQ(DMA2_Stream5_IRQn);
	//NVIC_EnableIRQ(DMA2_Stream7_IRQn);


	USART1->CR1 	|= USART_ENABLE;

	char buffer[100] = "";
	char c;

	Tx_Rx_Init(buffer, RX_INIT);
	Tx_Rx_Init(buffer, TX_INIT);

	while(1)
	{
		if (strncmp(buffer, "blink led", 9) == 0)
		{

			for (int i = 0; i < 10; ++i)
			{
				BB(GPIOD->ODR, PD15) ^= 1;
				BB(GPIOD->ODR, PD12) ^= 1;
				BB(GPIOD->ODR, PD13) ^= 1;
				BB(GPIOD->ODR, PD14) ^= 1;
				delay_ms(150);
			}
			memset(buffer,0,strlen(buffer));
		}


	}
	return 0;
}

void DMA2_Stream6_IRQHandler(void)
{
	if(DMA2->HISR & DMA_HISR_TCIF7)
	{
		/*termination of TX service begin*/

		/*termination of TX service end*/
		DMA2->HIFCR = DMA_HIFCR_CTCIF7;
		BB(GPIOD->ODR, PD14) ^= 1;

		if((DMA2_Stream7->CR & DMA_SxCR_EN) == 0 && (DMA2->HISR & DMA_HISR_TCIF7))
		{
			DMA2_Stream7->CR |= DMA_SxCR_EN;
		}
	}
}

void DMA2_Stream5_IRQHandler(void)
{
	if(DMA2->HISR & DMA_HISR_TCIF5)
	{
		/*termination of RX service code begin*/

		/*termination of RX service code end*/
		DMA2->HIFCR = DMA_HIFCR_CTCIF5;

		BB(GPIOD->ODR, PD13) = 1;

		if((DMA2_Stream5->CR & DMA_SxCR_EN) == 0 && (DMA2->HISR & DMA_HISR_TCIF5))
		{
			DMA2_Stream5->CR |= DMA_SxCR_EN;
		}

	}
}
void USART1_IRQHandler(void)
{
	if(USART1->SR & USART_SR_RXNE)
	{
		USART1->SR &= ~USART_SR_RXNE;

		delay_ms(1000);
		BB(GPIOD->ODR, PD14) ^= 1;
		
	}

}

void SysTick_Handler(void)
{
	ms_count++;
/*	if(ms_count > 500)
	{
		BB(GPIOD->ODR, PD15) ^= 1;
		ms_count = 0;
	}*/
}


void Tx_Rx_Init(char* buff, bool txrx)
{
	uint16_t buffLen = strlen(buff);
	if(txrx == 1)
	{
		DMA2_Stream7->M0AR		= 	(uint32_t)buff;
		DMA2_Stream7->NDTR		|= 	buffLen;	
		DMA2_Stream7->CR 		|=	DMA_SxCR_EN;	 
	}
	else if(txrx == 0)
	{
		DMA2_Stream5->M0AR		= 	(uint32_t)buff;
		DMA2_Stream5->NDTR		|= 	9;	
		DMA2_Stream5->CR 		|=	DMA_SxCR_EN;
	}
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
