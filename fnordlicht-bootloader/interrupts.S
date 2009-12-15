.extern __init
.global __vector_default
.section .vectors.bootloader

/* micro-jumptable, we are using just the reset vector */
exit:
__vector_default:
    /* use jmp if available on this architecture */
    #ifdef __AVR_HAVE_JMP_CALL__
        jmp __init
    #else
        rjmp __init
    #endif
