//
//  Low-level routines to read lock and fuse-bytes
//
//  Copyright (C) 2/2005 Martin Thomas, Kaiserslautern, Germany
//

#include "lowlevel.h"

unsigned char read_fuse_lock(unsigned short addr, unsigned char mode)
{
	unsigned char retval;
	
	asm volatile
	(
		"movw r30, %3\n\t" /* Z to addr */ \
		"sts %0, %2\n\t" /* set mode in SPM_REG */ \
		"lpm\n\t"  /* load fuse/lock value into r0 */ \
		"mov %1,r0\n\t" /* save return value */ \
		: "=m" (SPM_REG),
		  "=r" (retval)
		: "r" (mode),
		  "r" (addr)
		: "r30", "r31", "r0"
	);
	return retval;
}

