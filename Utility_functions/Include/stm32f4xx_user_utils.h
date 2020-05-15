/**
 * @defgroup   STM32F4XX_USER_UTILS stm32f4xx user utilities
 *
 * @brief      This file implements stm 32f4xx user utilities.
 *
 * @author     Fox
 * @date       2020.02
 */


#ifndef STM32F4XX_USER_UTILS_H_
#define STM32F4XX_USER_UTILS_H_

#include "stm32f4xx.h"
#include "stm32f411_gpio_drivers.h"
#include "stddef.h"

/*
	MACROS BEGIN
*/

/******************bit banding begin*******************
 *
 *
 *	Definition of regions and aliases boundaries
 */

#define SRAM_BB_REGION_START  	0x20000000
#define SRAM_BB_REGION_END		0x200fffff
#define SRAM_BB_ALIAS			0x22000000

#define PERIPH_BB_REGION_START	0x40000000
#define PERIPH_BB_REGION_END 	0x400fffff
#define PERIPH_BB_ALIAS 		0x42000000

/*
 *	Boundaries check for SRAM and PERIPHERALS
 */

#define SRAM_ADR_COND(adr)	    ( (uint32_t)&adr >= SRAM_BB_REGION_START \
								&& (uint32_t)&adr <= SRAM_BB_REGION_END )

#define PERIPH_ADR_COND(adr)	( (uint32_t)&adr >= PERIPH_BB_REGION_START\
								&& (uint32_t)&adr <= PERIPH_BB_REGION_END )

/*
 *	 Calculate bit band address using formula (bit_word_addr = bit_band_base + (byte_offset x 32) + (bit_number x 4))
 */

#define BB_SRAM2(adr, bit)		(SRAM_BB_ALIAS + ((uint32_t)&adr - SRAM_BB_REGION_START)* 32u\
								+ (uint32_t)(bit * 4u))

#define BB_PERIPH(adr, bit)		(PERIPH_BB_ALIAS + ((uint32_t)&adr - PERIPH_BB_REGION_START)* 32u\
								+ (uint32_t)(__builtin_ctz(bit)) * 4u)

/*bit - bit mask, not bit position!*/

#define BB(adr, bit) 			*(__IO uint32_t*)(SRAM_ADR_COND(adr) ? BB_SRAM2(adr, bit) : \
								(PERIPH_ADR_COND(adr)) ? BB_PERIPH(adr, bit) : 0 )

#define BB_SRAM(adr, bit)		*(__IO uint32_t*)BB_SRAM2(adr, bit)

/******************bit banding end*******************/


/******************Utility functions prototypes*******************/
void delay_ms(uint32_t time);
void intToStr(uint16_t number, char* container, size_t strSize);


#endif	/* STM32F4XX_USER_UTILS_H_ */


