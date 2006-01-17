/*****************************************************************************
*
* AVRPROG compatible boot-loader
* Version  : 0.7 (Feb. 2005)
* Compiler : avr-gcc 3.4.1 / avr-libc 1.0.2
* size     : depends on features and startup ( minmal features < 512 words)
* by       : Martin Thomas, Kaiserslautern, Germany
*            eversmith@heizung-thomas.de
*
* License  : Copyright (c) 2005 Martin Thomas
*            Free to use. You have to mention the copyright
*            owners in source-code and documentation of derived
*            work. No warranty.
*
* Tested with ATmega8, ATmega16, ATmega32, ATmega128
*
* - based on the Butterfly Bootloader-Code
*   Copyright (C) 1996-1998 Atmel Corporation
*   Author(s)     : BBrandal, PKastnes, ARodland, LHM
*   The orignal code has been made available by ATMEL together with the
*   Butterfly application code. Since ATMEL.NO had no problem with
*   the application gcc-port they hopefully will not have any concerns about
*   publishing this port. A lot of things have been change but the ATMEL
*   "skeleton" is still in this code. Make sure to keep the copyright notice
*   in derived work to avoid trouble.
*
* - based on boot.h from the avr-libc (c) Eric Weddington
*
****************************************************************************
*
*  The boot interrupt vector is included (this bootloader is completly in
*  ".text" section). If you need this space for further functions you have to
*  add a separate section for the bootloader-functions and add an attribute
*  for this section to _all_ function prototypes of functions in the loader.
*  With this the interrupt vector will be placed at .0000 and the bootloader
*  code (without interrupt vector) at the adress you define in the linker
*  options for the newly created section. See the avr-libc FAQ, the avr-
*  libc's avr/boot.h documentation and the makefile for further details.
*
*  See the makefile for information how to adopt the linker-settings to
*  the selected Boot Size (_Bxxx below)
*
*  With BOOT_SIMPLE this bootloader has 0x3DE bytes size and should fit
*  into a 512word bootloader-section.
*
*  Set AVR clock-frequency and the baudrate below, set MCU-type in
*  makefile.
*
****************************************************************************/
/*
	Does not work reliably so far:
	- lock bits set
*/

// programmers-notepad tabsize 4
#define VERSION_HIGH '0'
#define VERSION_LOW  '7'

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
// function not found in boot.h to read lock/fuses
#include "lowlevel.h"

/* Pin "BLPNUM" on port "BLPORT" in this port has to grounded
   (active low) to start the bootloader */
#define BLPORT  PORTC
#define BLDDR   DDRC
#define BLPIN   PINC
#define BLPNUM  PINC0

/*
   Select startup-mode
   * SIMPLE-Mode - Jump to bootloader main BL-loop if key is
     pressed (Pin grounded) "during" reset or jump to the
	 application if the pin is not grounded (=pulled up by
	 internal pull-up-resistor)
   * POWERSAVE-Mode - Startup is separated in two loops
     which makes power-saving a little easier if no firmware
	 is on the chip. Needs more memory
   * BOOTICE-Mode - to flash the JTAGICE upgrade.ebn file.
     No startup-sequence in this mode. Jump directly to the
     parser-loop on reset
     XTAL in BOOTICEMODE must be 7372800 Hz to be compatible
     with the org. JTAGICE-Firmware */
#define START_SIMPLE
//#define START_POWERSAVE
//#define START_BOOTICE

#ifndef START_BOOTICE
#define XTAL 16000000UL
// #define XTAL 4000000UL
#else
#warning "BOOTICE mode - External Crystal/Oszillator must be 7,3728 MHz"
#define XTAL 7372800
#endif

// UART handling - some definitions from P. Fleury's Library - thanks
#define BAUDRATE 19200
#include "uart.h"

/* enable/disable readout of fuse and lock-bits
  (will not work for Mega169 since not supported by AVRPROG 1.37 */
#define ENABLEREADFUSELOCK

/* enable/disable write of lock-bits
  WARNING: lock-bits can not be reseted by bootloader (as far as I know)
  Only protection no unprotection, "chip erase" from bootloader only
  clears the flash but does no real "chip erase" (this is not possible
  with a bootloader as far as I know)
  Keep this undefined!
 */
// #define WRITELOCKBITS

/* Select Boot Size (select one, comment out the others) */
// NO! #define _B128
// NO! #define _B256
// #define _B512
#define _B1024
//#define _B2048

#include "chipdef.h"

#define UART_RX_BUFFER_SIZE SPM_PAGESIZE
unsigned char gBuffer[UART_RX_BUFFER_SIZE];

#define eeprom_is_ready()             bit_is_clear(EECR, EEWE)
#define my_eeprom_busy_wait()         do{}while(!eeprom_is_ready())

unsigned char BufferLoad(unsigned int , unsigned char ) ;
void BlockRead(unsigned int , unsigned char ) ;

uint32_t address;
unsigned char device;

void send_boot(void)
{
	sendchar('A');
	sendchar('V');
	sendchar('R');
	sendchar('B');
	sendchar('O');
	sendchar('O');
	sendchar('T');
}

void (*jump_to_app)(void) = 0x0000;

int main(void)
{
	unsigned  tempi;
	char val;

	#ifdef START_POWERSAVE
	char OK = 1;
	#endif

	cli();

	MCUCR = (1<<IVCE);
	MCUCR = (1<<IVSEL);             //move interruptvectors to the Boot sector

	USART_Init(UART_BAUD_SELECT(BAUDRATE,XTAL),UARTSINGLE); 	// single speed
	// USART_Init(UART_BAUD_SELECT(BAUDRATE/2,XTAL),UARTDOUBLE);  // double speed

	#if defined(START_POWERSAVE)
	/*
		This is an adoption of the Butterfly Bootloader startup-sequence.
		It may look a little strange but separating the login-loop from
		the main parser-loop gives a lot a possibilities (timeout, sleep-modes
	    etc.).
	*/
	BLDDR  &= ~(1<<BLPNUM);	// set as Input
	BLPORT |= (1<<BLPNUM);		// Enable pullup

	for(;OK;)
	{
		if((BLPIN & (1<<BLPNUM)))
		{
			// jump to main app if pin is not grounded
			BLPORT &= ~(1<<BLPNUM);  // set to default
			MCUCR = (1<<IVCE);
			MCUCR = (0<<IVSEL);      // move interruptvectors to the Application sector
			jump_to_app();	// Jump to application sector
		}
		else
		{
			val = recchar();

			if( val == 0x1B ) /* ESC */
	        {				// AVRPROG connection
				while (val != 'S')	// Wait for signon
				{
					val = recchar();
				}
				send_boot();					// Report signon
				OK = 0;
			}
			else
				sendchar('?');
	        }
		// Power-Save code here
	}

	#elif defined(START_SIMPLE)
	BLDDR  &= ~(1<<BLPNUM);	// set as Input
	BLPORT |= (1<<BLPNUM);		// Enable pullup

	sendchar('b');

	if((BLPIN & (1<<BLPNUM)))
	{
		sendchar('a');
		// jump to main app if pin is not grounded
		BLPORT &= ~(1<<BLPNUM);  // set to default
		MCUCR = (1<<IVCE);
		MCUCR = (0<<IVSEL);             //move interruptvectors to the Application sector
		jump_to_app();	// Jump to application sector
	}
	sendchar('p');

	#elif defined(START_BOOTICE)
	#warning "BOOTICE mode - no startup-condition"

	#else
	#error "Select START_ condition for bootloader in main.c"
	#endif

    for(;;)
    {
		val=recchar();

        if(val=='a')                        //Autoincrement?
        {
          sendchar('Y');			//Autoincrement is quicker
        }

        else if(val=='A')                   //write address
        {
			address=recchar();                //read address 8 MSB
			address=(address<<8)|recchar();

			address=address<<1;               // !! convert from word address to byte address
			sendchar('\r');
        }

        else if(val=='b')
		{									// Buffer load support
			sendchar('Y');					// Report buffer load supported
			sendchar((UART_RX_BUFFER_SIZE >> 8) & 0xFF);
											// Report buffer size in bytes
			sendchar(UART_RX_BUFFER_SIZE & 0xFF);
		}

		else if(val=='B')					// Start buffer load
		{
			tempi = recchar() << 8;			// Load high byte of buffersize
			tempi |= recchar();				// Load low byte of buffersize
			val = recchar();				// Load memory type ('E' or 'F')
			sendchar (BufferLoad(tempi,val));
											// Start downloading of buffer
		}

		else if(val == 'g')					// Block read
		{
			tempi = (recchar() << 8) | recchar();

			val = recchar();				// Get memtype
			BlockRead(tempi,val);			// Perform the block read
		}

        else if(val=='e')                   //Chip erase
        {
			if (device == devtype)
			{
				// erase only main section (bootloader protection)
				address = 0;
				while ( APP_END > address )
				{
					boot_page_erase(address);	// Perform page erase
					boot_spm_busy_wait();		// Wait until the memory is erased.
					address += SPM_PAGESIZE;
				}
			}
			boot_rww_enable();
			sendchar('\r');
        }

        else if(val=='E')                   //Exit upgrade
        {
			wdt_enable(WDTO_15MS); // Enable Watchdog Timer to give reset
			sendchar('\r');
        }

	   #ifdef WRITELOCKBITS
	   #warning "Extension 'WriteLockBits' enabled"
	   // TODO: does not work reliably
        else if(val=='l')                   // write lockbits
        {
			if (device == devtype)
			{
				// write_lock_bits(recchar());
				boot_lock_bits_set(recchar());	// boot.h takes care of mask
				boot_spm_busy_wait();
			}
			sendchar('\r');
		}
		#endif

        else if(val=='P')     // Enter programming mode
        {
			sendchar('\r');
        }

        else if(val=='L')   // Leave programming mode
        {
			sendchar('\r');
        }

        else if (val=='p')		// return programmer type
        {
          sendchar('S');		// always serial programmer
        }

#ifdef ENABLEREADFUSELOCK
#warning "Extension 'ReadFuseLock' enabled"
        else if(val=='F')                   // read "low" fuse bits
        {
			sendchar( read_fuse_lock(0x0000, _BV(BLBSET)|_BV(SPMEN)) );
        }

        else if(val=='r')                   // read lock bits
        {
			sendchar( read_fuse_lock(0x0001, _BV(BLBSET)|_BV(SPMEN)) );
        }

        else if(val=='N')                   // read high fuse bits
        {
			sendchar( read_fuse_lock(0x0003,_BV(BLBSET)|_BV(SPMEN)) );
        }

        else if(val=='Q')                   // read extended fuse bits
        {
			sendchar( read_fuse_lock(0x0002,_BV(BLBSET)|_BV(SPMEN)) );
        }
#endif
// end of ENABLEREADFUSELOCK section

        else if(val=='t')                   // Return device type
        {
			sendchar(devtype);
			sendchar(0);
        }

        else if ((val=='x')||(val=='y'))	// clear and set LED ignored
        {
			recchar();
			sendchar('\r');
        }

		else if (val=='T')					// set device
		{
			device = recchar();
			sendchar('\r');
		}

        else if (val=='S')                  // Return software identifier
        {
			send_boot();
        }

        else if (val=='V')                  // Return Software Version
        {
			sendchar(VERSION_HIGH);
			sendchar(VERSION_LOW);
        }

        else if (val=='s')                  // Return Signature Byte
        {
			sendchar(sig_byte1);
			sendchar(sig_byte2);
			sendchar(sig_byte3);
        }

	else if (val=='X') {
		BLPORT &= ~(1<<BLPNUM);  // set to default
		MCUCR = (1<<IVCE);
		MCUCR = (0<<IVSEL);             //move interruptvectors to the Application sector
		jump_to_app();	// Jump to application sector
	}

        else if(val != 0x1b) /* ESC */
        {
			sendchar('?');
        }
    }	// end of "parser" for-loop

	return 0;
}


unsigned char BufferLoad(unsigned int size, unsigned char mem)
{
	unsigned int data, cnt;
	uint32_t tempaddress;

	for (cnt=0; cnt<UART_RX_BUFFER_SIZE; cnt++) {
		if (cnt<size) gBuffer[cnt]=recchar();
		else gBuffer[cnt]=0xFF;
	}
	cnt=0;

	tempaddress = address;					// Store address in page

	my_eeprom_busy_wait();

	if (device == devtype)
	{
		if (mem == 'F')		// Flash
		{
			do {
				data=gBuffer[cnt++];
				data|=(gBuffer[cnt++]<<8);
				boot_page_fill(address,data);
											//call asm routine.
				address=address+2;  		// Select next word in memory
				size -= 2;					// Reduce number of bytes to write by two
			} while(size);					// Loop until all bytes written

			/* commented out since not compatible with mega8 -
			   secondary benefit: saves memory
			tempaddress &= 0xFF80;			// Ensure the address points to the first byte in the page
			*/

			boot_page_write(tempaddress);
			boot_spm_busy_wait();
			boot_rww_enable();				//Re-enable the RWW section

			/* commented out since not compatible with mega8
			if (address != (address & 0xFF80))
			{								// Ensure that the address points to the beginning of the next page
				address &= 0xFF80;
				address += SPM_PAGESIZE;
			}
			*/
		}									// End FLASH

		if (mem == 'E')						// Start EEPROM
        {
			// address>>=1;
			do {
	 	        EEARL = address;			// Setup EEPROM address
	            EEARH = (address >> 8);
				address++;					// Select next byte
				EEDR=gBuffer[cnt++];

	            EECR |= (1<<EEMWE);			// Write data into EEPROM
	            EECR |= (1<<EEWE);

				while (EECR & (1<<EEWE));	// Wait for EEPROM write to finish
				size--;						// Decreas number of bytes to write
			} while(size);					// Loop until all bytes written

		}

		return '\r';						// Report programming OK

	}

	return 0;								// Report programming failed
}

void BlockRead(unsigned int size, unsigned char mem)
{
	unsigned int data;

	my_eeprom_busy_wait();

	if (mem == 'E')							// Read EEPROM
	{
		address>>=1; // not needed here - hmm, somehow inconsistant TODO
		do {
			EEARL = address;				// Setup EEPROM address
			EEARH = (address >> 8);
			address++;						// Select next EEPROM byte
			EECR |= (1<<EERE);				// Read EEPROM
			sendchar(EEDR);					// Transmit EEPROM data to PC
			size--;							// Decrease number of bytes to read
		} while (size);					// Repeat until all block has been read
	}
	else									// Read Flash
	{
		do {
#if defined(RAMPZ)
			data = pgm_read_word_far(address);
#else
			data = pgm_read_word_near((uint16_t)address);
#endif
			sendchar((unsigned char)data);		//send LSB
			sendchar((unsigned char)(data >> 8));	//send MSB
			address += 2;  					// Select next word in memory
			size -= 2;						// Subtract two bytes from number of bytes to read
		} while (size);					// Repeat until all block has been read
	}
}
