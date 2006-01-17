#ifndef UART_H
#define UART_H

#define UART_BAUD_SELECT(baudRate,xtalCpu) ((xtalCpu)/((baudRate)*16L)-1)

#define UARTDOUBLE    1
#define UARTSINGLE    0

void USART_Init(unsigned int, unsigned char);
void sendchar(char);
char recchar(void);

#endif
