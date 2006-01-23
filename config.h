/* define the cpu speed when using the fuse bits set by the
 * makefile (and documented in fuses.txt), using an external
 * crystal oscillator at 16MHz */


#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* debug defines */
#ifndef DEBUG
#define DEBUG 0
#endif

#define UART_FIFO_SIZE 32
#define UART_BAUDRATE 19200
