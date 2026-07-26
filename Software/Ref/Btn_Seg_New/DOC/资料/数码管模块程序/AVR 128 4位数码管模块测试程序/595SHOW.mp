NOTE: AVR Code addresses are word addresses
All other addresses (including ones in FLASH) are byte addresses

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                           idata   008C   0013 =     19. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       008C  __idata_start
       009F  __idata_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            text   00A0   0194 =    404. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       0050  __text_start
       0050  __start
       0072  _exit
       0073  _Delay
       0088  _Delay1
       009D  _SPI_595
       00BF  _main
       0102  push_xgsetF000
       0107  pop_xgsetF000
       010C  push_xgsetF00C
       0113  pop_xgsetF00C
       011A  __text_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            data   0100   0013 =     19. bytes (rel,con,ram)

       Addr  Global Symbol
      -----  --------------------------------
       0100  __data_start
       0100  _tabnum
       0101  _Tab
       0113  __data_end

Area                               Start  End    Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                          vector   0000   0004 =      4. bytes (abs,ovr,rom)

Files Linked      [ module(s) ]

H:\iccv7avr\lib\crtatmega.o	[ crtatmega.s ]
595.o	[ _595.c ]
<library>	[ gpushy.s, gpushz.s ]

User Global Definitions

ram_end = 0x10ff
hwstk_size = 0x1e

User Base Address Definitions

func_lit = 0x8c
data = 0x100
eeprom:0.4096

