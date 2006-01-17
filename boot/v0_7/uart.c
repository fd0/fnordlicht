
#include <avr/io.h>

/* most of the definitions borrowed from Peter Fleury's UART-Library 
   extende for control-register C and ATmega169 */
 
#if  defined(__AVR_ATmega8__)  || defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__) \
  || defined(__AVR_ATmega8515__) || defined(__AVR_ATmega8535__) \
  || defined(__AVR_ATmega323__) 
  /* ATMega with one USART */
 #define ATMEGA_USART
 #define UART0_UBRR_HIGH  UBRRH
 #define UART0_UBRR_LOW   UBRRL
 #define UART0_RECEIVE_INTERRUPT   SIG_UART_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_UART_DATA
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega162__)
 /* ATMega with two USART */
 #define ATMEGA_USART0
 #define ATMEGA_USART1
 #define UART0_UBRR_HIGH  UBRR0H
 #define UART0_UBRR_LOW   UBRR0L
 #define UART1_UBRR_HIGH  UBRR1H
 #define UART1_UBRR_LOW   UBRR1L
 #define UART0_RECEIVE_INTERRUPT   SIG_USART0_RECV
 #define UART1_RECEIVE_INTERRUPT   SIG_USART1_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_USART0_DATA
 #define UART1_TRANSMIT_INTERRUPT  SIG_USART1_DATA
 #define UART0_STATUS    UCSR0A
 #define UART0_CONTROL   UCSR0B
 #define UART0_CONTROL2  UCSR0C
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
 #define UART1_STATUS    UCSR1A
 #define UART1_CONTROL   UCSR1B
 #define UART1_CONTROL2  UCSR1C
 #define UART1_DATA     UDR1
 #define UART1_UDRIE    UDRIE1
#elif defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) 
 /* ATMega with two USART */
 #define ATMEGA_USART0
 #define ATMEGA_USART1
 #define UART0_UBRR_HIGH  UBRR0H
 #define UART0_UBRR_LOW   UBRR0L
 #define UART1_UBRR_HIGH  UBRR1H
 #define UART1_UBRR_LOW   UBRR1L
 #define UART0_RECEIVE_INTERRUPT   SIG_UART0_RECV
 #define UART1_RECEIVE_INTERRUPT   SIG_UART1_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_UART0_DATA
 #define UART1_TRANSMIT_INTERRUPT  SIG_UART1_DATA
 #define UART0_STATUS    UCSR0A
 #define UART0_CONTROL   UCSR0B
 #define UART0_CONTROL2  UCSR0C
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
 #define UART1_STATUS    UCSR1A
 #define UART1_CONTROL   UCSR1B
 #define UART1_CONTROL2  UCSR1C
 #define UART1_DATA     UDR1
 #define UART1_UDRIE    UDRIE1
#elif defined(__AVR_ATmega169__)
 #define ATMEGA_USART
 #define UART0_UBRR_HIGH  UBRR0H
 #define UART0_UBRR_LOW   UBRR0L
 // TODO #define UART0_RECEIVE_INTERRUPT   SIG_UART_RECV
 // TODO #define UART0_TRANSMIT_INTERRUPT  SIG_UART_DATA
 #define UART0_STATUS     UCSR0A
 #define UART0_CONTROL    UCSR0B
 #define UART0_CONTROL2   UCSR0C
 #define UART0_DATA       UDR0
 #define UART0_DOUBLEAVAIL 
 // TODO #define UART0_UDRIE     UDRIE
#else
 #error "Processor type not supported in uart.c !"
#endif

void USART_Init(unsigned int baudrate, unsigned char doublespeed)
{
    // Set baud rate
    UART0_UBRR_HIGH = (unsigned char)(baudrate>>8);
    UART0_UBRR_LOW  = (unsigned char)baudrate;

    // Enable 2x speed - TODO adopt to all uCs
	#ifdef UART0_DOUBLEAVAIL
    if (doublespeed) UCSR0A = (1<<U2X0);
	#endif

#if defined (ATMEGA_USART)
    /* Enable USART receiver and transmitter and disable interrupts */
    UART0_CONTROL = (1<<RXEN)|(1<<TXEN)|(0<<RXCIE)|(0<<UDRIE);
    /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
    #ifdef URSEL
    UCSRC = (1<<URSEL)|(3<<UCSZ0);
    #else
    UCSRC = (3<<UCSZ0);
    #endif 
    
#elif defined (ATMEGA_USART0 )
    /* Enable USART receiver and transmitter and disable interrupts */
    UART0_CONTROL = (1<<RXEN0)|(1<<TXEN0)|(0<<RXCIE0)|(0<<UDRIE0);
    /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
    #ifdef URSEL0
    UCSR0C = (1<<URSEL0)|(3<<UCSZ00);
    #else
    UCSR0C = (3<<UCSZ00);
    #endif 
#endif
    
}

void sendchar(char data)
{
    int i = 0;
    
    UART0_DATA  = data;
    
    if(SREG & 0x80)
    {
		while ( !(UART0_STATUS&0x40) && (i<10000) )
		{
			i++;
		}
    }
    else 
        while( !(UART0_STATUS&0x40) );
        
    UART0_STATUS=UART0_STATUS|0x40;                             //delete TXCflag
}


char recchar(void)
{
    int i = 0;
    
    if(SREG & 0x80)
    {
		// while (!(UART0_STATUS & (1<<RXC0)) && (i<10000))
		while (!(UART0_STATUS & 0x80) && (i<10000))
		{
			i++;
		}
    }
    else
        // while(!(UART0_STATUS & (1<<RXC0)));
		while(!(UART0_STATUS & 0x80));
        
    return UART0_DATA ;
}
