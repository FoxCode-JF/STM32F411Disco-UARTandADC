#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "UARTComms.h"
#include "stm32f4xx_user_utils.h"

#define MAX_CMD_SIZE 		50
#define UART_BUFFER_SIZE 	200
#define DMA_TX_BUFFER_SIZE 	32
//extern volatile uint32_t ms_count;
uint8_t buffer[MAX_CMD_SIZE];
uint8_t command[MAX_CMD_SIZE];
uint8_t DMA_TX_Buffer[DMA_TX_BUFFER_SIZE] = "transfer success\r\n";

volatile uint32_t commandIT = 0;
static volatile uint32_t ms_count = 0; //  variable used by function delay_ms(uint32_t time)
volatile uint16_t txMessageCalcLen =0;

void delay_ms(uint32_t time);
void uartDMASetup(void);
void commandReset(char* cmd);
uint16_t USART_DMA_Sending(void);
uint8_t USART_DMA_Send(USART_TypeDef* USARTx, uint8_t* DataString, uint16_t strlen);

int main(void)
{

	/*********Enable clock signals for peripherals*********/

	GPIO_Init(GPIOA, ENABLE);
	GPIO_Init(GPIOD, ENABLE);

	__DSB();

	GPIO_PinCfg(GPIOD, PD12, gpio_mode_output_PP_LS);
	GPIO_PinCfg(GPIOD, PD13, gpio_mode_output_PP_LS);
	GPIO_PinCfg(GPIOD, PD14, gpio_mode_output_PP_LS);
	GPIO_PinCfg(GPIOD, PD15, gpio_mode_output_PP_LS);


	GPIO_PinCfg(GPIOA, PA0, gpio_mode_in_floating);
/*	GPIO_PinCfg(GPIOA, PA9, gpio_mode_AF7_PP_LS);
	GPIO_PinCfg(GPIOA, PA10, gpio_mode_AF7_PP_LS);*/


/*	GPIOA->AFR[1] 	= AF_USART1_RX_PA10 |
					  AF_USART1_TX_PA9;*/

	SysTick_Config(16000);
				  
	uartDMASetup();		 

	/*************Configure Interrupts*************/

	EXTI->IMR 			= EXTI_IMR_MR0;
	EXTI->FTSR 			= EXTI_FTSR_TR0;

	DMA2->HIFCR			= DMA_HIFCR_CTCIF7 | 
						  DMA_HIFCR_CTCIF5;   //Clear Transfer Complete Interrupt flags
	SYSCFG->EXTICR[0] 	|= SYSCFG_EXTICR1_EXTI0_PA;
	
	delay_ms(100);

	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_EnableIRQ(DMA2_Stream5_IRQn);
	NVIC_EnableIRQ(DMA2_Stream7_IRQn);

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

void USART1_IRQHandler(void)
{
	if(USART1->SR & USART_SR_IDLE)
	{
		volatile uint32_t tmp;
		tmp = USART1->SR;
		tmp = USART1->DR;
		DMA2_Stream5->CR &= ~DMA_SxCR_EN;
	}
}
void DMA2_Stream5_IRQHandler(void)
{
	if(DMA2->HISR & DMA_HISR_TCIF5)
	{
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

void DMA2_Stream7_IRQHandler(void)
{
	if(DMA2->HISR & DMA_HISR_TCIF7)
	{
		DMA2->HIFCR |= DMA_HIFCR_CTCIF7; // Clear transfer complete flag

		while(!(USART1->SR & USART_SR_TC));
		
		if((DMA2_Stream7->CR & DMA_SxCR_EN) == 0 \
			&& (DMA2->HISR & DMA_HISR_TCIF7) == 0 \
			&& DMA2_Stream7->NDTR != 0)
		{
			DMA2_Stream7->CR |= DMA_SxCR_EN;
		}
	}
}

void EXTI0_IRQHandler(void)
{
	if(EXTI->PR & EXTI_PR_PR0)
	{
		EXTI->PR = EXTI_PR_PR0;
		DMA2_Stream7->CR |= DMA_SxCR_EN;
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

	/* Send message to terminal */
	memset(command, '\0', cmdLen);
	memcpy(DMA_TX_Buffer, "RESET\n", 6);
	DMA2_Stream7->CR |=	DMA_SxCR_EN;
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

void uartDMASetup(void)
{
	/******************Configure Clocks******************/
	RCC->AHB1ENR 	|= RCC_AHB1ENR_DMA2EN;
	RCC->APB2ENR 	|= RCC_APB2ENR_USART1EN;
	GPIO_Init(GPIOB, ENABLE);

	//Configure pins for UART TxRx
	GPIO_PinCfg(GPIOB, PB6, gpio_mode_AF7_PP_LS);
	GPIO_PinCfg(GPIOB, PB7, gpio_mode_AF7_PP_LS);

	/******************Configure UART******************/

	USART1->CR1			= USART_MODE_RX_TX 		|
						  USART_WORDLENGTH_8B 	|
						  USART_CR1_IDLEIE		| //enable idle interrupt
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
	USART1->CR1 	|= USART_ENABLE;

		/*************Configure DMA Controller*************/

	/*Transmission stream config (USART1_TX)
	 *
	 *	DMA2, Channel 4, Stream 7
	 */
	
	DMA2_Stream7->CR 	= DMA_SxCR_CHSEL_2 	| 	// select channel 4
						  DMA_SxCR_PL_1		| 	// Priority level medium
						  DMA_SxCR_MINC 	| 	// Memory increment mode
						  DMA_SxCR_DIR_0 	|
						  DMA_SxCR_TCIE 	; 	// Transfer Complete Interrupt Enable

	DMA2_Stream7->PAR 	= (uint32_t)&USART1->DR;

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

	uint32_t buffSize 		= strlen(DMA_TX_Buffer);
	DMA2_Stream7->M0AR		= 	(uint32_t)DMA_TX_Buffer;
	DMA2_Stream7->NDTR		|= 	buffSize;	
	DMA2_Stream7->CR 		|=	DMA_SxCR_EN;
}


uint16_t USART_DMA_Sending(void) 
{
    /* DMA works */
    if (DMA2_Stream7->NDTR) {
        return 1;
    }
    return !((USART1)->SR & USART_SR_TXE);
}
