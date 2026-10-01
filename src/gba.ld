OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {
  rom   : ORIGIN = 0x08000000, LENGTH = 32M
  iwram : ORIGIN = 0x03000000, LENGTH = 0x7E00
}

SECTIONS {
  .text : {
    KEEP(*(.crt0))
    *(.text .text.*)
    *(.rodata .rodata.*)
    . = ALIGN(4);
  } > rom

  .ARM.exidx : { *(.ARM.exidx*) } > rom

  .data : {
    __data_start = .;
    *(.data .data.*)
    *(.iwram .iwram.*)
    . = ALIGN(4);
    __data_end = .;
  } > iwram AT > rom
  __data_lma = LOADADDR(.data);

  .bss (NOLOAD) : {
    __bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end = .;
  } > iwram

  /DISCARD/ : { *(.comment) *(.ARM.attributes) }
}
