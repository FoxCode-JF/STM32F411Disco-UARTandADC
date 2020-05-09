#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "UARTComms.h"
#include "stm32f4xx_user_utils.h"

uint8_t buffer[MAX_CMD_SIZE];
uint8_t command[MAX_CMD_SIZE];
uint8_t DMA_TX_Buffer[DMA_TX_BUFFER_SIZE] = "Welcome to Fox UART communication test program\n";

volatile uint32_t commandIT = 0;
static uint32_t ms_count = 0; //  variable used by function delay_ms(uint32_t time)
volatile uint16_t txMessageCalcLen =0;

volatile uint16_t xCordJoyRead = 0;
volatile uint16_t yCordJoyRead = 0;

bool isXChannelRead = false, isYChannelRead = false;


int main(void)
{

	/*********Enable clock signals for peripherals*********/
 	RCC->APB2ENR = RCC_APB2ENR_SYSCFGEN;

	GPIO_Init(GPIOA, ENABLE);
	GPIO_Init(GPIOB, ENABLE);
	GPIO_Init(GPIOC, ENABLE);
	GPIO_Init(GPIOD, ENABLE);
	GPIO_Init(GPIOE, ENABLE);
	
	__DSB();
	GPIO_PinCfg(GPIOD, PD12, gpio_mode_output_PP_LS);
	GPIO_PinCfg(GPIOD, PD13, gpio_mode_output_PP_LS);
	GPIO_PinCfg(GPIOD, PD14, gpio_mode_output_PP_LS);
	GPIO_PinCfg(GPIOD, PD15, gpio_mode_output_PP_LS);

	GPIO_PinCfg(GPIOA, PA0, gpio_mode_in_floating);;

	SysTick_Config(16000);
				  
	uartDMASetup();	
	joystickADCSetup();	 

	/*************Configure Interrupts*************/

	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI0_PA | SYSCFG_EXTICR1_EXTI2_PE;
 	EXTI->FTSR = EXTI_FTSR_TR2;
 	EXTI->RTSR = EXTI_RTSR_TR0;
	EXTI->IMR = EXTI_IMR_MR0 | EXTI_IMR_MR2;

	DMA2->HIFCR			= DMA_HIFCR_CTCIF7 | 
						  DMA_HIFCR_CTCIF5;   //Clear Transfer Complete Interrupt flags

	delay_ms(100);

	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
	NVIC_EnableIRQ(USART1_IRQn); 
	NVIC_EnableIRQ(ADC_IRQn);
	
	NVIC_EnableIRQ(DMA2_Stream5_IRQn);
	NVIC_EnableIRQ(DMA2_Stream7_IRQn);

	ADC1->CR2 		|= ADC_CR2_SWSTART; 	// Start conversion

	while(1)
	{
		if (strncmp(command, "blink blue\r", 11) == 0)
		{

			for (int i = 0; i < 10; ++i)
			{
				BB(GPIOD->ODR, PD15) ^= 1;
				delay_ms(150);
				UART_sendMessage("I'M BLUE\n", 9);	
			}
			commandReset(command);
			
		}
		else if (strncmp(command, "blink yellow\r", 13) == 0)
		{
			for (int i = 0; i < 10; ++i)
			{
				BB(GPIOD->ODR, PD12) ^= 1;
				delay_ms(150);
				UART_sendMessage("I'M YELLOW\n", 11);	
			}
			commandReset(command);
		}
	}
	return 0;
}

void ADC_IRQHandler(void)
{

	if (ADC1->SR & ADC_SR_EOC)
	{
		ADC1->SR &= ~ADC_SR_EOC;
		xCordJoyRead = ADC1->DR;
		/*char* testStr = intToStr(xCordJoyRead);
		UART_sendMessage(testStr, 4);*/
		BB(GPIOD->ODR, PD15) = 1;
		/*if (!isXChannelRead)
		{
			xCordJoyRead = ADC1->DR;
			isXChannelRead = true;
			isYChannelRead = false;
		}
		else if(!isYChannelRead)
		{
			yCordJoyRead = ADC1->DR;
			isXChannelRead = false;
			isYChannelRead = true;
		}*/
	}
}
void USART1_IRQHandler(void)
{
	if(USART1->SR & USART_SR_IDLE)
	{
		uint32_t tmp;
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

		/*Save characters into command string*/
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
		
		commandReset(command);
		BB(GPIOD->ODR, PD14) ^= 1;
	}
}

void EXTI2_IRQHandler(void)
{

	if(EXTI->PR & EXTI_PR_PR2)
	{
		EXTI->PR = EXTI_PR_PR2;

		UART_sendMessage("Joystick pressed\n", 17);
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

	/* Send "RESET" message to terminal*/
	UART_sendMessage("RESET\n", 6);
}

void UART_sendMessage(const char* msg, uint8_t msgLen)
{
	while(!(USART1->SR & USART_SR_TC));
	DMA2_Stream7->NDTR = msgLen;
	memcpy(DMA_TX_Buffer, msg, msgLen);
	DMA2_Stream7->CR |=	DMA_SxCR_EN;
}

char* intToStr(uint16_t number)
{
	uint8_t i = 0;
	uint16_t tmp = number;
	char result[DECIMAL_DIGITS_NUM_12b];

	//result = (char*)malloc(DECIMAL_DIGITS_NUM_12b * sizeof(char));

	while(tmp)
	{
		result[i++] = tmp % 10 + '0';
		tmp /= 10;
	}
	return result;
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
	__DSB();

	//Configure pins for UART TxRx (alternate functions)
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

	DMA2_Stream5->PAR 		= (uint32_t)&USART1->DR;
	DMA2_Stream5->M0AR		= 	(uint32_t)buffer;
	DMA2_Stream5->NDTR		|= 	1;	
	DMA2_Stream5->CR 		|=	DMA_SxCR_EN;

	DMA2_Stream7->M0AR		= 	(uint32_t)DMA_TX_Buffer;
	DMA2_Stream7->NDTR		|= 	48;	
	DMA2_Stream7->CR 		|=	DMA_SxCR_EN;
}

void joystickADCSetup(void)
{
	/*Clock enable*/
	GPIO_Init(GPIOC, ENABLE);
	RCC->APB2ENR 	|= RCC_APB2ENR_ADC1EN;
	__DSB();

	/*Configure pins for user pushbutton and XY axes data*/
	GPIO_PinCfg(GPIOE, PE1, gpio_mode_in_PU);
	GPIO_PinCfg(GPIOC, PC1, gpio_mode_analog);
	GPIO_PinCfg(GPIOC, PC0, gpio_mode_analog);

	//ADC->CCR 		= ADC_CCR_ADCPRE;	// ADC Clk freq set to PLCK2/8
	ADC1->CR1 		= /*ADC_CR1_SCAN | */	// convert inputs set in ADC_SQRx
					  ADC_CR1_EOCIE;	// EOC interrupt enable
	ADC1->CR2 		= ADC_CR2_CONT ;/*|	// Continous measurement
					  ADC_CR2_EOCS;*/		// End Of Conversion bit set after each regular conversion
	//ADC1->SQR1 		|= ADC_SQR1_L_0;	// Use two channels
	ADC1->SQR3 		= 10 /*| 11 << 5*/;		// Scan channel 10 and 11
	ADC1->CR2 		|= ADC_CR2_ADON;	// A/D converter on
	delay_ms(1000);
}