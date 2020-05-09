#define DECIMAL_DIGITS_NUM_12b 4 //ADC register can have maximum of 4 digits 2^13 - 1 = 8191

#define MAX_CMD_SIZE 		50
#define UART_BUFFER_SIZE 	200
#define DMA_TX_BUFFER_SIZE 	32

#define RX_INIT 			true
#define TX_INIT 			false

#define AF_USART1_TX_PA9	0x070u
#define AF_USART1_RX_PA10	0x700u 
#define AF_USART2_TX_PA2	0x0700u
#define AF_USART2_RX_PA3	0x7000u 

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

void joystickADCSetup(void);
void uartDMASetup(void);

void delay_ms(uint32_t time);
void commandReset(char* cmd);
void UART_sendMessage(const char* msg, uint8_t msgLen);
char* intToStr(uint16_t number);
