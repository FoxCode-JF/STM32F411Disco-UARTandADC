#include "stm32f4xx_user_utils.h"
#include "stm32f411_gpio_drivers.h"
#include "stm32f4xx_conf.h"

/**
 * @brief      Enables chosen port
 *
 * @param[in]  port  The port according to port definitions in stm32f4xx.h
 */
void GPIO_Init(GPIO_TypeDef * const restrict port, bool EnableDisable)
{

	if (EnableDisable == ENABLE)
	{
			
		if 		(port == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
		else if (port == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
		else if (port == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
		else if (port == GPIOD) RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
		else if (port == GPIOE) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
		else if (port == GPIOF) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
		else if (port == GPIOG) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
		else if (port == GPIOH) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
		else if (port == GPIOI) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;

	}else if (EnableDisable == DISABLE)
	{
		if 		(port == GPIOA) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN;
		else if (port == GPIOB) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;
		else if (port == GPIOC) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOCEN;
		else if (port == GPIOD) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIODEN;
		else if (port == GPIOE) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOEEN;
		else if (port == GPIOF) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOFEN;
		else if (port == GPIOG) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOGEN;
		else if (port == GPIOH) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOHEN;
		else if (port == GPIOI) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOIEN;
	}

}
/**
 * @brief      GPIO initialization
 * @details    Enables all GPIO ports
 */
void GPIO_InitAll(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN |
			RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN |
			RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIOGEN | RCC_AHB1ENR_GPIOHEN |
			RCC_AHB1ENR_GPIOIEN;			
}

/**
 * @brief      GPIO configuration
 *
 * @param      port  The port
 * @param[in]  pin   The pin
 * @param[in]  mode  The mode
 */

//in progress

void GPIO_PinCfg(GPIO_TypeDef * const __restrict__ port, GpioPin_t pin, GpioMode_t mode)
{
	if(mode & 0x100u) port->OTYPER |= pin;
	else port->OTYPER &= (uint32_t)~pin;

	pin = __builtin_ctz(pin)*2;

	uint32_t reset_mask = ~(0x03u << pin);
	uint32_t reg_val;

	reg_val 	= 	port->MODER;
	reg_val 	&=	reset_mask;
	reg_val 	|= (((mode & 0x600u) >> 9u) << pin);
	port->MODER = reg_val;

	reg_val 	= 	port->PUPDR;
	reg_val 	&=	reset_mask;
	reg_val 	|= (((mode & 0x30u) >> 4u) << pin);
	port->PUPDR = reg_val;

	reg_val 	= 	port->OSPEEDR;
	reg_val 	&=	reset_mask;
	reg_val 	|= (((mode & 0xC0u) >> 6u) << pin);
	port->OSPEEDR = reg_val;


/*Setting alternate function bits moved outside*/
	volatile uint32_t* reg_adr;
	reg_adr 	= &port->AFR[0];

	pin *= 2;

	if(pin > 28)
	{
		pin 	-=32;
		reg_adr = &port->AFR[1];
	}

	reg_val 	= *reg_adr;
	reg_val 	&= ~(0x0Fu << pin);
	reg_val 	|= (uint32_t)(mode & 0x0Ful) << pin;
	*reg_adr 	= reg_val;
}

/*void delay_ms(uint32_t time)
{	
	ms_count = 0;

	while(ms_count != time);

	if(ms_count == time)
	{
		ms_count = 0;
	}
}*/
/*

void blink_LED(GpioPin_t LED_Pin, GPIO_TypeDef * const __restrict__ port, uint32_t time_ms, uint16_t blink_count)
{
	for (int i = 0; i < blink_count; ++i)
	{
		BB(port->ODR, LED_Pin) = 1;
		delay_ms(time_ms);
		BB(port->ODR, LED_Pin) = 0;
		delay_ms(time_ms);
	}
}*/