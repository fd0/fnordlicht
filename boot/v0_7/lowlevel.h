#ifndef LOWLEVEL_H
#define LOWLEVEL_H

#include <avr/io.h>

/* Check for SPM Control Register in processor. */
#if defined (SPMCSR)
#  define SPM_REG    SPMCSR
#elif defined (SPMCR)
#  define SPM_REG    SPMCR
#else
#  error AVR processor does not provide bootloader support!
#endif

unsigned char read_fuse_lock(unsigned short addr, unsigned char mode);


#endif
