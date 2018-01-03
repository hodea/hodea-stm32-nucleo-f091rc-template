/*
 * gcc linker script for the application.
 */

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x20008000;    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x200;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  m_bootloader (rx)         : ORIGIN = 0x08000000, LENGTH = 0x2000
  m_appl_info (r)           : ORIGIN = 0x08002000, LENGTH = 0x40
  m_isr_vector (r)          : ORIGIN = 0x08002040, LENGTH = 0xbc
  FLASH (rx)                : ORIGIN = 0x080020fc, LENGTH = 0x3df04
  m_appl_vector_ram (rw)    : ORIGIN = 0x20000000, LENGTH = 0xbc
  m_boot_data (rw)          : ORIGIN = 0x200000bc, LENGTH = 0x10
  RAM (rw)                  : ORIGIN = 0x200000d0, LENGTH = 0x7f30
  /*
   * When using Segger tools the option bytes must be located at
   * 0x06000000. Keil and ST-Link/V2 are able to handle option bytes
   * located at 0x1ffff800.
   *
   * From the Segger Wiki:
   * > Note: The address 0x06000000 is a virtual address only. The option
   * > bytes are originally located at address 0x1FFFF800. The remap from
   * > 0x06000000 to 0x1FFFF800 is done automatically by J-Flash.
   */
/*  m_option_bytes (r)        : ORIGIN = 0x1ffff800, LENGTH = 0x10 */
  m_option_bytes (r)        : ORIGIN = 0x06000000, LENGTH = 0x10
}

/* Define output sections */
SECTIONS
{
  /* bootloader */
  .bootloader :
  {
    KEEP(*(.bootloader*))
  } > m_bootloader

  /* Application information */
  .appl_info :
  {
    . = ALIGN(4);
    KEEP(*(.appl_info*))
    . = ALIGN(4);
  } >m_appl_info

  /* Application vector table */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >m_isr_vector


  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM AT> FLASH

  
  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* Copy of application interrupt vector table in RAM. */
  .appl_vector_ram (NOLOAD) :
  {
    KEEP(*(.appl_vector_ram*))
  } > m_appl_vector_ram

  /* Persistent data shared between bootloader and application. */
  .boot_data (NOLOAD) :
  {
    KEEP(*(.boot_data*))
  } > m_boot_data

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* option bytes */
  .option_bytes :
  {
    KEEP(*(.option_bytes*))
  } > m_option_bytes

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}


