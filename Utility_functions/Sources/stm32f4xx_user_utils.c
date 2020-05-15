#include "stm32f4xx_user_utils.h"

static uint32_t ms_count;

void delay_ms(uint32_t time)
{	
	ms_count = 0;

	while(ms_count != time);

	if(ms_count == time)
	{
		ms_count = 0;
	}
}

void intToStr(uint16_t number, char* container, size_t strSize)
{
	int  tmp = number;

	for (int i = strSize - 1; i < strSize; --i)
	{
		container[i] = tmp % 10 + '0';
		tmp /= 10;
	}
}