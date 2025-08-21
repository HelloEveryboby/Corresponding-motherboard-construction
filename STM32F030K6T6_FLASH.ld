/*
 * Linker script for the STMicroelectronics STM32F030K6T6 MCU
 * 32 KB FLASH, 4 KB RAM
 */

/* Define memory regions */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 32K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 4K
}

/* Define the entry point of the application */
ENTRY(Reset_Handler)

/* Define a symbol for the top of the stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  /* The Interrupt Vector Table goes at the beginning of FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Vector table */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other read-only data */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (read-only data) */
    *(.rodata*)        /* .rodata* sections (read-only data) */
    . = ALIGN(4);
    _etext = .;        /* Define a global symbol at end of code */
  } >FLASH

  /* Initialized data section */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* Create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    . = ALIGN(4);
    _edata = .;        /* Define a global symbol at data end */
  } >RAM AT> FLASH

  /* Uninitialized data section (BSS) */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;         /* Define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;         /* Define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + 0x200; /* Minimum Heap Size */
    . = . + 0x400; /* Minimum Stack Size */
    . = ALIGN(8);
  } >RAM
}
