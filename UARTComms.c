#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "UARTComms.h"
#include "stm32f4xx_user_utils.h"

#define MAX_CMD_SIZE 	50

//extern volatile uint32_t ms_count;
char buffer[MAX_CMD_SIZE];
char command[MAX_CMD_SIZE];

volatile uint32_t commandIT = 0;
static volatile uint32_t ms_count = 0; //  variable used by function delay_ms(uint32_t time)

void delay_ms(uint32_t time);
void usartSetup(void);
void commandReset(char* cmd);

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


	GPIO_PinCfg(GPIOA, PA0, gpio_mode_in_floating);
	GPIO_PinCfg(GPIOA, PA9, gpio_mode_AF9_PP_LS);
	GPIO_PinCfg(GPIOA, PA10, gpio_mode_AF10_PP_LS);

	GPIOA->AFR[1] 	= AF_USART1_RX_PA10 |
					  AF_USART1_TX_PA9;

	SysTick_Config(16000);
				  
	usartSetup();		 




	/*************Configure Interrupts*************/

	EXTI->IMR 			= EXTI_IMR_MR0;
	EXTI->FTSR 			= EXTI_FTSR_TR0;

	DMA2->HIFCR			= DMA_HIFCR_CTCIF7 | DMA_HIFCR_CTCIF5;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;
	
	delay_ms(100);

	NVIC_EnableIRQ(EXTI0_IRQn);
	//NVIC_EnableIRQ(USART1_IRQn);
	NVIC_EnableIRQ(DMA2_Stream5_IRQn);
	//NVIC_EnableIRQ(DMA2_Stream7_IRQn);




	//memset(buffer,'\0',strlen(buffer));

	while(1)
	{
		if (strncmp(command, "blink blue\r", 11) == 0)
		{

			for (int i = 0; i < 10; ++i)
			{
				BB(GPIOD->ODR, PD15) ^= 1;
				delay_ms(150);
			}
			commandReset(command);
			
		}
		else if (strncmp(command, "blink yellow\r", 13) == 0)
		{
			for (int i = 0; i < 10; ++i)
			{
				BB(GPIOD->ODR, PD12) ^= 1;
				delay_ms(150);
			}
			commandReset(command);	
		}
	}
	return 0;
}

void DMA2_Stream5_IRQHandler(void)
{
	if(DMA2->HISR & DMA_HISR_TCIF5)
	{
		/*termination of RX service code begin*/

		/*termination of RX service code end*/
		DMA2->HIFCR = DMA_HIFCR_CTCIF5;

		BB(GPIOD->ODR, PD13) ^= 1;


		while(!(USART1->SR & USART_SR_TC));

		memcpy(&command[commandIT++], buffer, 1);
		
		if((DMA2_Stream5->CR & DMA_SxCR_EN) == 0 && (DMA2->HISR & DMA_HISR_TCIF5) == 0)
		{
			DMA2_Stream5->CR |= DMA_SxCR_EN;
		}
	}
}

void DMA2_Stream7_IRQHandler

void EXTI0_IRQHandler(void)
{
	if(EXTI->PR & EXTI_PR_PR0)
	{
		EXTI->PR = EXTI_PR_PR0;
		commandReset(command);
		BB(GPIOD->ODR, PD14) ^= 1;
	}
}

void SysTick_Handler(void)
{
	ms_count++;
}


void commandReset(char* cmd)
{
	uint8_t cmdLen = strlen(cmd);
	if(cmdLen > MAX_CMD_SIZE) cmdLen = 50;
	memset(command, '\0', cmdLen);
	commandIT = 0;
}

void Tx_Rx_Init(char* buff, bool txrx)
{
	uint16_t buffLen = strlen(buff);
	if(txrx == 1)
	{
		DMA2_Stream7->M0AR		= 	(uint32_t)&buff;
		DMA2_Stream7->NDTR		|= 	buffLen;	
		DMA2_Stream7->CR 		|=	DMA_SxCR_EN;	 
	}
	else if(txrx == 0)
	{
		DMA2_Stream5->M0AR		= 	(uint32_t)&buff;
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

void usartSetup(void)
{
		/******************Configure UART******************/

	USART1->CR1			= USART_MODE_RX_TX 		|
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
	USART1->CR3 	= /*USART_CR3_DMAT |*/ USART_CR3_DMAR;

	USART1->CR1 	|= USART_ENABLE;

		/*************Configure DMA Controller*************/

	/*Transmission stream config (USART1_TX)
	 *
	 *	DMA1, Channel 4, Stream 7
	 */
	
/*	DMA2_Stream7->CR 	= DMA_SxCR_CHSEL_2 	| 	// select channel 4
						  DMA_SxCR_PL_1		| 	// Priority level medium
						  DMA_SxCR_MINC 	| 	// Memory increment mode
						  DMA_SxCR_TCIE 	; 	// Transfer Complete Interrupt Enable

	DMA2_Stream7->PAR 	= (uint32_t)&USART1->DR;*/

	/*Receiving stream config (USART1_RX)*/
	/*
	 *	DMA1, Channel 4, Stream 5
	 */
						  
	DMA2_Stream5->CR 	= DMA_SxCR_CHSEL_2 	|	// select channel 4
						  DMA_SxCR_PL_1		| 	// Priority level medium
						  DMA_SxCR_TCIE 	; 	// Transfer Complete Interrupt Enable
						//  DMA_SxCR_MINC 	| 	// Memory increment mode


	DMA2_Stream5->PAR 	= (uint32_t)&USART1->DR;

	DMA2_Stream5->M0AR		= 	(uint32_t)buffer;
	DMA2_Stream5->NDTR		|= 	1;	
	DMA2_Stream5->CR 		|=	DMA_SxCR_EN;
}
