
======================================================

  ATMEL AVR UART Bootloader for AVR-GCC/avr-libc
  based on the AVR Butterfly bootloader code

  by Martin Thomas, Kaiserslautern, Germany
  mthomas@rhrk.uni-kl.de 
  eversmith@heizung-thomas.de

  http://www.siwawi.arubi.uni-kl.de/avr_projects

======================================================

23. Feb. 2005 - Version 0.7

* (Version 0.6 has never been available on the web-page)
* ATmega128 support
* code cleanup
* Tested with ATmega8, ATmega32 and ATmega128

7. Apr. 2004 - Version 0.5

* added different startup-methods
* compatible with ATmega8 now
* included makefile adopted to ATmega8 now 
  (ATmega16 options still available)
* fixed jump to application which did not work
  reliably before
* tested with ATmega8
* minimal options and startup-code result in 
  bootloader-size < 512 words

6. Apr. 2004 - Version 0.4

* Buffered read of chars from UART during programming
since eeprom-write is too slow for unbuffered 
operation. So EEPROM-upload does work now.
* Added BOOTICE-mode to flash JTAGICE-compatible
hardware (ATmega16@7,3Mhz) (if you know about BOOTICE,
you may unterstand why this has been added, if not
just keep the option disabled)
* small changes in (my)boot.h (lock-bit-mask) found
out during the development of the STK-500-compatible
bootloader. But setting lock-bits still does not
work with this bootloader.
* read of the low-fuse byte works (high byte still TODO)
* read of the lock-byte works (write still TODO)

27. Mar 2004 - Version 0.3

Felt that as much functions from avr-libc's boot.h
as possible should be used without modifications. 
Latest CVS-version of boot.h is included.
Only the read-routine is still "self-made" based
on ATMELs assembler-code.
EEPROM write on Mega16 does not work (and did not
work with V0.2 too). May be caused by my old Mega16
chip. Needs testing. Flash read/write and EEPROM
read works. Still only tested with ATmega16.
This version may not work with the ATmega169 any 
more.

24. Mar 2004 - Version 0.2

During the development of a data-logger application
with the AVR-Butterfly there was a need to make
some changes in the bootloader. The same problem
again: no IAR compiler. The same way to solve the
problem: a port of the code to avr-gcc/avr-libc.
So this code is based on the ATMEL Butterfly 
bootloader source code Rev 0.2 for IAR.

The bootloader-port for the Butterfly which mimics
the complete functionality of the original 
BF-bootloader is availabe at:
www.siwawi.arubi.uni-kl.de/avr_projects

Atmel used a separate "lib" written in "pure" 
assembly to access the low-level functions
for flash read/write. Well, so far I
don't know how to use "mixed language sources" 
with the avr-gcc toolchain, so the low-level
routines have been implemented as inline assembler.
The avr-libc boot.h module written by Eric
Weddington served as a template  Three of the four 
low-level routines found in lowlevel.c come from 
boot.h with minimal changes. The read routine has 
been developed based on the ATMEL assembler code.

Ignore the fuse and lock-bit readout. Read and Set is
not enabled (TODO).


--------------- Installation -----------------

- Change the MCU type in the makefile (so far
  ATmega16 has been tested, ATmega169, ATmega8 
  and ATmega32 should be o.k. too.

- Change the boot(loader)-size in main.c, this
  bootloader is larger than 512 words (1024 bytes), 
  so select at least _B1024!

- Change the XTAL in main.c to the clock-frequency
  of your board (keep BAUDRATE at 19200). See
  the datasheet for frequencies with minimum 
  error at 19200bps and select Crystal/Oscillator
  to get minimum errors.

- Change the start-condition in main.c. Default
  is: enter bootloader if Pin A7 is connected to
  GND during reset/startup

- Edit the value MT_BOOTLOADER_ADDRESS in the 
  makefile according to the selected bootloader
  size. Keep in mind that this value has to be
  given in bytes (not words) in the linker options.

  i.e. ATMega16, boot-size 1024 words, see
  datasheet and find "Boot Reset Adress" for
  "Boot Size" 1024 words is 1C00. 1C00 is given
  in words, so set MT_BOOTLOADER_ADDRESS to
  3800 (=2*1C00)

- Please use at least avr-gcc 3.3.1/avr-libc 1.0 
  or WINAVR Sept. 2003 or later to compile and link 
  the bootloader.

- upload the hex-File to the AVR (STK500, STK200, SP12
  etc.)

- program the "Boot Flash section size" (BOOTSZ fuses)
  according to the boot-size selected in main.c
  i.e. BOOTSZ=00 for boot-size 1024 words on ATmega16

- enable the BOOT Reset Vector (BOOTTRST=0)

- Set the lock bits to protect the bootloader from
  SPM-writes (Boot Loader Protection Mode 2 in STK500-
  plugin)

- Connect the AVR UART Pins via level-shifter/inverter
  (i.e. MAX232) to you PCs Com-Port.

- Reset the AVR while fullfilling the bootloader start-
  condition. Which means connect PA7 to GND in the default 
  config during reste/power-cycle. Keep the connection 
  or hold the key down until you see the AVRPROG dialog!

- Start AVRPROG (AVRStuido/Tools or stand-alone) - 
  keep PA7 grounded!

- AVRPROG will detect the bootloader, you may release
  PA7 now

- see AVRStuido online-help for more information how
  to use AVRPROG

- make sure to EXIT from AVRPROG (button) to start
  your main-application (or toogle power/reset)

good luck, feedback welcome.
Martin
