ENTRY(start)
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i8086")

MEMORY
{
    rom (rx) : ORIGIN = 0, LENGTH = 64K
}

PHDRS
{
  text PT_LOAD ;
  data PT_LOAD ;
}

SECTIONS
{
    . = 0;

    /* header for Option ROM (signature, size in 512-byte chunks), and jump to code */
    .optionhdr : ALIGN(1) {
        BYTE(0x55)
        BYTE(0xAA)
        BYTE(0x80)
        /* short jump past description strings */
        *(.optionrom_short)
        /* optional description / copyright info */
        *(.description)
    } >rom :text

    . = ALIGN(16);

    .text : {
        /* longer jump to actual entry point */
        *(.optionrom_long)
        *(.text)
    } >rom :text

    . = ALIGN(64);

    .data : {
        *(.data)
        *(.rdata)
    } >rom :data

    /* F000:E05B POST Entry Point */
    .entry 0xE05B : {
        *(.entry)
    } >rom

    /* F000:E2C3 NMI Entry Point */
    .nmi 0xE2C3 : {
        *(.nmi)
    } >rom

    /* F000:E6F2 INT 19 Entry Point */
    .int19 0xE6F2 : {
        *(.int19)
    } >rom

    /* F000:E6F5 Configuration Data Table */
    .cfgtable 0xE6F5 : {
        *(.cfgtable)
    } >rom

    /* F000:E729 Baut Rate Generator Table */
    .baudtable 0xE729 : {
        *(.baudtable)
    } >rom

    /* F000:E739 INT 14 Entry Point */
    .int14 0xE739 : {
        *(.int14)
    } >rom

    /* F000:E82E INT 16 Entry Point */
    .int16 0xE82E : {
        *(.int16)
    } >rom

    /* F000:E987 INT 09 Entry Point */
    .int9 0xE987 : {
        *(.int9)
    } >rom

    /* F000:EC59 INT 13 (Floppy) Entry Point */
    .int13 0xEC59 : {
        *(.int13)
    } >rom

    /* F000:EF57 INT 0E Entry Point */
    .int0e 0xEF57 : {
        *(.int0e)
    } >rom

    /* F000:EFC7 Floppy Disk Controller Parameter Table */
    .fdctab 0xEFC7 : {
        *(.fdctab)
    } >rom

    /* F000:EFD2 INT 17 */
    .int17 0xEFD2 : {
        *(.int17)
    } >rom

    /* F000:F065 INT 10 (Video) Entry Point */
    .int10 0xF065 : {
        *(.int10)
    } >rom

    /* F000:F0A4 INT 1D MDA and CGA Video Parameter Table */
    .int1d 0xF0A4 : {
        *(.int1d)
    } >rom

    /* F000:F841 INT 12 Entry Point */
    .int12 0xF841 : {
        *(.int12)
    } >rom

    /* F000:F84D INT 11 Entry Point */
    .int11 0xF84D : {
        *(.int11)
    } >rom

    /* F000:F859 INT 15 Entry Point */
    .int15 0xF859 : {
        *(.int15)
    } >rom

    /* F000:FA6E Low 128 Characters of Graphic Video Font */
    .font 0xFA6E : {
        *(.font)
    } >rom

    /* F000:FE6E INT 1A Entry Point */
    .int1a 0xFE6E : {
        *(.int1a)
    } >rom

    /* F000:FEA5 INT 08 Entry Point */
    .int8 0xFEA5 : {
        *(.int8)
    } >rom

    /* F000:FF53 Dummy Interrupt Handler (IRET) */
    .emptyint 0xFF53 : {
        *(.emptyint)
    } >rom

    /* F000:FF54 INT 05 (Print Screen) Entry Point */
    .int5 0xFF54 : {
        *(.int5)
    } >rom

    /* F000:FFF0 Power-On Entry Point */
    .poweron 0xFFF0 : {
        *(.poweron)
    } >rom

    /* F000:FFF5 ROM Date in ASCII "MM/DD/YY" Format (8 Characters) */
    .romdate 0xFFF5 : {
        *(.romdate)
    } >rom

    /* F000:FFFE System Model (0xFC - AT, 0xFE - XT) */
    .sysmodel 0xFFFE : {
        BYTE(0xFC)

        /* pad image to 64K */
        BYTE(0x00)
    } >rom

    /DISCARD/ : { *(.comment) *(.eh_frame) *(.stab) *(.stabstr) }
}
