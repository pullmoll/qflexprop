'' +--------------------------------------------------------------------------+
'' | Cluso's Z80 emulation for P2                                       v0.xx |
'' | Based largely on Z80 emulation code for P1 by pullmoll (Juergen)         |
'' |       qz80-0.9.32\qz80.spin                                              |
'' +--------------------------------------------------------------------------+
'' |  Author(s):     (c)2010 "pullmoll" (Juergen)     (original code)         |
'' |                 (c)2019 "Cluso99" (Ray Rodrick)  (rewrite for P2)        |
'' |  License:       Juergen gratiously gave me permission to modify his code |
'' +--------------------------------------------------------------------------+
'' Original qz80-0.9.32\qz80.spin by Pullmoll
''-------------------------------------------------
'' RR20190801   -rr000  start conversion from P1 
'' RR20190804   -rr001  Tidy ready for P2 version
''                 002  remove spektrum/etc options
''                 003  reorg alu_to_xx & xx_to_alu
'' RR20190805      004  fix vectors for P1/P2
''                 005  convert to 10b vectors & LUT
''                 007  continue; remove #ifdef PORT16 options
''                 009  compiles ok (not tested)
'' RR20190807      010  move routines around
''                 011  add crystal init and use ROM serial/monitor
''                 012  mask3FF, reposition code in hub, 1st vector 12-bit address, then 2 * 10-bit addresses
'' RR20190808      013  tweek
'' RR20190808      014  fix cog/lut load and start
''                 015  add debug code for each opcode fetch, set hub _clkfreq, _clkmode, _serbaud
''                 016
'' RR20190809      017  vector addresses validated; can fetch opcodes and decode the first byte and fetch vector(s)
''                 018  (posted on forum)
''                 019-021  tweek, jmp #next_vector --> _RET_ and xxx_ret ret --> _RET_ 
'' RR20190811      022  fix fetch loop (w debug)   (posted on forum)
'' RR20190814      023  test new fetch loop
'' RR20190816      024  _RET_ invalid on jmp/call  (posted on forum)
'' RR20190817      025  prepare for instruction validation (bdos call 5 coded using wr_port)
''                 026  try "zexall.src"->"zexall.cmd" at $0100 and OUT (nn),A at 5
''                 027  try "zexall.hex" loaded as bytes
''                 028  try "zexdoc.cim" (.cim is a binary file with org $0100)
''                      _debug_opc display 4*opc bytes
''                        requires bdos entry at $0005 to output string if c=9, de=addr
''                        make $xx = OUT nn,A perform write string at (DE) terminated with '$'
'' RR20190819      029  fix rdxxxx/wrxxxx with Z80_MEM offset
''                      add ea,ram_base -> or ea,ram_base
''                      insert OR xx,ram_base before rd/wrbyte/word in various places
''                 030-031  debug with zexdoc
''                 032  tweek

 
''============================[ CON ]============================================================
CON
'+-------[ Select for P2-EVAL ]------------------------------------------------+ 
'  _XTALFREQ     = 20_000_000                                    ' crystal frequency
'  _XDIV         = 2             '\                              '\ crystal divider                      to give 10.0MHz
'  _XMUL         = 15            '| 150 MHz                      '| crystal / div * mul                  to give 150MHz
'  _XDIVP        = 1             '/                              '/ crystal / div * mul /divp            to give 150MHz
'  _XOSC         = %10                                   '15pF   ' %00=OFF, %01=OSC, %10=15pF, %11=30pF
'+-------[ Select for P2D2 ]---------------------------------------------------+ 
  _XTALFREQ     = 12_000_000                                    ' crystal frequency
  _XDIV         = 4             '\                              '\ crystal divider                      to give   3.0MHz
  _XMUL         = 99            '| 148.5MHz                     '| crystal / div * mul                  to give 297.0MHz
  _XDIVP        = 2             '/                              '/ crystal / div * mul /divp            to give 148.5MHz
  _XOSC         = %01                                   'OSC    ' %00=OFF, %01=OSC, %10=15pF, %11=30pF
'+-----------------------------------------------------------------------------+
  _XSEL         = %11                                   'XI+PLL ' %00=rcfast(20+MHz), %01=rcslow(~20KHz), %10=XI(5ms), %11=XI+PLL(10ms)
  _XPPPP        = ((_XDIVP>>1) + 15) & $F                       ' 1->15, 2->0, 4->1, 6->2...30->14
  _CLOCKFREQ    = _XTALFREQ / _XDIV * _XMUL / _XDIVP            ' internal clock frequency                
  _SETFREQ      = 1<<24 + (_XDIV-1)<<18 + (_XMUL-1)<<8 + _XPPPP<<4 + _XOSC<<2  ' %0000_000e_dddddd_mmmmmmmmmm_pppp_cc_00  ' setup  oscillator
  _ENAFREQ      = _SETFREQ + _XSEL                                             ' %0000_000e_dddddd_mmmmmmmmmm_pppp_cc_ss  ' enable oscillator

  _1us          = _clockfreq/1_000_000                          ' 1us
'------------------------------------------------------------------------------------------------
'  _baud         = 115_200
  _baud         = 230_400
  _bitper       = (_clockfreq / _baud) << 16 + 7          ' 115200 baud, 8 bits
  _txmode       = %0000_0000_000_0000000000000_01_11110_0 'async tx mode, output enabled for smart output
  _rxmode       = %0000_0000_000_0000000000000_00_11111_0 'async rx mode, input  enabled for smart input
'------------------------------------------------------------------------------------------------
  rx_pin        = 63            ' pin serial receiver
  tx_pin        = 62            ' pin serial transmitter
  spi_cs        = 61            ' pin SPI memory select          (also sd_ck)
  spi_ck        = 60            ' pin SPI memory clock           (also sd_cs)
  spi_di        = 59            ' pin SPI memory data in         (also sd_di)
  spi_do        = 58            ' pin SPI memory data out        (also sd_do)
'------------------------------------------------------------------------------------------------

'' +--------------------------------------------------------------------------+
'' | Cluso's LMM_SerialDebugger for P2    (c)2013-2018 "Cluso99" (Ray Rodrick)|
'' +--------------------------------------------------------------------------+
''  xxxxxx : xx xx xx xx ... <cr>  DOWNLOAD:  to cog/lut/hub {addr1} following {byte(s)}
''  xxxxxx - [xxxxxx] [L] <cr>     LIST:      from cog/lut/hub {addr1} to < {addr2} L=longs
''  xxxxxx G <cr>                  GOTO:      to cog/lut/hub {addr1}
''  Q <cr>                         QUIT:      Quit Rom Monitor and return to the User Program
''  Lffffffff[.]xxx<cr>            LOAD:      Load file from SD
''  Rffffffff[.]xxx<cr>            RUN:       Load & Run file from SD
''  <esc><cr>                      TAQOZ:     goto TAQOZ
'' +--------------------------------------------------------------------------+
''   LMM DEBUGGER - CALL Modes...(not all modes supported)
'' +--------------------------------------------------------------------------+
  _MODE         = $F << 5                       ' mode bits defining the call b8..b5 (b4..b0 are modifier options)
  _SHIFT        = 5                             ' shr # to extract mode bits
  _HEX_         = 2 << 5                        ' hex...
    _REV_               = 1 << 4                '   - reverse byte order
    _SP                 = 1 << 3                '   - space between hex output pairs
   '_DIGITS             = 7..0 where 8->0       '   - no. of digits to display
  _LIST         = 3 << 5                        ' LIST memory line (1/4 longs) from cog/hub
    _ADDR2              = 1 << 4                ' 1= use lmm_p2 as to-address
    _LONG_              = 1 << 1                ' 1=display longs xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
  _TXSTRING     = 4 << 5                        ' tx string (nul terminated) from hub
  _RXSTRING     = 5 << 5                        ' rx string
    _ECHO_              = 1 << 4                '    - echo char
    _PROMPT             = 1 << 3                '    - prompt (lmm_x)
    _ADDR               = 1 << 2                '    - addr of string buffer supplied
    _NOLF               = 1 << 1                '    - strip <lf>
  _MONITOR      = 7 << 5                        ' goto rom monitor
'' +--------------------------------------------------------------------------+
''   P2 ROM SERIAL ROUTINES (HUBEXEC)
'' +--------------------------------------------------------------------------+
  _SerialInit      = $fcab8     ' Serial Initialise     (lmm_x & lmm_bufad must be set first)
  _hubTxCR         = $fcae4     ' Sends <cr><lf>        (overwrites lmm_x)
  _hubTxRev        = $fcaec     ' Sends lmm_x with bytes reversed
  _hubTx           = $fcaf0     ' Sends lmm_x           (can be up to 4 bytes)
  _hubHexRev       = $fcb24     ' Sends lmm_x with bytes reversed as Hex char(s) as defined in lmm_f
  _hubHex8         = $fcb28     ' Sends lmm_x as Hex char(s) after setting lmm_f as 8 hex chars
  _hubHex          = $fcb2c     ' Sends lmm_x as Hex char(s) as defined in lmm_f
  _hubTxStrVer     = $fcb9c     ' Sends $0 terminated string at lmm_p address after setting lmm_p=##_str_vers
  _hubTxString     = $fcba4     ' Sends $0 terminated string at lmm_p address
  _hubListA2H      = $fcbc4     ' List/Dump line(s) from lmm_p address to lmm_p2 address after setting lmm_f=#_LIST+_ADDR2 
  _hubList         = $fcbc8     ' List/Dump line(s) from lmm_p address to lmm_p2 address according to lmm_f
  _hubRx           = $fcb10     ' Recv char into lmm_x
  _hubRxStrMon     = $fccc4     ' Recv string into lmm_bufad address after setting prompt=lmm_x=#"*" & params=lmm_f=#_RXSTRING+_ECHO_+_PROMPT
  _hubRxString     = $fcccc     ' Recv string into lmm_p/lmm_bufad address according to params in lmm_f
  _hubMonitor      = $fcd78     ' Calls the Monitor; uses lmm_bufad as the input buffer address
  _RdLongCogHub    = $fcf34     ' read cog/lut/hub long from lmm_p address into lmm_x, then lmm_p++
  _str_vers        = $fd014     ' locn of hub string, $0 terminated
'' +--------------------------------------------------------------------------+
'' HUB ADDRESSES
'' +--------------------------------------------------------------------------+
  _HUBROM       = $FC000        ' ROM $FC000
  _HUBBUF       = $FC000        ' overwrite Booter
  _HUBBUFSIZE   = 80            ' RxString default size for _HUBBUF
'' +--------------------------------------------------------------------------+

''============[ COG VARIABLES $1E0-$1EF - MONITOR]=============================
''-------[ LMM parameters, etc ]-----------------------------------------------
  lmm_x         = $1e0          ' parameter passed to/from LMM routine (typically a value)
  lmm_f         = $1e1          ' parameter passed to      LMM routine (function options; returns unchanged)
  lmm_p         = $1e2          ' parameter passed to/from LMM routine (typically a hub/cog ptr/addr)
  lmm_p2        = $1e3          ' parameter passed to/from LMM routine (typically a 2nd hub/cog address)
  lmm_c         = $1e4          ' parameter passed to/from LMM routine (typically a count)
''-------[ LMM additional workareas ]------------------------------------------
  lmm_w         = $1e5          ' workarea (never saved - short term use between calls, except _HubTx)
  lmm_tx        = $1e6          ' _HubTx
  lmm_hx        = $1e7          ' _HubHex/_HubString
  lmm_hx2       = $1e8          ' _HubHex
  lmm_hc        = $1e9          '   "
  lmm_lx        = $1ea          ' _HubList
  lmm_lf        = $1eb          '   "
  lmm_lp        = $1ec          '   "
  lmm_lp2       = $1ed          '   "
  lmm_lc        = $1ee          '   "
  lmm_bufad     = $1ef          ' _HubRxString
'' +--------------------------------------------------------------------------+
'' ASCII equates
'' +--------------------------------------------------------------------------+
  _CLS_         = $0C     
  _BS_          = $08
  _LF_          = $0A
  _CR_          = $0D
  _TAQOZ_       = $1B           ' <esc>   goto TAQOZ
'' +--------------------------------------------------------------------------+

CON
        SF                      =       %10000000       ' sign
        ZF                      =       %01000000       ' zero
        YF                      =       %00100000       ' undocumented y
        HF                      =       %00010000       ' auxiliary (half) carry
        XF                      =       %00001000       ' undocumented x
        PF                      =       %00000100       ' parity
        VF                      =       PF              ' overflow (shares the parity flag bit)
        NF                      =       %00000010       ' negative
        CF                      =       %00000001       ' carry

        bit7                    =       %10000000
        bit6                    =       %01000000
        bit5                    =       %00100000
        bit4                    =       %00010000
        bit3                    =       %00001000
        bit2                    =       %00000100
        bit1                    =       %00000010
        bit0                    =       %00000001
        
        io_cmd_out              =       $01             ' command to write to an i/o port
        io_cmd_in               =       $02             ' command to read from an i/o port
        io_cmd_break            =       $03             ' command to execute a break (halt)
        io_cmd_kbd              =       $04             ' command to read the TRS-80 keyboard matrix

        IFF1                    =       %00000001       ' first interrupt flip-flop
        IFF2                    =       %00000010       ' second interrupt flip-flop
        IM1                     =       %00000100       ' interrupt mode 1 flag
        IM2                     =       %00001000       ' interrupt mode 2 flag
        HALT                    =       %00010000       ' CPU is halted flag

        Z80_MEM                 =       $1_0000         ' Z80 memory 64KB
        
DAT
'===============================================================================================================
' Initialisation code (gets copied to cog and run)...
'===============================================================================================================
                orgh    0
                org     0
''+-------[ Set Xtal ]---------------------------------------------------------+ 
init_entry      hubset  #0                              ' set 20MHz+ mode
                hubset  ##_SETFREQ                      ' setup oscillator
                waitx   ##20_000_000/100                ' ~10ms
                hubset  ##_ENAFREQ                      ' enable oscillator
                setq    #3-1                            '\ copy clkfreq/clockmode/serialbaud
                wrlong  __clkfreq, #$14                 '/    to hub $14/$18/$1C
''+-------[ delay ]------------------------------------------------------------+
                waitx   ##_clockfreq*2                  ' just a delay to get pc terminal running
''+-------[ Start Serial ]-----------------------------------------------------+ 
                mov     lmm_bufad,        ##_HUBBUF     ' locn of hub buffer for serial routine 
                mov     lmm_x,            ##_bitper     ' sets serial baud
                call    #_SerialInit                    ' initialise serial
''+------[ Display string (in hub, $0 terminated) ]----------------------------+
                mov     lmm_f,            #_TXSTRING+0  ' send string, $00 terminated (not required)
                mov     lmm_p,            ##@hubidstring' must be in hub!
                call    #_HubTxString
''+------[ Load LUT code ]-----------------------------------------------------+
                setq2   #512-1                          '\ load LUT
                rdlong  0, ##@lut_code                 '/
''+------[ Jump to HUBEXEC (loads new COG code) ]------------------------------+
                jmp     #reload_cog                     ' jump to hubexec & reload new cog code
''+----------------------------------------------------------------------------+
__clkfreq       long    _CLOCKFREQ
__clkmode       long    _SETFREQ
__serbaud       long    _BAUD
''+----------------------------------------------------------------------------+
                fit     $100                            ' max 1KB
''+============================================================================+

'===============================================================================================================
' HUBEXEC code...
'===============================================================================================================
                orgh    $400
'***************************************************************************************************************
' Store all registers in their hub RAM locations
' and send a break command to the io handler
break                   mov     ea, regptr                      ' destination in hub RAM
                        wrbyte  C, ea
                        add     ea, #1
                        wrbyte  B, ea
                        add     ea, #1
                        wrbyte  E, ea
                        add     ea, #1
                        wrbyte  D, ea
                        add     ea, #1
                        wrbyte  L, ea
                        add     ea, #1
                        wrbyte  H, ea
                        add     ea, #1
                        wrbyte  F, ea
                        add     ea, #1
                        wrbyte  A, ea
                        add     ea, #(@R_reg - @A_reg)          ' skip alternate register set    '????????
                        wrbyte  R, ea
                        add     ea, #1
                        wrbyte  R2, ea
                        add     ea, #1
                        wrbyte  IFF, ea
                        add     ea, #(@SP_reg - @IFF_reg)                                        '????????
                        wrword  SP, ea
                        add     ea, #2
                        wrword  PC, ea
                        wrlong  io_break, io_command
.wait                   rdlong  t1, io_command
                        test    t1, topbyte     WZ
                if_nz   jmp     #.wait
'                       jmp     #lmm_ret
                        jmp     #fetch                                                           '????????
'***************************************************************************************************************
zed_sbc_hl_sp   {ed 72} mov     t1, SP                          ' HL = HL-SP-CF & set FLAGS
                        and     t1, #$ff
                        mov     tmp, SP
                        shr     tmp, #8
                        test    F, #CF          WC              ' get C into prop's carry
                        subx    L, t1                           ' subtract the LSB tmp value with carry
                        test    L, #$100        WC              ' get LSB carry
                        and     L, #$ff
                        mov     F, #NF                          ' clear all flags but N flag for SBC
                        mov     alu, H
                        mov     aux, H
                        subx    alu, tmp                        ' subtract the MSB tmp value with carry
                        call    #szhvc_flags
                if_z    cmp     L, #0           WZ
                if_nz   andn    F, #ZF                          ' clear Z flag if L is non zero
                        jmp     #alu_to_h
'***************************************************************************************************************
zed_adc_hl_bc   {ed 4a} mov     t1, C                           ' HL = HL+BC+CF & set FLAGS
                        mov     tmp, B
                        test    F, #CF          WC              ' get C into prop's carry
                        addx    L, t1                           ' add the LSB tmp value with carry
                        test    L, #$100        WC              ' get LSB carry
                        and     L, #$ff
                        mov     F, #0                           ' clear all flags
                        mov     alu, H
                        mov     aux, H
                        addx    alu, tmp                        ' add the MSB tmp value with carry
                        call    #szhvc_flags
                if_z    cmp     L, #0           WZ
                if_nz   andn    F, #ZF                          ' clear Z flag if L is non zero
                        jmp     #alu_to_h
'***************************************************************************************************************
zed_adc_hl_de   {ed 5a} mov     t1, E                           ' HL = HL+DE+CF & set FLAGS
                        mov     tmp, D
                        test    F, #CF          WC              ' get C into prop's carry
                        addx    L, t1                           ' add the LSB tmp value with carry
                        test    L, #$100        WC              ' get LSB carry
                        and     L, #$ff
                        mov     F, #0                           ' clear all flags
                        mov     alu, H
                        mov     aux, H
                        addx    alu, tmp                        ' add the MSB tmp value with carry
                        call    #szhvc_flags
                if_z    cmp     L, #0           WZ
                if_nz   andn    F, #ZF                          ' clear Z flag if L is non zero
                        jmp     #alu_to_h
'***************************************************************************************************************
zed_adc_hl_hl   {ed 6a} mov     t1, L                           ' HL = HL+HL+CF & set FLAGS
                        mov     tmp, H
                        test    F, #CF          WC              ' get C into prop's carry
                        addx    L, t1                           ' add the LSB tmp value with carry
                        test    L, #$100        WC              ' get LSB carry
                        and     L, #$ff
                        mov     F, #0                           ' clear all flags
                        mov     alu, H
                        mov     aux, H
                        addx    alu, tmp                        ' add the MSB tmp value with carry
                        call    #szhvc_flags
                if_z    cmp     L, #0           WZ
                if_nz   andn    F, #ZF                          ' clear Z flag if L is non zero
                        jmp     #alu_to_h
'***************************************************************************************************************
zed_adc_hl_sp   {ed 7a} mov     t1, SP                          ' HL = HL+SP+CF & set FLAGS
                        and     t1, #$ff
                        mov     tmp, SP
                        shr     tmp, #8
                        test    F, #CF          WC              ' get C into prop's carry
                        addx    L, t1                           ' add the LSB tmp value with carry
                        test    L, #$100        WC              ' get LSB carry
                        and     L, #$ff
                        mov     F, #0                           ' clear all flags
                        mov     alu, H
                        mov     aux, H
                        addx    alu, tmp                        ' add the MSB tmp value with carry
                        call    #szhvc_flags
                if_z    cmp     L, #0           WZ
                if_nz   andn    F, #ZF                          ' clear Z flag if L is non zero
                        jmp     #alu_to_h
'***************************************************************************************************************
zed_ld_abs16_bc {ed 43} call    #rd_opword
                        mov     ea, alu
                        mov     alu, B
                        shl     alu, #8
                        or      alu, C
                        call    #wr_word
                        jmp     #fetch
'***************************************************************************************************************
zed_ld_abs16_de {ed 53} call    #rd_opword
                        mov     ea, alu
                        mov     alu, D
                        shl     alu, #8
                        or      alu, E
                        call    #wr_word
                        jmp     #fetch
'***************************************************************************************************************
zed_ld_abs16_hl_2 {ed 63} call    #rd_opword
                        mov     ea, alu
                        mov     alu, H
                        shl     alu, #8
                        or      alu, L
                        call    #wr_word
                        jmp     #fetch
'***************************************************************************************************************
zed_ld_abs16_sp {ed 73} call    #rd_opword
                        mov     ea, alu
                        mov     alu, SP
                        call    #wr_word
                        jmp     #fetch
'***************************************************************************************************************
zed_ld_bc_abs16 {ed 4b} call    #rd_opword
                        mov     ea, alu
                        call    #rd_word
                        jmp     #alu_to_bc
'***************************************************************************************************************
zed_ld_de_abs16 {ed 5b} call    #rd_opword
                        mov     ea, alu
                        call    #rd_word
                        jmp     #alu_to_de
'***************************************************************************************************************
zed_ld_hl_abs16_2 {ed 6b} call    #rd_opword
                        mov     ea, alu
                        call    #rd_word
                        jmp     #alu_to_hl
'***************************************************************************************************************
zed_ld_sp_abs16 {ed 7b} call    #rd_opword
                        mov     ea, alu
                        call    #rd_word
                        jmp     #alu_to_sp
'***************************************************************************************************************
zed_neg         {ed 44/4c/54/5c/64/6c/74/7c}
                        mov     tmp, A
                        mov     F, #NF                          ' clear all flags but the N flag
                        mov     alu, #0
                        mov     aux, #0
                        sub     alu, tmp                        ' subtract the tmp value
                        call    #szhvc_flags
                        jmp     #alu_to_a
'***************************************************************************************************************
zed_retn        {ed 45/55/65/75}
                        test    IFF, #IFF2              WZ      ' copy interrupt flip-flop #2
                        muxnz   IFF, #IFF1                      ' to #1
                        jmp     #ret_
'***************************************************************************************************************
zed_reti        {ed 4d/5d/6d/7d}
                        test    IFF, #IFF2              WZ      ' copy interrupt flip-flop #2
                        muxnz   IFF, #IFF1                      ' to #1
                        ' TODO: call Z80 SIO, PIO, other daisy chain peripherals
                        jmp     #ret_
'***************************************************************************************************************
zed_im_0        {ed 46/4e/66/6e}
                        andn    IFF, #(IM1 | IM2)
                        jmp     #fetch
'***************************************************************************************************************
zed_im_1        {ed 56/76}
                        andn    IFF, #IM2
                        or      IFF, #IM1
                        jmp     #fetch
'***************************************************************************************************************
zed_im_2        {ed 5e/7e}
                        andn    IFF, #IM1
                        or      IFF, #IM2
                        jmp     #fetch
'***************************************************************************************************************
zed_ld_i_a      {ed 47} mov     XY, regptr
                        add     XY, #(@I_reg - @C_reg)                                           '??????????????????
                        wrbyte  A, XY
                        jmp     #fetch
'***************************************************************************************************************
zed_ld_r_a      {ed 4f} mov     R, A
                        mov     R2, A
                        and     R2, #$80
                        jmp     #fetch
'***************************************************************************************************************
zed_ld_a_i      {ed 57} mov     XY, regptr
                        add     XY, #(@I_reg - @C_reg)                                           '??????????????????
                        rdbyte  A, XY
                        test    IFF, #IFF2              WZ      ' copy interrupt flip-flop #2
                        muxnz   F, #PF                          ' to parity flag
                        jmp     #fetch
'***************************************************************************************************************
zed_ld_a_r      {ed 5f} mov     A, R                            ' get current counter
                        and     A, #$7f                         ' mask lower 7 bits
                        or      A, R2                           ' combine with bit #7 from last write
                        test    IFF, #IFF2              WZ      ' copy interrupt flip-flop #2
                        muxnz   F, #PF                          ' to parity flag
                        jmp     #fetch
'***************************************************************************************************************
' ed 67 Rotate right digit memory (HL)
'       A:[7][6][5][4][3][2][1][0] M:[7][6][5][4][3][2][1][0]
'       before         x  x  x  x     y  y  y  y  z  z  z  z
'       after          z  z  z  z     x  x  x  x  y  y  y  y
zed_rrd_m               call    #rd_byte_hl
                        mov     tmp, alu
                        shr     tmp, #4                         ' upper nibble to 3..0
                        mov     aux, A
                        shl     aux, #4                         ' A to bits 11..4
                        and     aux, #$f0                       ' mask bits 7..4
                        or      tmp, aux                        ' combine with lower nibble
                        and     alu, #$0f                       ' mask 3..0 of memory
                        andn    A, #%0000_1111                  ' clears bits 3..0 in accu
                        or      A, alu                          ' combine with accu
                        mov     alu, tmp                        ' write back memory
                        call    #wr_byte
                        and     F, #CF                          ' keep C flag
                        and     A, #$ff         WZ, WC          ' get zero and parity
                        muxz    F, #ZF                          ' set Z flag on zero
                        muxnc   F, #PF                          ' clear P flag on parity
                        test    A, #$80         WZ              ' get sign
                        muxnz   F, #SF                          ' set S flag on sign
                        jmp     #fetch
'***************************************************************************************************************
' ed 6f Rotate left digit memory (HL)
'       A:[7][6][5][4][3][2][1][0] M:[7][6][5][4][3][2][1][0]
'       before         x  x  x  x     y  y  y  y  z  z  z  z
'       after          y  y  y  y     z  z  z  z  x  x  x  x
zed_rld_m               call    #rd_byte_hl
                        mov     tmp, alu
                        shl     tmp, #4                         ' mem to bits 11..4
                        mov     aux, A
                        and     aux, #$0f                       ' mask bits 3..0 of accu
                        or      tmp, aux                        ' combine with upper nibble
                        shr     alu, #4                         ' mem upper nibble to 3..0
                        andn    A, #%0000_1111                  ' clear bits 3..0 in accu
                        or      A, alu                          ' combine with accu
                        mov     alu, tmp                        ' write back memory
                        and     alu, #$ff                       ' needed?
                        call    #wr_byte
                        and     F, #CF                          ' keep C flag
                        and     A, #$ff         WZ, WC          ' get zero and parity
                        muxz    F, #ZF                          ' set Z flag on zero
                        muxnc   F, #PF                          ' clear P flag on parity
                        test    A, #$80         WZ              ' get sign
                        muxnz   F, #SF                          ' set S flag on sign
                        jmp     #fetch
'***************************************************************************************************************
zed_bcde_cnt    {ed 80} getct   t1                                                               '????????
                        mov     E, t1
                        and     E, #$ff
                        ror     t1, #8
                        mov     D, t1
                        and     D, #$ff
                        ror     t1, #8
                        mov     C, t1
                        and     C, #$ff
                        ror     t1, #8
                        mov     B, t1
                        and     B, #$ff
                        jmp     #fetch
'***************************************************************************************************************
zed_ldi         {ed a0} call    #rd_byte_hl
                        add     L, #1                           ' HL++
                        test    L, #$100        WC
                        and     L, #$ff
                        addx    H, #0
                        and     H, #$ff
                        mov     ea, D
                        shl     ea, #8
                        or      ea, E
                        call    #wr_byte
                        add     E, #1                           ' DE++
                        test    E, #$100        WC
                        and     E, #$ff
                        addx    D, #0
                        and     D, #$ff
                        sub     C, #1                           ' BC--
                        test    C, #$100        WC
                        and     C, #$ff         WZ
                        subx    B, #0
                if_nz   and     B, #$ff
                if_z    and     B, #$ff         WZ
                        and     F, #(SF | ZF | CF)
                if_nz   or      F, #PF
                        jmp     #fetch
'***************************************************************************************************************
zed_ldd         {ed a8} call    #rd_byte_hl
                        sub     L, #1                           ' HL--
                        test    L, #$100        WC
                        and     L, #$ff
                        subx    H, #0
                        and     H, #$ff
                        mov     ea, D
                        shl     ea, #8
                        or      ea, E
                        call    #wr_byte
                        sub     E, #1                           ' DE--
                        test    E, #$100        WC
                        and     E, #$ff
                        subx    D, #0
                        and     D, #$ff
                        sub     C, #1                           ' BC--
                        test    C, #$100        WC
                        and     C, #$ff         WZ
                        subx    B, #0
                if_nz   and     B, #$ff
                if_z    and     B, #$ff         WZ
                        and     F, #(SF | ZF | CF)
                if_nz   or      F, #PF
                        jmp     #fetch
'***************************************************************************************************************
zed_ldir        {ed b0} shl     H, #8                           ' pack HL
                        or      H, L
                        shl     D, #8                           ' pack DE
                        or      D, E
                        shl     B, #8                           ' pack BC
                        or      B, C
.loop                   mov     ea, H
                        call    #rd_byte
                        add     H, #1                           ' HL++
                        and     H, low_word
                        mov     ea, D
                        call    #wr_byte
                        add     D, #1                           ' DE++
                        and     D, low_word
                        sub     B, #1                           ' BC--
                        and     B, low_word     WZ
                if_nz   jmp     #.loop
                        mov     C, #0                           ' BC is 0
                        mov     E, D                            ' unpack D E
                        and     E, #$ff
                        shr     D, #8
                        mov     L, H                            ' unpack H L
                        and     L, #$ff
                        shr     H, #8
                        and     F, #(SF | ZF | CF)
                        jmp     #fetch
'***************************************************************************************************************
zed_lddr        {ed b8} shl     H, #8                           ' pack HL
                        or      H, L
                        shl     D, #8                           ' pack DE
                        or      D, E
                        shl     B, #8                           ' pack BC
                        or      B, C
.loop                   mov     ea, H
                        call    #rd_byte
                        sub     H, #1                           ' HL--
                        and     H, low_word
                        mov     ea, D
                        call    #wr_byte
                        sub     D, #1                           ' DE--
                        and     D, low_word
                        sub     B, #1                           ' BC--
                        and     B, low_word     WZ
                if_nz   jmp     #.loop
                        mov     C, #0                           ' BC is 0
                        mov     E, D                            ' unpack D E
                        and     E, #$ff
                        shr     D, #8
                        mov     L, H                            ' unpack H L
                        and     L, #$ff
                        shr     H, #8
                        and     F, #(SF | ZF | CF)
                        jmp     #fetch
'***************************************************************************************************************
zed_cpi         {ed a1} call    #rd_byte_hl
                        mov     tmp, alu
                        and     F, #CF                          ' keep carry flag
                        or      F, #NF                          ' always set negative flag
                        mov     alu, A
                        mov     aux, alu                        ' keep a copy in aux for later
                        sub     alu, tmp        WZ
                        muxz    F, #ZF                          ' set zero flag if SUB was zero
                        xor     aux, alu
                        xor     aux, tmp
                        and     aux, #HF                        ' mask the auxiliary flag
                        or      F, aux
                        test    alu, #$80       WZ
                        muxnz   F, #SF                          ' set sign flag if alu bit #7 is set
                        add     L, #1                           ' HL++
                        test    L, #$100        WC
                        and     L, #$ff
                        addx    H, #0
                        and     H, #$ff
                        sub     C, #1                           ' BC--
                        test    C, #$100        WC
                        and     C, #$ff         WZ
                        subx    B, #0
                if_nz   and     B, #$ff
                if_z    and     B, #$ff         WZ
                        muxnz   F, #PF                          ' set parity flag if BC was non zero
                        jmp     #fetch
'***************************************************************************************************************
zed_cpd         {ed a9} call    #rd_byte_hl
                        mov     tmp, alu
                        and     F, #CF                          ' keep carry flag
                        or      F, #NF                          ' always set negative flag
                        mov     alu, A
                        mov     aux, alu                        ' keep a copy in aux for later
                        sub     alu, tmp        WZ
                        muxz    F, #ZF                          ' set zero flag if SUB was zero
                        xor     aux, alu
                        xor     aux, tmp
                        and     aux, #HF                        ' mask the auxiliary flag
                        or      F, aux
                        test    alu, #$80       WZ
                        muxnz   F, #SF                          ' set sign flag if alu bit #7 is set
                        sub     L, #1                           ' HL--
                        test    L, #$100        WC
                        and     L, #$ff
                        subx    H, #0
                        and     H, #$ff
                        sub     C, #1                           ' BC--
                        test    C, #$100        WC
                        and     C, #$ff         WZ
                        subx    B, #0
                if_nz   and     B, #$ff
                if_z    and     B, #$ff         WZ
                        muxnz   F, #PF                          ' set parity flag if BC was non zero
                        jmp     #fetch
'***************************************************************************************************************
zed_cpir        {ed b1} shl     H, #8                           ' pack HL
                        or      H, L
                        shl     B, #8                           ' pack BC
                        or      B, C
.loop                   mov     ea, H
                        call    #rd_byte
                        add     H, #1                           ' HL++
                        and     H, low_word
                        sub     B, #1                           ' BC--
                        and     B, low_word     WZ
                if_nz   mov     tmp, A
                if_nz   sub     tmp, alu        WZ
                if_nz   jmp     #.loop                          ' until alu == A or BC == 0
                        mov     C, B            WZ              ' unpack BC
                        and     C, #$ff
                        shr     B, #8
                        mov     L, H                            ' unpack HL
                        and     L, #$ff
                        shr     H, #8
                        and     F, #CF                          ' keep carry flag
                        or      F, #NF                          ' always set negative flag
                if_nz   or      F, #PF                          ' set parity flag if BC <> 0
                        mov     tmp, alu
                        mov     alu, A
                        mov     aux, alu                        ' keep a copy in aux for later
                        sub     alu, tmp        WZ
                        muxz    F, #ZF                          ' set zero flag if SUB was zero
                        xor     aux, alu
                        xor     aux, tmp
                        and     aux, #HF                        ' mask the auxiliary flag
                        or      F, aux
                        test    alu, #$80       WZ
                        muxnz   F, #SF                          ' set sign flag if alu bit #7 is set
                        jmp     #fetch
'***************************************************************************************************************
zed_cpdr        {ed b9} shl     H, #8                           ' pack HL
                        or      H, L
                        shl     B, #8                           ' pack BC
                        or      B, C
.loop                   mov     ea, H
                        call    #rd_byte
                        sub     H, #1                           ' HL--
                        and     H, low_word
                        sub     B, #1                           ' BC--
                        and     B, low_word     WZ
                if_nz   mov     tmp, A
                if_nz   sub     tmp, alu        WZ
                if_nz   jmp     #.loop                          ' until alu == A or BC == 0
                        mov     C, B            WZ              ' unpack BC
                        and     C, #$ff
                        shr     B, #8
                        mov     L, H                            ' unpack HL
                        and     L, #$ff
                        shr     H, #8
                        and     F, #CF                          ' keep carry flag
                        or      F, #NF                          ' always set negative flag
                if_nz   or      F, #PF                          ' set parity flag if BC <> 0
                        mov     tmp, alu                        ' tmp = (HL)
                        mov     alu, A
                        mov     aux, alu                        ' keep a copy in aux for later
                        sub     alu, tmp        WZ
                        muxz    F, #ZF                          ' set zero flag if SUB was zero
                        xor     aux, alu
                        xor     aux, tmp
                        and     aux, #HF                        ' mask the auxiliary flag
                        or      F, aux
                        test    alu, #$80       WZ
                        muxnz   F, #SF                          ' set sign flag if alu bit #7 is set
                        jmp     #fetch
'***************************************************************************************************************
zed_ini         {ed a2} call    #bc_into_ea                     '\ port[HL] <-- alu <-- port[C/BC]
                        call    #rd_port                        '|
                        call    #hl_into_ea                      '|
                        call    #wr_byte                        '/
                        add     L, #1                           ' HL++
                        test    L, #$100        WC
                        and     L, #$ff
                        addx    H, #0
                        and     H, #$ff
                        sub     B, #1           WZ              ' B--
                        muxz    F, #ZF
                        test    B, #$80         WC
                        muxc    F, #SF
                        and     B, #$ff
                        jmp     #fetch
'***************************************************************************************************************
zed_ind         {ed aa} call    #bc_into_ea                     '\ port[HL] <-- alu <-- port[C/BC]
                        call    #rd_port                        '|                                
                        call    #hl_into_ea                      '|                                
                        call    #wr_byte                        '/                                
                        sub     L, #1                           ' HL--
                        test    L, #$100        WC
                        and     L, #$ff
                        subx    H, #0
                        and     H, #$ff
                        sub     B, #1           WZ              ' B--
                        muxz    F, #ZF
                        test    B, #$80         WC
                        muxc    F, #SF
                        and     B, #$ff
                        jmp     #fetch
'***************************************************************************************************************
zed_inir        {ed b2}
.loop                   call    #bc_into_ea                     '\ port[HL] <-- alu <-- port[C/BC] 
                        call    #rd_port                        '|                                 
                        call    #hl_into_ea                      '|                                 
                        call    #wr_byte                        '/                                 
                        add     L, #1                           ' HL++
                        test    L, #$100        WC
                        and     L, #$ff
                        addx    H, #0
                        and     H, #$ff
                        sub     B, #1                           ' B--
                        and     B, #$ff         wz
                if_nz   jmp     #.loop
                        andn    F, #SF
                        or      F, #ZF
                        jmp     #fetch
'***************************************************************************************************************
zed_indr        {ed ba}
.loop                   call    #bc_into_ea                     '\ port[HL] <-- alu <-- port[C/BC] 
                        call    #rd_port                        '|                                 
                        call    #hl_into_ea                      '|                                 
                        call    #wr_byte                        '/                                 
                        sub     L, #1                           ' HL--
                        test    L, #$100        WC
                        and     L, #$ff
                        subx    H, #0
                        and     H, #$ff
                        sub     B, #1                           ' B--
                        and     B, #$ff         wz
                if_nz   jmp     #.loop
                        andn    F, #SF
                        or      F, #ZF
                        jmp     #fetch
'***************************************************************************************************************
zed_outi        {ed a3} call    #rd_byte_hl                     '\ port[C/BC] <-- alu <-- mem[HL]
                        call    #bc_into_ea                     '|
                        call    #wr_port                        '/
                        add     L, #1                           ' HL++
                        test    L, #$100        WC
                        and     L, #$ff
                        addx    H, #0
                        and     H, #$ff
                        sub     B, #1           WZ              ' B--
                        muxz    F, #ZF
                        test    B, #$80         WC
                        muxc    F, #SF
                        and     B, #$ff
                        jmp     #fetch
'***************************************************************************************************************
zed_outd        {ed ab} call    #rd_byte_hl                     '\ port[C/BC] <-- alu <-- mem[HL]
                        call    #bc_into_ea                     '|
                        call    #wr_port                        '/
                        sub     L, #1                           ' HL--
                        test    L, #$100        WC
                        and     L, #$ff
                        subx    H, #0
                        and     H, #$ff
                        sub     B, #1           WZ              ' B--
                        muxz    F, #ZF
                        test    B, #$80         WC
                        muxc    F, #SF
                        and     B, #$ff
                        jmp     #fetch
'***************************************************************************************************************
zed_otir        {ed b3}
.loop                   call    #rd_byte_hl                     '\ port[C/BC] <-- alu <-- mem[HL] 
                        call    #bc_into_ea                     '|
                        call    #wr_port                        '/
                        add     L, #1                           ' HL++
                        test    L, #$100        WC
                        and     L, #$ff
                        addx    H, #0
                        and     H, #$ff
                        sub     B, #1                           ' B--
                        and     B, #$ff         wz
                if_nz   jmp     #.loop
                        andn    F, #SF
                        or      F, #ZF
                        jmp     #fetch
'***************************************************************************************************************
zed_otdr        {ed bb}
.loop                   call    #rd_byte_hl                     '\ port[C/BC] <-- alu <-- mem[HL] 
                        call    #bc_into_ea                     '|
                        call    #wr_port                        '/
                        sub     L, #1                           ' HL--
                        test    L, #$100        WC
                        and     L, #$ff
                        subx    H, #0
                        and     H, #$ff
                        sub     B, #1                           ' B--
                        and     B, #$ff         wz
                if_nz   jmp     #.loop
                        andn    F, #SF
                        or      F, #ZF
                        jmp     #fetch
'***************************************************************************************************************
'               OPCODES DD/FD
'***************************************************************************************************************
z_add_xy_bc  {dd/fd 09} mov     tmp, B
                        shl     tmp, #8
                        or      tmp, C
                        or      XY, ram_base                    '                    (add Z80_MEM hub base addr)
                        rdword  alu, XY
                        mov     aux, alu
                        add     alu, tmp
                        xor     aux, alu
                        xor     aux, tmp
                        shr     aux, #8                         ' get MSB to LSB
                        test    aux, #HF        WC              ' carry from bit 11 to bit 12?
                        muxc    F, #HF
                        test    aux, #$100      WC              ' carry from bit 15 to bit 16?
                        muxc    F, #CF
                        andn    F, #NF                          ' always clear negative flag
                        or      XY, ram_base                    '                    (add Z80_MEM hub base addr)???
                        wrword  alu, XY
                        jmp     #fetch
'***************************************************************************************************************
z_add_xy_de  {dd/fd 19} mov     tmp, D
                        shl     tmp, #8
                        or      tmp, E
                        or      XY, ram_base                    '                    (add Z80_MEM hub base addr)
                        rdword  alu, XY
                        mov     aux, alu
                        add     alu, tmp
                        xor     aux, alu
                        xor     aux, tmp
                        shr     aux, #8                         ' get MSB to LSB
                        test    aux, #HF        WC              ' carry from bit 11 to bit 12?
                        muxc    F, #HF
                        test    aux, #$100      WC              ' carry from bit 15 to bit 16?
                        muxc    F, #CF
                        andn    F, #NF                          ' always clear negative flag
                        or      XY, ram_base                    '                    (add Z80_MEM hub base addr)???
                        wrword  alu, XY
                        jmp     #fetch
'***************************************************************************************************************
z_add_xy_xy  {dd/fd 29} or      XY, ram_base                    '                    (add Z80_MEM hub base addr)
                        rdword  alu, XY
                        mov     tmp, alu
                        mov     aux, alu
                        add     alu, tmp
                        xor     aux, alu
                        xor     aux, tmp
                        shr     aux, #8                         ' get MSB to LSB
                        test    aux, #HF        WC              ' carry from bit 3 to bit 4?
                        muxc    F, #HF
                        test    aux, #$100      WC              ' carry from bit 15 to bit 16?
                        muxc    F, #CF
                        andn    F, #NF                          ' always clear negative flag
                        or      XY, ram_base                    '                    (add Z80_MEM hub base addr)???
                        wrword  alu, XY
                        jmp     #fetch
'***************************************************************************************************************
z_add_xy_sp  {dd/fd 39} mov     tmp, SP
                        or      XY, ram_base                    '                    (add Z80_MEM hub base addr)
                        rdword  alu, XY
                        mov     aux, alu
                        add     alu, tmp
                        xor     aux, alu
                        xor     aux, tmp
                        shr     aux, #8                         ' get MSB to LSB
                        test    aux, #HF        WC              ' carry from bit 3 to bit 4?
                        muxc    F, #HF
                        test    aux, #$100      WC              ' carry from bit 15 to bit 16?
                        muxc    F, #CF
                        andn    F, #NF                          ' always clear negative flag
                        or      XY, ram_base                    '                    (add Z80_MEM hub base addr)???
                        wrword  alu, XY
                        jmp     #fetch
'***************************************************************************************************************
z_ld_mxy_imm8 {dd/fd 36}call    #mxy                            ' compute (IX/Y+disp8)
                        mov     tmp, ea                         ' save ea
                        call    #rd_opcode                      ' fetch byte
                        mov     ea, tmp                         ' restore ea
                        call    #wr_byte                        ' write byte to mem[ea]
                        jmp     #fetch
'***************************************************************************************************************
z_ex_sp_xy   {dd/fd e3} mov     ea, SP
                        call    #rd_word
                        or      XY, ram_base                    '                    (add Z80_MEM hub base addr)
                        rdword  tmp, XY
                        xor     alu, tmp
                        xor     tmp, alu
                        xor     alu, tmp
                        wrword  tmp, XY
                        mov     ea, SP
                        or      XY, ram_base                    '                    (add Z80_MEM hub base addr)???
                        call    #wr_word
                        jmp     #fetch
'***************************************************************************************************************
'               OPCODES DD/FD CB
'***************************************************************************************************************
z_rlc_mxy               call    #alu_rlc                        {dd/fd cb 00..07}
                        call    #wr_byte
                        jmp     postop                                                                    '?????????
'***************************************************************************************************************
z_rrc_mxy               call    #alu_rrc                        {dd/fd cb 08..0f}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_rl_mxy                call    #alu_rl                         {dd/fd cb 10..17}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_rr_mxy                call    #alu_rr                         {dd/fd cb 18..1f}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_sla_mxy               call    #alu_sla                        {dd/fd cb 20..27}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_sra_mxy               call    #alu_sra                        {dd/fd cb 28..2f}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_sli_mxy               call    #alu_sli                        {dd/fd cb 30..37}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_srl_mxy               call    #alu_srl                        {dd/fd cb 38..3f}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_bit_0_mxy             mov     tmp, #bit0                      {dd/fd cb 40..7f}
                        jmp     #alu_bit
'***************************************************************************************************************
z_bit_1_mxy             mov     tmp, #bit1                      {dd/fd cb 40..7f}
                        jmp     #alu_bit
'***************************************************************************************************************
z_bit_2_mxy             mov     tmp, #bit2                      {dd/fd cb 40..7f}
                        jmp     #alu_bit
'***************************************************************************************************************
z_bit_3_mxy             mov     tmp, #bit3                      {dd/fd cb 40..7f}
                        jmp     #alu_bit
'***************************************************************************************************************
z_bit_4_mxy             mov     tmp, #bit4                      {dd/fd cb 40..7f}
                        jmp     #alu_bit
'***************************************************************************************************************
z_bit_5_mxy             mov     tmp, #bit5                      {dd/fd cb 40..7f}
                        jmp     #alu_bit
'***************************************************************************************************************
z_bit_6_mxy             mov     tmp, #bit6                      {dd/fd cb 40..7f}
                        jmp     #alu_bit
'***************************************************************************************************************
z_bit_7_mxy             mov     tmp, #bit7                      {dd/fd cb 40..7f}
                        jmp     #alu_bit
'***************************************************************************************************************
z_res_0_mxy             andn    alu, #bit0                      {dd/fd cb 80..bf}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_res_1_mxy             andn    alu, #bit1                      {dd/fd cb 80..bf}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_res_2_mxy             andn    alu, #bit2                      {dd/fd cb 80..bf}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_res_3_mxy             andn    alu, #bit3                      {dd/fd cb 80..bf}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_res_4_mxy             andn    alu, #bit4                      {dd/fd cb 80..bf}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_res_5_mxy             andn    alu, #bit5                      {dd/fd cb 80..bf}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_res_6_mxy             andn    alu, #bit6                      {dd/fd cb 80..bf}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_res_7_mxy             andn    alu, #bit7                      {dd/fd cb 80..bf}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_set_0_mxy             or      alu, #bit0                      {dd/fd cb c0..ff}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_set_1_mxy             or      alu, #bit1                      {dd/fd cb c0..ff}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_set_2_mxy             or      alu, #bit2                      {dd/fd cb c0..ff}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_set_3_mxy             or      alu, #bit3                      {dd/fd cb c0..ff}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_set_4_mxy             or      alu, #bit4                      {dd/fd cb c0..ff}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_set_5_mxy             or      alu, #bit5                      {dd/fd cb c0..ff}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_set_6_mxy             or      alu, #bit6                      {dd/fd cb c0..ff}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************
z_set_7_mxy             or      alu, #bit7                      {dd/fd cb c0..ff}
                        call    #wr_byte
                        jmp     postop
'***************************************************************************************************************

'***************************************************************************************************************
' Don't change the order of the registers unless you also
' change the order of their cog RAM counterparts and the
' break function
C_reg                   byte    0
B_reg                   byte    0
E_reg                   byte    0
D_reg                   byte    0
L_reg                   byte    0
H_reg                   byte    0
F_reg                   byte    0
A_reg                   byte    0
C2_reg                  byte    0
B2_reg                  byte    0
E2_reg                  byte    0
D2_reg                  byte    0
L2_reg                  byte    0
H2_reg                  byte    0
F2_reg                  byte    0
A2_reg                  byte    0
R_reg                   byte    0
R2_reg                  byte    0
IFF_reg                 byte    0
I_reg                   byte    0
IX_reg                  word    0
IY_reg                  word    0
SP_reg                  word    0
PC_reg                  word    0
                        byte    0,0,0,0,0,0,0,0

'***************************************************************************************************************
' This table for opcodes beginning $00-$FF...
' Routines (vectors) must be in COG or LUT RAM (ie 10-bit addresses)
opcodes_00
{00}    long    fetch                                                            ' NOP
{01}    long    imm16_to_alu    | alu_to_bc       << 12                          ' LD   BC,nnnn
{02}    long    bc_to_ea        | a_to_alu        << 12  | wr_byte_ea     << 22  ' LD   (BC),A
{03}    long    bc_to_alu       | inc16           << 12  | alu_to_bc      << 22  ' INC  BC
{04}    long    b_to_alu        | alu_inc         << 12  | alu_to_b       << 22  ' INC  B
{05}    long    b_to_alu        | alu_dec         << 12  | alu_to_b       << 22  ' DEC  B
{06}    long    imm8_to_alu     | alu_to_b        << 12                          ' LD   B,nn
{07}    long    rlca_                                                            ' RLCA

{08}    long    ex_af_af2                                                        ' EX   AF,AF'
{09}    long    add_hl_bc                                                        ' ADD  HL,BC
{0a}    long    bc_to_ea        | rd_byte_ea      << 12  | alu_to_a       << 22  ' LD   A,(BC)
{0b}    long    bc_to_alu       | dec16           << 12  | alu_to_bc      << 22  ' DEC  BC
{0c}    long    c_to_alu        | alu_inc         << 12  | alu_to_c       << 22  ' INC  C
{0d}    long    c_to_alu        | alu_dec         << 12  | alu_to_c       << 22  ' DEC  C
{0e}    long    imm8_to_alu     | alu_to_c        << 12                          ' LD   C,nn
{0f}    long    rrca_                                                            ' RRCA

{10}    long    djnz_                                                            ' DJNZ offs8
{11}    long    imm16_to_alu    | alu_to_de       << 12                          ' LD   DE,nnnn
{12}    long    de_to_ea        | a_to_alu        << 12  | wr_byte_ea     << 22  ' LD   (DE),A
{13}    long    de_to_alu       | inc16           << 12  | alu_to_de      << 22  ' INC  DE
{14}    long    d_to_alu        | alu_inc         << 12  | alu_to_d       << 22  ' INC  D
{15}    long    d_to_alu        | alu_dec         << 12  | alu_to_d       << 22  ' DEC  D
{16}    long    imm8_to_alu     | alu_to_d        << 12                          ' LD   D,nn
{17}    long    rla_                                                             ' RLA

{18}    long    jr_uncond                                                        ' JR   offs8
{19}    long    add_hl_de                                                        ' ADD  HL,DE
{1a}    long    de_to_ea        | rd_byte_ea      << 12  | alu_to_a       << 22  ' LD   A,(DE)
{1b}    long    de_to_alu       | dec16           << 12  | alu_to_de      << 22  ' DEC  DE
{1c}    long    e_to_alu        | alu_inc         << 12  | alu_to_e       << 22  ' INC  E
{1d}    long    e_to_alu        | alu_dec         << 12  | alu_to_e       << 22  ' DEC  E
{1e}    long    imm8_to_alu     | alu_to_e        << 12                          ' LD   E,nn
{1f}    long    rra_                                                             ' RRA

{20}    long    test_flag_0     | jr_cond         << 12  | ZF             << 22  ' JR   NZ,offs8
{21}    long    imm16_to_alu    | alu_to_hl       << 12                          ' LD   HL,nnnn
{22}    long    imm16_to_ea     | hl_to_alu       << 12  | wr_word_ea     << 22  ' LD   (nnnn),HL
{23}    long    hl_to_alu       | inc16           << 12  | alu_to_hl      << 22  ' INC  HL
{24}    long    h_to_alu        | alu_inc         << 12  | alu_to_h       << 22  ' INC  H
{25}    long    h_to_alu        | alu_dec         << 12  | alu_to_h       << 22  ' DEC  H
{26}    long    imm8_to_alu     | alu_to_h        << 12                          ' LD   H,nn
{27}    long    daa_                                                             ' DAA

{28}    long    test_flag_1     | jr_cond         << 12  | ZF             << 22  ' JR   Z,offs8
{29}    long    add_hl_hl                                                        ' ADD  HL,HL
{2a}    long    imm16_to_ea     | rd_word_ea      << 12  | alu_to_hl      << 22  ' LD   HL,(nnnn)
{2b}    long    hl_to_alu       | dec16           << 12  | alu_to_hl      << 22  ' DEC  HL
{2c}    long    l_to_alu        | alu_inc         << 12  | alu_to_l       << 22  ' INC  L
{2d}    long    l_to_alu        | alu_dec         << 12  | alu_to_l       << 22  ' DEC  L
{2e}    long    imm8_to_alu     | alu_to_l        << 12                          ' LD   L,nn
{2f}    long    cpl_                                                             ' CPL

{30}    long    test_flag_0     | jr_cond         << 12  | CF             << 22  ' JR   NC,offs8
{31}    long    imm16_to_alu    | alu_to_sp       << 12                          ' LD   SP,nnnn
{32}    long    imm16_to_ea     | a_to_alu        << 12  | wr_byte_ea     << 22  ' LD   (nnnn),A
{33}    long    sp_to_alu       | inc16           << 12  | alu_to_sp      << 22  ' INC  SP
{34}    long    m_to_alu        | alu_inc         << 12  | alu_to_m       << 22  ' INC  (HL)
{35}    long    m_to_alu        | alu_dec         << 12  | alu_to_m       << 22  ' DEC  (HL)
{36}    long    imm8_to_alu     | hl_to_ea        << 12  | alu_to_m       << 22  ' LD   (HL),nn
{37}    long    scf_                                                             ' SCF

{38}    long    test_flag_1     | jr_cond         << 12  | CF             << 22  ' JR   C,offs8
{39}    long    add_hl_sp                                                        ' ADD  HL,SP
{3a}    long    imm16_to_ea     | rd_byte_ea      << 12  | alu_to_a       << 22  ' LD   A,(nnnn)
{3b}    long    sp_to_alu       | dec16           << 12  | alu_to_sp      << 22  ' DEC  SP
{3c}    long    a_to_alu        | alu_inc         << 12  | alu_to_a       << 22  ' INC  A
{3d}    long    a_to_alu        | alu_dec         << 12  | alu_to_a       << 22  ' DEC  A
{3e}    long    imm8_to_alu     | alu_to_a        << 12                          ' LD   A,nn
{3f}    long    ccf_                                                             ' CCF

{40}    long    fetch                                                            ' LD   B,B
{41}    long    c_to_alu        | alu_to_b        << 12                          ' LD   B,C
{42}    long    d_to_alu        | alu_to_b        << 12                          ' LD   B,D
{43}    long    e_to_alu        | alu_to_b        << 12                          ' LD   B,E
{44}    long    h_to_alu        | alu_to_b        << 12                          ' LD   B,H
{45}    long    l_to_alu        | alu_to_b        << 12                          ' LD   B,L
{46}    long    m_to_alu        | alu_to_b        << 12                          ' LD   B,(HL)
{47}    long    a_to_alu        | alu_to_b        << 12                          ' LD   B,A

{48}    long    b_to_alu        | alu_to_c        << 12                          ' LD   C,B
{49}    long    fetch                                                            ' LD   C,C
{4a}    long    d_to_alu        | alu_to_c        << 12                          ' LD   C,D
{4b}    long    e_to_alu        | alu_to_c        << 12                          ' LD   C,E
{4c}    long    h_to_alu        | alu_to_c        << 12                          ' LD   C,H
{4d}    long    l_to_alu        | alu_to_c        << 12                          ' LD   C,L
{4e}    long    m_to_alu        | alu_to_c        << 12                          ' LD   C,(HL)
{4f}    long    a_to_alu        | alu_to_c        << 12                          ' LD   C,A

{50}    long    b_to_alu        | alu_to_d        << 12                          ' LD   D,B
{51}    long    c_to_alu        | alu_to_d        << 12                          ' LD   D,C
{52}    long    fetch                                                            ' LD   D,D
{53}    long    e_to_alu        | alu_to_d        << 12                          ' LD   D,E
{54}    long    h_to_alu        | alu_to_d        << 12                          ' LD   D,H
{55}    long    l_to_alu        | alu_to_d        << 12                          ' LD   D,L
{56}    long    m_to_alu        | alu_to_d        << 12                          ' LD   D,(HL)
{57}    long    a_to_alu        | alu_to_d        << 12                          ' LD   D,A

{58}    long    b_to_alu        | alu_to_e        << 12                          ' LD   E,B
{59}    long    c_to_alu        | alu_to_e        << 12                          ' LD   E,C
{5a}    long    d_to_alu        | alu_to_e        << 12                          ' LD   E,D
{5b}    long    fetch                                                            ' LD   E,E
{5c}    long    h_to_alu        | alu_to_e        << 12                          ' LD   E,H
{5d}    long    l_to_alu        | alu_to_e        << 12                          ' LD   E,L
{5e}    long    m_to_alu        | alu_to_e        << 12                          ' LD   E,(HL)
{5f}    long    a_to_alu        | alu_to_e        << 12                          ' LD   E,A

{60}    long    b_to_alu        | alu_to_h        << 12                          ' LD   H,B
{61}    long    c_to_alu        | alu_to_h        << 12                          ' LD   H,C
{62}    long    d_to_alu        | alu_to_h        << 12                          ' LD   H,D
{63}    long    e_to_alu        | alu_to_h        << 12                          ' LD   H,E
{64}    long    fetch                                                            ' LD   H,H
{65}    long    l_to_alu        | alu_to_h        << 12                          ' LD   H,L
{66}    long    m_to_alu        | alu_to_h        << 12                          ' LD   H,(HL)
{67}    long    a_to_alu        | alu_to_h        << 12                          ' LD   H,A

{68}    long    b_to_alu        | alu_to_l        << 12                          ' LD   L,B
{69}    long    c_to_alu        | alu_to_l        << 12                          ' LD   L,C
{6a}    long    d_to_alu        | alu_to_l        << 12                          ' LD   L,D
{6b}    long    e_to_alu        | alu_to_l        << 12                          ' LD   L,E
{6c}    long    h_to_alu        | alu_to_l        << 12                          ' LD   L,H
{6d}    long    fetch                                                            ' LD   L,L
{6e}    long    m_to_alu        | alu_to_l        << 12                          ' LD   L,(HL)
{6f}    long    a_to_alu        | alu_to_l        << 12                          ' LD   L,A

{70}    long    hl_to_ea        | b_to_alu        << 12  | alu_to_m       << 22  ' LD   (HL),B
{71}    long    hl_to_ea        | c_to_alu        << 12  | alu_to_m       << 22  ' LD   (HL),C
{72}    long    hl_to_ea        | d_to_alu        << 12  | alu_to_m       << 22  ' LD   (HL),D
{73}    long    hl_to_ea        | e_to_alu        << 12  | alu_to_m       << 22  ' LD   (HL),E
{74}    long    hl_to_ea        | h_to_alu        << 12  | alu_to_m       << 22  ' LD   (HL),H
{75}    long    hl_to_ea        | l_to_alu        << 12  | alu_to_m       << 22  ' LD   (HL),L
{76}    long    halt_                                                            ' HALT
{77}    long    hl_to_ea        | a_to_alu        << 12  | alu_to_m       << 22  ' LD   (HL),A

{78}    long    b_to_alu        | alu_to_a        << 12                          ' LD   A,B
{79}    long    c_to_alu        | alu_to_a        << 12                          ' LD   A,C
{7a}    long    d_to_alu        | alu_to_a        << 12                          ' LD   A,D
{7b}    long    e_to_alu        | alu_to_a        << 12                          ' LD   A,E
{7c}    long    h_to_alu        | alu_to_a        << 12                          ' LD   A,H
{7d}    long    l_to_alu        | alu_to_a        << 12                          ' LD   A,L
{7e}    long    m_to_alu        | alu_to_a        << 12                          ' LD   A,(HL)
{7f}    long    fetch                                                            ' LD   A,A

{80}    long    b_to_alu        | alu_add         << 12                          ' ADD  A,B
{81}    long    c_to_alu        | alu_add         << 12                          ' ADD  A,C
{82}    long    d_to_alu        | alu_add         << 12                          ' ADD  A,D
{83}    long    e_to_alu        | alu_add         << 12                          ' ADD  A,E
{84}    long    h_to_alu        | alu_add         << 12                          ' ADD  A,H
{85}    long    l_to_alu        | alu_add         << 12                          ' ADD  A,L
{86}    long    m_to_alu        | alu_add         << 12                          ' ADD  A,(HL)
{87}    long    a_to_alu        | alu_add         << 12                          ' ADD  A,A

{88}    long    b_to_alu        | alu_adc         << 12                          ' ADC  A,B
{89}    long    c_to_alu        | alu_adc         << 12                          ' ADC  A,C
{8a}    long    d_to_alu        | alu_adc         << 12                          ' ADC  A,D
{8b}    long    e_to_alu        | alu_adc         << 12                          ' ADC  A,E
{8c}    long    h_to_alu        | alu_adc         << 12                          ' ADC  A,H
{8d}    long    l_to_alu        | alu_adc         << 12                          ' ADC  A,L
{8e}    long    m_to_alu        | alu_adc         << 12                          ' ADC  A,(HL)
{8f}    long    a_to_alu        | alu_adc         << 12                          ' ADC  A,A

{90}    long    b_to_alu        | alu_sub         << 12                          ' SUB  B
{91}    long    c_to_alu        | alu_sub         << 12                          ' SUB  C
{92}    long    d_to_alu        | alu_sub         << 12                          ' SUB  D
{93}    long    e_to_alu        | alu_sub         << 12                          ' SUB  E
{94}    long    h_to_alu        | alu_sub         << 12                          ' SUB  H
{95}    long    l_to_alu        | alu_sub         << 12                          ' SUB  L
{96}    long    m_to_alu        | alu_sub         << 12                          ' SUB  (HL)
{97}    long    a_to_alu        | alu_sub         << 12                          ' SUB  A

{98}    long    b_to_alu        | alu_sbc         << 12                          ' SBC  A,B
{99}    long    c_to_alu        | alu_sbc         << 12                          ' SBC  A,C
{9a}    long    d_to_alu        | alu_sbc         << 12                          ' SBC  A,D
{9b}    long    e_to_alu        | alu_sbc         << 12                          ' SBC  A,E
{9c}    long    h_to_alu        | alu_sbc         << 12                          ' SBC  A,H
{9d}    long    l_to_alu        | alu_sbc         << 12                          ' SBC  A,L
{9e}    long    m_to_alu        | alu_sbc         << 12                          ' SBC  A,(HL)
{9f}    long    a_to_alu        | alu_sbc         << 12                          ' SBC  A,A

{a0}    long    b_to_alu        | alu_and         << 12                          ' AND  B
{a1}    long    c_to_alu        | alu_and         << 12                          ' AND  C
{a2}    long    d_to_alu        | alu_and         << 12                          ' AND  D
{a3}    long    e_to_alu        | alu_and         << 12                          ' AND  E
{a4}    long    h_to_alu        | alu_and         << 12                          ' AND  H
{a5}    long    l_to_alu        | alu_and         << 12                          ' AND  L
{a6}    long    m_to_alu        | alu_and         << 12                          ' AND  (HL)
{a7}    long    a_to_alu        | alu_and         << 12                          ' AND  A

{a8}    long    b_to_alu        | alu_xor         << 12                          ' XOR  B
{a9}    long    c_to_alu        | alu_xor         << 12                          ' XOR  C
{aa}    long    d_to_alu        | alu_xor         << 12                          ' XOR  D
{ab}    long    e_to_alu        | alu_xor         << 12                          ' XOR  E
{ac}    long    h_to_alu        | alu_xor         << 12                          ' XOR  H
{ad}    long    l_to_alu        | alu_xor         << 12                          ' XOR  L
{ae}    long    m_to_alu        | alu_xor         << 12                          ' XOR  (HL)
{af}    long    a_to_alu        | alu_xor         << 12                          ' XOR  A

{b0}    long    b_to_alu        | alu_or          << 12                          ' OR   B
{b1}    long    c_to_alu        | alu_or          << 12                          ' OR   C
{b2}    long    d_to_alu        | alu_or          << 12                          ' OR   D
{b3}    long    e_to_alu        | alu_or          << 12                          ' OR   E
{b4}    long    h_to_alu        | alu_or          << 12                          ' OR   H
{b5}    long    l_to_alu        | alu_or          << 12                          ' OR   L
{b6}    long    m_to_alu        | alu_or          << 12                          ' OR   (HL)
{b7}    long    a_to_alu        | alu_or          << 12                          ' OR   A

{b8}    long    b_to_alu        | alu_cp          << 12                          ' CP   B
{b9}    long    c_to_alu        | alu_cp          << 12                          ' CP   C
{ba}    long    d_to_alu        | alu_cp          << 12                          ' CP   D
{bb}    long    e_to_alu        | alu_cp          << 12                          ' CP   E
{bc}    long    h_to_alu        | alu_cp          << 12                          ' CP   H
{bd}    long    l_to_alu        | alu_cp          << 12                          ' CP   L
{be}    long    m_to_alu        | alu_cp          << 12                          ' CP   (HL)
{bf}    long    a_to_alu        | alu_cp          << 12                          ' CP   A

{c0}    long    test_flag_0     | ret_cond        << 12  | ZF             << 22  ' RET  NZ
{c1}    long    pop_bc                                                           ' POP  BC
{c2}    long    test_flag_0     | jp_cond         << 12  | ZF             << 22  ' JP   NZ,nnnn
{c3}    long    imm16_to_alu    | alu_to_pc       << 12                          ' JP   nnnn
{c4}    long    test_flag_0     | call_cond       << 12  | ZF             << 22  ' CALL NZ,nnnn
{c5}    long    bc_to_alu       | push_reg        << 12                          ' PUSH BC
{c6}    long    imm8_to_alu     | alu_add         << 12                          ' ADD  A,imm8
{c7}    long    rst_                                                             ' RST  00

{c8}    long    test_flag_1     | ret_cond        << 12  | ZF             << 22  ' RET  Z
{c9}    long    ret_                                                             ' RET
{ca}    long    test_flag_1     | jp_cond         << 12  | ZF             << 22  ' JP   Z,nnnn
{cb xx} long    cb_xx                                                            ' $CB xx
{cc}    long    test_flag_1     | call_cond       << 12  | ZF             << 22  ' CALL Z,nnnn
{cd}    long    imm16_to_alu    | call_uncond         << 12                      ' CALL nnnn
{ce}    long    imm8_to_alu     | alu_adc         << 12                          ' ADC  A,imm8
{cf}    long    rst_                                                             ' RST  08

{d0}    long    test_flag_0     | ret_cond        << 12  | CF             << 22  ' RET  NC
{d1}    long    pop_de                                                           ' POP  DE
{d2}    long    test_flag_0     | jp_cond         << 12  | CF             << 22  ' JP   NC,nnnn
{d3}    long    imm8_to_ea      | a_to_alu        << 12  | alu_to_port    << 22  ' OUT  (nn),A
{d4}    long    test_flag_0     | call_cond       << 12  | CF             << 22  ' CALL NC,nnnn
{d5}    long    de_to_alu       | push_reg        << 12                          ' PUSH DE
{d6}    long    imm8_to_alu     | alu_sub         << 12                          ' SUB  imm8
{d7}    long    rst_                                                             ' RST  10

{d8}    long    test_flag_1     | ret_cond        << 12  | CF             << 22  ' RET  C
{d9}    long    exx_                                                             ' EXX
{da}    long    test_flag_1     | jp_cond         << 12  | CF             << 22  ' JP   C,nnnn
{db}    long    imm8_to_ea      | port_to_alu     << 12  | alu_to_a       << 22  ' IN   A,(nn)
{dc}    long    test_flag_1     | call_cond       << 12  | CF             << 22  ' CALL C,nnnn
{dd xx} long    dd_xx                                                            ' $DD xx
{de}    long    imm8_to_alu     | alu_sbc         << 12                          ' SBC  A,imm8
{df}    long    rst_                                                             ' RST  18

{e0}    long    test_flag_0     | ret_cond        << 12  | PF             << 22  ' RET  PE
{e1}    long    pop_hl                                                           ' POP  HL
{e2}    long    test_flag_0     | jp_cond         << 12  | PF             << 22  ' JP   PE,nnnn
{e3}    long    ex_sp_hl                                                         ' EX   (SP),HL
{e4}    long    test_flag_0     | call_cond       << 12  | PF             << 22  ' CALL PE,nnnn
{e5}    long    hl_to_alu       | push_reg        << 12                          ' PUSH HL
{e6}    long    imm8_to_alu     | alu_and         << 12                          ' AND  imm8
{e7}    long    rst_                                                             ' RST  20

{e8}    long    test_flag_1     | ret_cond        << 12  | PF             << 22  ' RET  PO
{e9}    long    hl_to_alu       | alu_to_pc       << 12                          ' JP   (HL)
{ea}    long    test_flag_1     | jp_cond         << 12  | PF             << 22  ' JP   PO,nnnn
{eb}    long    ex_de_hl                                                         ' EX   DE,HL
{ec}    long    test_flag_1     | call_cond       << 12  | PF             << 22  ' CALL PO,nnnn
{ed xx} long    ed_xx                                                            ' $ED xx
{ee}    long    imm8_to_alu     | alu_xor         << 12                          ' XOR  imm8
{ef}    long    rst_                                                             ' RST  28

{f0}    long    test_flag_0     | ret_cond        << 12  | SF             << 22  ' RET  P
{f1}    long    pop_af                                                           ' POP  AF
{f2}    long    test_flag_0     | jp_cond         << 12  | SF             << 22  ' JP   P,nnnn
{f3}    long    di_                                                              ' DI
{f4}    long    test_flag_0     | call_cond       << 12  | SF             << 22  ' CALL P,nnnn
{f5}    long    af_to_alu       | push_reg        << 12                          ' PUSH AF
{f6}    long    imm8_to_alu     | alu_or          << 12                          ' OR   imm8
{f7}    long    rst_                                                             ' RST  30

{f8}    long    test_flag_1     | ret_cond        << 12  | SF             << 22  ' RET  M
{f9}    long    hl_to_alu       | alu_to_sp       << 12                          ' LD   SP,HL
{fa}    long    test_flag_1     | jp_cond         << 12  | SF             << 22  ' JP   M,nnnn
{fb}    long    ei_                                                              ' EI
{fc}    long    test_flag_1     | call_cond       << 12  | SF             << 22  ' CALL M,nnnn
{fd xx} long    fd_xx                                                            ' $FD xx
{fe}    long    imm8_to_alu     | alu_cp          << 12                          ' CP   imm8
{ff}    long    rst_                                                             ' RST  38

'***************************************************************************************************************
opcodes_xy                                           {dd/fd xx}
{dd/fd 00}    long    fetch                                                            ' NOP
{dd/fd 01}    long    imm16_to_alu    | alu_to_bc       << 12                          ' LD   BC,nnnn
{dd/fd 02}    long    bc_to_ea        | a_to_alu        << 12  | wr_byte_ea     << 22  ' LD   (BC),A
{dd/fd 03}    long    bc_to_alu       | inc16           << 12  | alu_to_bc      << 22  ' INC  BC
{dd/fd 04}    long    b_to_alu        | alu_inc         << 12  | alu_to_b       << 22  ' INC  B
{dd/fd 05}    long    b_to_alu        | alu_dec         << 12  | alu_to_b       << 22  ' DEC  B
{dd/fd 06}    long    imm8_to_alu     | alu_to_b        << 12                          ' LD   B,nn
{dd/fd 07}    long    rlca_                                                            ' RLCA

{dd/fd 08}    long    ex_af_af2                                                        ' EX   AF,AF'
{dd/fd 09}    long    z_add_xy_bc                                                      ' ADD  IX,BC
{dd/fd 0a}    long    bc_to_ea        | rd_byte_ea      << 12  | alu_to_a       << 22  ' LD   A,(BC)
{dd/fd 0b}    long    bc_to_alu       | dec16           << 12  | alu_to_bc      << 22  ' DEC  BC
{dd/fd 0c}    long    c_to_alu        | alu_inc         << 12  | alu_to_c       << 22  ' INC  C
{dd/fd 0d}    long    c_to_alu        | alu_dec         << 12  | alu_to_c       << 22  ' DEC  C
{dd/fd 0e}    long    imm8_to_alu     | alu_to_c        << 12                          ' LD   C,nn
{dd/fd 0f}    long    rrca_                                                            ' RRCA

{dd/fd 10}    long    djnz_                                                            ' DJNZ offs8
{dd/fd 11}    long    imm16_to_alu    | alu_to_de       << 12                          ' LD   DE,nnnn
{dd/fd 12}    long    de_to_ea        | a_to_alu        << 12  | wr_byte_ea     << 22  ' LD   (DE),A
{dd/fd 13}    long    de_to_alu       | inc16           << 12  | alu_to_de      << 22  ' INC  DE
{dd/fd 14}    long    d_to_alu        | alu_inc         << 12  | alu_to_d       << 22  ' INC  D
{dd/fd 15}    long    d_to_alu        | alu_dec         << 12  | alu_to_d       << 22  ' DEC  D
{dd/fd 16}    long    imm8_to_alu     | alu_to_d        << 12                          ' LD   D,nn
{dd/fd 17}    long    rla_                                                             ' RLA

{dd/fd 18}    long    jr_uncond                                                        ' JR   offs8
{dd/fd 19}    long    z_add_xy_de                                                      ' ADD  IX,DE
{dd/fd 1a}    long    de_to_ea        | rd_byte_ea      << 12  | alu_to_a       << 22  ' LD   A,(DE)
{dd/fd 1b}    long    de_to_alu       | dec16           << 12  | alu_to_de      << 22  ' DEC  DE
{dd/fd 1c}    long    e_to_alu        | alu_inc         << 12  | alu_to_e       << 22  ' INC  E
{dd/fd 1d}    long    e_to_alu        | alu_dec         << 12  | alu_to_e       << 22  ' DEC  E
{dd/fd 1e}    long    imm8_to_alu     | alu_to_e        << 12                          ' LD   E,nn
{dd/fd 1f}    long    rra_                                                             ' RRA

{dd/fd 20}    long    test_flag_0     | jr_cond         << 12  | ZF             << 22  ' JR   NZ,offs8
{dd/fd 21}    long    imm16_to_alu    | alu_to_xy       << 12                          ' LD   IX,nnnn
{dd/fd 22}    long    imm16_to_ea     | xy_to_alu       << 12  | wr_word_ea     << 22  ' LD   (nnnn),IX
{dd/fd 23}    long    xy_to_alu       | inc16           << 12  | alu_to_xy      << 22  ' INC  IX
{dd/fd 24}    long    hxy_to_alu      | alu_inc         << 12  | alu_to_hxy     << 22  ' INC  HX
{dd/fd 25}    long    hxy_to_alu      | alu_dec         << 12  | alu_to_hxy     << 22  ' DEC  HX
{dd/fd 26}    long    imm8_to_alu     | alu_to_hxy      << 12                          ' LD   HX,nn
{dd/fd 27}    long    daa_                                                             ' DAA

{dd/fd 28}    long    test_flag_1     | jr_cond         << 12  | ZF             << 22  ' JR   Z,offs8
{dd/fd 29}    long    z_add_xy_xy                                                      ' ADD  IX,IX
{dd/fd 2a}    long    imm16_to_ea     | rd_word_ea      << 12  | alu_to_xy      << 22  ' LD   IX,(nnnn)
{dd/fd 2b}    long    xy_to_alu       | dec16           << 12  | alu_to_xy      << 22  ' DEC  IX
{dd/fd 2c}    long    lxy_to_alu      | alu_inc         << 12  | alu_to_lxy     << 22  ' INC  LX
{dd/fd 2d}    long    lxy_to_alu      | alu_dec         << 12  | alu_to_lxy     << 22  ' DEC  LX
{dd/fd 2e}    long    imm8_to_alu     | alu_to_lxy      << 12                          ' LD   LX,nn
{dd/fd 2f}    long    cpl_                                                             ' CPL

{dd/fd 30}    long    test_flag_0     | jr_cond         << 12  | CF             << 22  ' JR   NC,offs8
{dd/fd 31}    long    imm16_to_alu    | alu_to_sp       << 12                          ' LD   SP,nnnn
{dd/fd 32}    long    imm16_to_ea     | a_to_alu        << 12  | wr_byte_ea     << 22  ' LD   (nnnn),A
{dd/fd 33}    long    sp_to_alu       | inc16           << 12  | alu_to_sp      << 22  ' INC  SP
{dd/fd 34}    long    mxy_to_alu      | alu_inc         << 12  | alu_to_m       << 22  ' INC  (IX+disp8)
{dd/fd 35}    long    mxy_to_alu      | alu_dec         << 12  | alu_to_m       << 22  ' DEC  (IX+disp8)
{dd/fd 36}    long    z_ld_mxy_imm8                                                    ' LD   (IX+disp8),nn
{dd/fd 37}    long    scf_                                                             ' SCF

{dd/fd 38}    long    test_flag_1     | jr_cond         << 12  | CF             << 22  ' JR   C,offs8
{dd/fd 39}    long    z_add_xy_sp                                                      ' ADD  IX,SP
{dd/fd 3a}    long    imm16_to_ea     | rd_byte_ea      << 12  | alu_to_a       << 22  ' LD   A,(nnnn)
{dd/fd 3b}    long    sp_to_alu       | dec16           << 12  | alu_to_sp      << 22  ' DEC  SP
{dd/fd 3c}    long    a_to_alu        | alu_inc         << 12  | alu_to_a       << 22  ' INC  A
{dd/fd 3d}    long    a_to_alu        | alu_dec         << 12  | alu_to_a       << 22  ' DEC  A
{dd/fd 3e}    long    imm8_to_alu     | alu_to_a        << 12                          ' LD   A,nn
{dd/fd 3f}    long    ccf_                                                             ' CCF

{dd/fd 40}    long    fetch                                                            ' LD   B,B
{dd/fd 41}    long    c_to_alu        | alu_to_b        << 12                          ' LD   B,C
{dd/fd 42}    long    d_to_alu        | alu_to_b        << 12                          ' LD   B,D
{dd/fd 43}    long    e_to_alu        | alu_to_b        << 12                          ' LD   B,E
{dd/fd 44}    long    hxy_to_alu      | alu_to_b        << 12                          ' LD   B,HX
{dd/fd 45}    long    lxy_to_alu      | alu_to_b        << 12                          ' LD   B,LX
{dd/fd 46}    long    mxy_to_alu      | alu_to_b        << 12                          ' LD   B,(IX+disp8)
{dd/fd 47}    long    a_to_alu        | alu_to_b        << 12                          ' LD   B,A

{dd/fd 48}    long    b_to_alu        | alu_to_c        << 12                          ' LD   C,B
{dd/fd 49}    long    fetch                                                            ' LD   C,C
{dd/fd 4a}    long    d_to_alu        | alu_to_c        << 12                          ' LD   C,D
{dd/fd 4b}    long    e_to_alu        | alu_to_c        << 12                          ' LD   C,E
{dd/fd 4c}    long    hxy_to_alu      | alu_to_c        << 12                          ' LD   C,HX
{dd/fd 4d}    long    lxy_to_alu      | alu_to_c        << 12                          ' LD   C,LX
{dd/fd 4e}    long    mxy_to_alu      | alu_to_c        << 12                          ' LD   C,(IX+disp8)
{dd/fd 4f}    long    a_to_alu        | alu_to_c        << 12                          ' LD   C,A

{dd/fd 50}    long    b_to_alu        | alu_to_d        << 12                          ' LD   D,B
{dd/fd 51}    long    c_to_alu        | alu_to_d        << 12                          ' LD   D,C
{dd/fd 52}    long    fetch                                                            ' LD   D,D
{dd/fd 53}    long    e_to_alu        | alu_to_d        << 12                          ' LD   D,E
{dd/fd 54}    long    hxy_to_alu      | alu_to_d        << 12                          ' LD   D,HX
{dd/fd 55}    long    lxy_to_alu      | alu_to_d        << 12                          ' LD   D,LX
{dd/fd 56}    long    mxy_to_alu      | alu_to_d        << 12                          ' LD   D,(IX+disp8)
{dd/fd 57}    long    a_to_alu        | alu_to_d        << 12                          ' LD   D,A

{dd/fd 58}    long    b_to_alu        | alu_to_e        << 12                          ' LD   E,B
{dd/fd 59}    long    c_to_alu        | alu_to_e        << 12                          ' LD   E,C
{dd/fd 5a}    long    d_to_alu        | alu_to_e        << 12                          ' LD   E,D
{dd/fd 5b}    long    fetch                                                            ' LD   E,E
{dd/fd 5c}    long    hxy_to_alu      | alu_to_e        << 12                          ' LD   E,HX
{dd/fd 5d}    long    lxy_to_alu      | alu_to_e        << 12                          ' LD   E,LX
{dd/fd 5e}    long    mxy_to_alu      | alu_to_e        << 12                          ' LD   E,(IX+disp8)
{dd/fd 5f}    long    a_to_alu        | alu_to_e        << 12                          ' LD   E,A

{dd/fd 60}    long    b_to_alu        | alu_to_hxy      << 12                          ' LD   HX,B
{dd/fd 61}    long    c_to_alu        | alu_to_hxy      << 12                          ' LD   HX,C
{dd/fd 62}    long    d_to_alu        | alu_to_hxy      << 12                          ' LD   HX,D
{dd/fd 63}    long    e_to_alu        | alu_to_hxy      << 12                          ' LD   HX,E
{dd/fd 64}    long    fetch                                                            ' LD   HX,HX
{dd/fd 45}    long    lxy_to_alu      | alu_to_hxy      << 12                          ' LD   HX,LX
{dd/fd 66}    long    mxy_to_alu      | alu_to_h        << 12                          ' LD   H,(IX+disp8)
{dd/fd 67}    long    a_to_alu        | alu_to_hxy      << 12                          ' LD   HX,A

{dd/fd 68}    long    b_to_alu        | alu_to_lxy      << 12                          ' LD   LX,B
{dd/fd 69}    long    c_to_alu        | alu_to_lxy      << 12                          ' LD   LX,C
{dd/fd 6a}    long    d_to_alu        | alu_to_lxy      << 12                          ' LD   LX,D
{dd/fd 6b}    long    e_to_alu        | alu_to_lxy      << 12                          ' LD   LX,E
{dd/fd 6c}    long    hxy_to_alu      | alu_to_lxy      << 12                          ' LD   LX,HX
{dd/fd 6d}    long    fetch                                                            ' LD   LX,LX
{dd/fd 6e}    long    mxy_to_alu      | alu_to_l        << 12                          ' LD   L,(IX+disp8)
{dd/fd 6f}    long    a_to_alu        | alu_to_lxy      << 12                          ' LD   LX,A

{dd/fd 70}    long    mxy_to_ea       | b_to_alu        << 12  | alu_to_m       << 22  ' LD   (IX+disp8),B
{dd/fd 71}    long    mxy_to_ea       | c_to_alu        << 12  | alu_to_m       << 22  ' LD   (IX+disp8),C
{dd/fd 72}    long    mxy_to_ea       | d_to_alu        << 12  | alu_to_m       << 22  ' LD   (IX+disp8),D
{dd/fd 73}    long    mxy_to_ea       | e_to_alu        << 12  | alu_to_m       << 22  ' LD   (IX+disp8),E
{dd/fd 74}    long    mxy_to_ea       | h_to_alu        << 12  | alu_to_m       << 22  ' LD   (IX+disp8),H
{dd/fd 75}    long    mxy_to_ea       | l_to_alu        << 12  | alu_to_m       << 22  ' LD   (IX+disp8),L
{dd/fd 76}    long    halt_                                                            ' HALT
{dd/fd 77}    long    mxy_to_ea       | a_to_alu        << 12  | alu_to_m       << 22  ' LD   (IX+disp8),A

{dd/fd 78}    long    b_to_alu        | alu_to_a        << 12                          ' LD   A,B
{dd/fd 79}    long    c_to_alu        | alu_to_a        << 12                          ' LD   A,C
{dd/fd 7a}    long    d_to_alu        | alu_to_a        << 12                          ' LD   A,D
{dd/fd 7b}    long    e_to_alu        | alu_to_a        << 12                          ' LD   A,E
{dd/fd 7c}    long    hxy_to_alu      | alu_to_a        << 12                          ' LD   A,HX
{dd/fd 7d}    long    lxy_to_alu      | alu_to_a        << 12                          ' LD   A,LX
{dd/fd 7e}    long    mxy_to_alu      | alu_to_a        << 12                          ' LD   A,(IX+disp8)
{dd/fd 7f}    long    fetch                                                            ' LD   A,A

{dd/fd 80}    long    b_to_alu        | alu_add         << 12                          ' ADD  A,B
{dd/fd 81}    long    c_to_alu        | alu_add         << 12                          ' ADD  A,C
{dd/fd 82}    long    d_to_alu        | alu_add         << 12                          ' ADD  A,D
{dd/fd 83}    long    e_to_alu        | alu_add         << 12                          ' ADD  A,E
{dd/fd 84}    long    hxy_to_alu      | alu_add         << 12                          ' ADD  A,HX
{dd/fd 85}    long    lxy_to_alu      | alu_add         << 12                          ' ADD  A,HX
{dd/fd 86}    long    mxy_to_alu      | alu_add         << 12                          ' ADD  A,(IX+disp8)
{dd/fd 87}    long    a_to_alu        | alu_add         << 12                          ' ADD  A,A

{dd/fd 88}    long    b_to_alu        | alu_adc         << 12                          ' ADC  A,B
{dd/fd 89}    long    c_to_alu        | alu_adc         << 12                          ' ADC  A,C
{dd/fd 8a}    long    d_to_alu        | alu_adc         << 12                          ' ADC  A,D
{dd/fd 8b}    long    e_to_alu        | alu_adc         << 12                          ' ADC  A,E
{dd/fd 8c}    long    hxy_to_alu      | alu_adc         << 12                          ' ADC  A,HX
{dd/fd 8d}    long    lxy_to_alu      | alu_adc         << 12                          ' ADC  A,LX
{dd/fd 8e}    long    mxy_to_alu      | alu_adc         << 12                          ' ADC  A,(IX+disp8)
{dd/fd 8f}    long    a_to_alu        | alu_adc         << 12                          ' ADC  A,A

{dd/fd 90}    long    b_to_alu        | alu_sub         << 12                          ' SUB  B
{dd/fd 91}    long    c_to_alu        | alu_sub         << 12                          ' SUB  C
{dd/fd 92}    long    d_to_alu        | alu_sub         << 12                          ' SUB  D
{dd/fd 93}    long    e_to_alu        | alu_sub         << 12                          ' SUB  E
{dd/fd 94}    long    hxy_to_alu      | alu_sub         << 12                          ' SUB  HX
{dd/fd 95}    long    lxy_to_alu      | alu_sub         << 12                          ' SUB  LX
{dd/fd 96}    long    mxy_to_alu      | alu_sub         << 12                          ' SUB  (IX+disp8)
{dd/fd 97}    long    a_to_alu        | alu_sub         << 12                          ' SUB  A

{dd/fd 98}    long    b_to_alu        | alu_sbc         << 12                          ' SBC  A,B
{dd/fd 99}    long    c_to_alu        | alu_sbc         << 12                          ' SBC  A,C
{dd/fd 9a}    long    d_to_alu        | alu_sbc         << 12                          ' SBC  A,D
{dd/fd 9b}    long    e_to_alu        | alu_sbc         << 12                          ' SBC  A,E
{dd/fd 9c}    long    hxy_to_alu      | alu_sbc         << 12                          ' SBC  A,HX
{dd/fd 9d}    long    lxy_to_alu      | alu_sbc         << 12                          ' SBC  A,LX
{dd/fd 9e}    long    mxy_to_alu      | alu_sbc         << 12                          ' SBC  A,(IX+disp8)
{dd/fd 9f}    long    a_to_alu        | alu_sbc         << 12                          ' SBC  A,A

{dd/fd a0}    long    b_to_alu        | alu_and         << 12                          ' AND  B
{dd/fd a1}    long    c_to_alu        | alu_and         << 12                          ' AND  C
{dd/fd a2}    long    d_to_alu        | alu_and         << 12                          ' AND  D
{dd/fd a3}    long    e_to_alu        | alu_and         << 12                          ' AND  E
{dd/fd a4}    long    hxy_to_alu      | alu_and         << 12                          ' AND  HX
{dd/fd a5}    long    lxy_to_alu      | alu_and         << 12                          ' AND  LX
{dd/fd a6}    long    mxy_to_alu      | alu_and         << 12                          ' AND  (IX+disp8)
{dd/fd a7}    long    a_to_alu        | alu_and         << 12                          ' AND  A

{dd/fd a8}    long    b_to_alu        | alu_xor         << 12                          ' XOR  B
{dd/fd a9}    long    c_to_alu        | alu_xor         << 12                          ' XOR  C
{dd/fd aa}    long    d_to_alu        | alu_xor         << 12                          ' XOR  D
{dd/fd ab}    long    e_to_alu        | alu_xor         << 12                          ' XOR  E
{dd/fd ac}    long    hxy_to_alu      | alu_xor         << 12                          ' XOR  HX
{dd/fd ad}    long    lxy_to_alu      | alu_xor         << 12                          ' XOR  LX
{dd/fd ae}    long    mxy_to_alu      | alu_xor         << 12                          ' XOR  (IX+disp8)
{dd/fd af}    long    a_to_alu        | alu_xor         << 12                          ' XOR  A

{dd/fd b0}    long    b_to_alu        | alu_or          << 12                          ' OR   B
{dd/fd b1}    long    c_to_alu        | alu_or          << 12                          ' OR   C
{dd/fd b2}    long    d_to_alu        | alu_or          << 12                          ' OR   D
{dd/fd b3}    long    e_to_alu        | alu_or          << 12                          ' OR   E
{dd/fd b4}    long    hxy_to_alu      | alu_or          << 12                          ' OR   HX
{dd/fd b5}    long    lxy_to_alu      | alu_or          << 12                          ' OR   LX
{dd/fd b6}    long    mxy_to_alu      | alu_or          << 12                          ' OR   (IX+disp8)
{dd/fd b7}    long    a_to_alu        | alu_or          << 12                          ' OR   A

{dd/fd b8}    long    b_to_alu        | alu_cp          << 12                          ' CP   B
{dd/fd b9}    long    c_to_alu        | alu_cp          << 12                          ' CP   C
{dd/fd ba}    long    d_to_alu        | alu_cp          << 12                          ' CP   D
{dd/fd bb}    long    e_to_alu        | alu_cp          << 12                          ' CP   E
{dd/fd bc}    long    hxy_to_alu      | alu_cp          << 12                          ' CP   HX
{dd/fd bd}    long    lxy_to_alu      | alu_cp          << 12                          ' CP   LX
{dd/fd be}    long    mxy_to_alu      | alu_cp          << 12                          ' CP   (IX+disp8)
{dd/fd bf}    long    a_to_alu        | alu_cp          << 12                          ' CP   A

{dd/fd c0}    long    test_flag_0     | ret_cond        << 12  | ZF             << 22  ' RET  NZ
{dd/fd c1}    long    pop_bc                                                           ' POP  BC
{dd/fd c2}    long    test_flag_0     | jp_cond         << 12  | ZF             << 22  ' JP   NZ,nnnn
{dd/fd c3}    long    imm16_to_alu    | alu_to_pc       << 12                          ' JP   nnnn
{dd/fd c4}    long    test_flag_0     | call_cond       << 12  | ZF             << 22  ' CALL NZ,nnnn
{dd/fd c5}    long    bc_to_alu       | push_reg        << 12                          ' PUSH BC
{dd/fd c6}    long    imm8_to_alu     | alu_add         << 12                          ' ADD  A,imm8
{dd/fd c7}    long    rst_                                                             ' RST  00

{dd/fd c8}    long    test_flag_1     | ret_cond        << 12  | ZF             << 22  ' RET  Z
{dd/fd c9}    long    ret_                                                             ' RET
{dd/fd ca}    long    test_flag_1     | jp_cond         << 12  | ZF             << 22  ' JP   Z,nnnn
{dd/fd cb}    long    cb_xx                                                            ' $CB xx
{dd/fd cc}    long    test_flag_1     | call_cond       << 12  | ZF             << 22  ' CALL Z,nnnn
{dd/fd cd}    long    imm16_to_alu    | call_uncond     << 12                          ' CALL nnnn
{dd/fd ce}    long    imm8_to_alu     | alu_adc         << 12                          ' ADC  A,imm8
{dd/fd cf}    long    rst_                                                             ' RST  08

{dd/fd d0}    long    test_flag_0     | ret_cond        << 12  | CF             << 22  ' RET  NC
{dd/fd c1}    long    pop_de                                                           ' POP  DE
{dd/fd d2}    long    test_flag_0     | jp_cond         << 12  | CF             << 22  ' JP   NC,nnnn
{dd/fd d3}    long    imm8_to_ea      | a_to_alu        << 12  | alu_to_port    << 22  ' OUT  (nn),A
{dd/fd d4}    long    test_flag_0     | call_cond       << 12  | CF             << 22  ' CALL NC,nnnn
{dd/fd d5}    long    de_to_alu       | push_reg        << 12                          ' PUSH DE
{dd/fd d6}    long    imm8_to_alu     | alu_sub         << 12                          ' SUB  imm8
{dd/fd d7}    long    rst_                                                             ' RST  10

{dd/fd d8}    long    test_flag_1     | ret_cond        << 12  | CF             << 22  ' RET  C
{dd/fd d9}    long    exx_                                                             ' EXX
{dd/fd da}    long    test_flag_1     | jp_cond         << 12  | CF             << 22  ' JP   C,nnnn
{dd/fd db}    long    imm8_to_ea      | port_to_alu     << 12  | alu_to_a       << 22  ' IN   A,(nn)
{dd/fd dc}    long    test_flag_1     | call_cond       << 12  | CF             << 22  ' CALL C,nnnn
{dd/fd dd}    long    dd_xx                                                            ' $DD xx
{dd/fd de}    long    imm8_to_alu     | alu_sbc         << 12                          ' SBC  A,imm8
{dd/fd df}    long    rst_                                                             ' RST  18

{dd/fd e0}    long    test_flag_0     | ret_cond        << 12  | PF             << 22  ' RET  PE
{dd/fd e1}    long    pop_xy                                                           ' POP  IX/IY
{dd/fd e2}    long    test_flag_0     | jp_cond         << 12  | PF             << 22  ' JP   PE,nnnn
{dd/fd e3}    long    z_ex_sp_xy                                                       ' EX   (SP),IX/IY
{dd/fd e4}    long    test_flag_0     | call_cond       << 12  | PF             << 22  ' CALL PE,nnnn
{dd/fd e5}    long    xy_to_alu       | push_reg        << 12                          ' PUSH IX/IY
{dd/fd e6}    long    imm8_to_alu     | alu_and         << 12                          ' AND  imm8
{dd/fd e7}    long    rst_                                                             ' RST  20

{dd/fd e8}    long    test_flag_1     | ret_cond        << 12  | PF             << 22  ' RET  PO
{dd/fd e9}    long    xy_to_alu       | alu_to_pc       << 12                          ' JP   (IX/IY)
{dd/fd ea}    long    test_flag_1     | jp_cond         << 12  | PF             << 22  ' JP   PO,nnnn
{dd/fd eb}    long    ex_de_hl                                                         ' EX   DE,HL
{dd/fd ec}    long    test_flag_1     | call_cond       << 12  | PF             << 22  ' CALL PO,nnnn
{dd/fd ed}    long    ed_xx                                                            ' $ED xx
{dd/fd ee}    long    imm8_to_alu     | alu_xor         << 12                          ' XOR  imm8
{dd/fd ef}    long    rst_                                                             ' RST  28

{dd/fd f0}    long    test_flag_0     | ret_cond        << 12  | SF             << 22  ' RET  P
{dd/fd f1}    long    pop_af                                                           ' POP  AF
{dd/fd f2}    long    test_flag_0     | jp_cond         << 12  | SF             << 22  ' JP   P,nnnn
{dd/fd f3}    long    di_                                                              ' DI
{dd/fd f4}    long    test_flag_0     | call_cond       << 12  | SF             << 22  ' CALL P,nnnn
{dd/fd f5}    long    af_to_alu       | push_reg        << 12                          ' PUSH AF
{dd/fd f6}    long    imm8_to_alu     | alu_or          << 12                          ' OR   imm8
{dd/fd f7}    long    rst_                                                             ' RST  30

{dd/fd f8}    long    test_flag_1     | ret_cond        << 12  | SF             << 22  ' RET  M
{dd/fd f9}    long    xy_to_alu       | alu_to_sp       << 12                          ' LD   SP,IX/IY
{dd/fd fa}    long    test_flag_1     | jp_cond         << 12  | SF             << 22  ' JP   M,nnnn
{dd/fd fb}    long    ei_                                                              ' EI
{dd/fd fc}    long    test_flag_1     | call_cond       << 12  | SF             << 22  ' CALL M,nnnn
{dd/fd fd}    long    fd_xx                                                            ' $FD xx
{dd/fd fe}    long    imm8_to_alu     | alu_cp          << 12                          ' CP   imm8
{dd/fd ff}    long    rst_                                                             ' RST  38

'***************************************************************************************************************
opcodes_cb                                           {cb xx}
{cb 00}    word    cb_rlc_b                             ' RLC  B
{cb 01}    word    cb_rlc_c                             ' RLC  C
{cb 02}    word    cb_rlc_d                             ' RLC  D
{cb 03}    word    cb_rlc_e                             ' RLC  E
{cb 04}    word    cb_rlc_h                             ' RLC  H
{cb 05}    word    cb_rlc_l                             ' RLC  L
{cb 06}    word    cb_rlc_m                             ' RLC  (HL)
{cb 07}    word    cb_rlc_a                             ' RLC  A

{cb 08}    word    cb_rrc_b                             ' RRC  B
{cb 09}    word    cb_rrc_c                             ' RRC  C
{cb 0a}    word    cb_rrc_d                             ' RRC  D
{cb 0b}    word    cb_rrc_e                             ' RRC  E
{cb 0c}    word    cb_rrc_h                             ' RRC  H
{cb 0d}    word    cb_rrc_l                             ' RRC  L
{cb 0e}    word    cb_rrc_m                             ' RRC  (HL)
{cb 0f}    word    cb_rrc_a                             ' RRC  A

{cb 10}    word    cb_rl_b                              ' RL   B
{cb 11}    word    cb_rl_c                              ' RL   C
{cb 12}    word    cb_rl_d                              ' RL   D
{cb 13}    word    cb_rl_e                              ' RL   E
{cb 14}    word    cb_rl_h                              ' RL   H
{cb 15}    word    cb_rl_l                              ' RL   L
{cb 16}    word    cb_rl_m                              ' RL   (HL)
{cb 17}    word    cb_rl_a                              ' RL   A

{cb 18}    word    cb_rr_b                              ' RR   B
{cb 19}    word    cb_rr_c                              ' RR   C
{cb 1a}    word    cb_rr_d                              ' RR   D
{cb 1b}    word    cb_rr_e                              ' RR   E
{cb 1c}    word    cb_rr_h                              ' RR   H
{cb 1d}    word    cb_rr_l                              ' RR   L
{cb 1e}    word    cb_rr_m                              ' RR   (HL)
{cb 1f}    word    cb_rr_a                              ' RR   A

{cb 20}    word    cb_sla_b                             ' SLA  B
{cb 21}    word    cb_sla_c                             ' SLA  C
{cb 22}    word    cb_sla_d                             ' SLA  D
{cb 23}    word    cb_sla_e                             ' SLA  E
{cb 24}    word    cb_sla_h                             ' SLA  H
{cb 25}    word    cb_sla_l                             ' SLA  L
{cb 26}    word    cb_sla_m                             ' SLA  (HL)
{cb 27}    word    cb_sla_a                             ' SLA  A

{cb 28}    word    cb_sra_b                             ' SRA  B
{cb 29}    word    cb_sra_c                             ' SRA  C
{cb 2a}    word    cb_sra_d                             ' SRA  D
{cb 2b}    word    cb_sra_e                             ' SRA  E
{cb 2c}    word    cb_sra_h                             ' SRA  H
{cb 2d}    word    cb_sra_l                             ' SRA  L
{cb 2e}    word    cb_sra_m                             ' SRA  (HL)
{cb 2f}    word    cb_sra_a                             ' SRA  A

{cb 30}    word    cb_sli_b                             ' SLI  B
{cb 31}    word    cb_sli_c                             ' SLI  C
{cb 32}    word    cb_sli_d                             ' SLI  D
{cb 33}    word    cb_sli_e                             ' SLI  E
{cb 34}    word    cb_sli_h                             ' SLI  H
{cb 35}    word    cb_sli_l                             ' SLI  L
{cb 36}    word    cb_sli_m                             ' SLI  (HL)
{cb 37}    word    cb_sli_a                             ' SLI  A

{cb 38}    word    cb_srl_b                             ' SRL  B
{cb 39}    word    cb_srl_c                             ' SRL  C
{cb 3a}    word    cb_srl_d                             ' SRL  D
{cb 3b}    word    cb_srl_e                             ' SRL  E
{cb 3c}    word    cb_srl_h                             ' SRL  H
{cb 3d}    word    cb_srl_l                             ' SRL  L
{cb 3e}    word    cb_srl_m                             ' SRL  (HL)
{cb 3f}    word    cb_srl_a                             ' SRL  A

{cb 40}    word    cb_bit_n_b                           ' BIT  n,B
{cb 41}    word    cb_bit_n_c                           ' BIT  n,C
{cb 42}    word    cb_bit_n_d                           ' BIT  n,D
{cb 43}    word    cb_bit_n_e                           ' BIT  n,E
{cb 44}    word    cb_bit_n_h                           ' BIT  n,H
{cb 45}    word    cb_bit_n_l                           ' BIT  n,L
{cb 46}    word    cb_bit_n_m                           ' BIT  n,(HL)
{cb 47}    word    cb_bit_n_a                           ' BIT  n,A

{cb 80}    word    cb_res_n_b                           ' RES  n,B
{cb 81}    word    cb_res_n_c                           ' RES  n,C
{cb 82}    word    cb_res_n_d                           ' RES  n,D
{cb 83}    word    cb_res_n_e                           ' RES  n,E
{cb 84}    word    cb_res_n_h                           ' RES  n,H
{cb 85}    word    cb_res_n_l                           ' RES  n,L
{cb 86}    word    cb_res_n_m                           ' RES  n,(HL)
{cb 87}    word    cb_res_n_a                           ' RES  n,A

{cb c0}    word    cb_set_n_b                           ' SET  n,B
{cb c1}    word    cb_set_n_c                           ' SET  n,C
{cb c2}    word    cb_set_n_d                           ' SET  n,D
{cb c3}    word    cb_set_n_e                           ' SET  n,E
{cb c4}    word    cb_set_n_h                           ' SET  n,H
{cb c5}    word    cb_set_n_l                           ' SET  n,L
{cb c6}    word    cb_set_n_m                           ' SET  n,(HL)
{cb c7}    word    cb_set_n_a                           ' SET  n,A

'***************************************************************************************************************
opcodes_ed                                           {ed xx} 
{ed 40}    word    ed_in_b_bc                           ' IN   B,(C)
{ed 41}    word    ed_out_bc_b                          ' OUT  (C),B
{ed 42}    word    ed_sbc_hl_bc                         ' SBC  HL,BC
{ed 43}    word    zed_ld_abs16_bc                      ' LD   (abs16),BC
{ed 44}    word    zed_neg                              ' NEG
{ed 45}    word    zed_retn                             ' RETN
{ed 46}    word    zed_im_0                             ' IM   0
{ed 47}    word    zed_ld_i_a                           ' LD   I,A

{ed 48}    word    ed_in_c_bc                           ' IN   C,(C)
{ed 49}    word    ed_out_bc_c                          ' OUT  (C),C
{ed 4a}    word    zed_adc_hl_bc                        ' ADC  HL,BC
{ed 4b}    word    zed_ld_bc_abs16                      ' LD   BC,(abs16)
{ed 4c}    word    zed_neg              ' undocumented  ' NEG
{ed 4d}    word    zed_reti                             ' RETI
{ed 4e}    word    zed_im_0             ' undocumented  ' IM   0
{ed 4f}    word    zed_ld_r_a                           ' LD   R,A

{ed 50}    word    ed_in_d_bc                           ' IN   D,(C)
{ed 51}    word    ed_out_bc_d                          ' OUT  (C),D
{ed 52}    word    ed_sbc_hl_de                         ' SBC  HL,DE
{ed 53}    word    zed_ld_abs16_de                      ' LD   (abs16),DE
{ed 54}    word    zed_neg              ' undocumented  ' NEG
{ed 55}    word    zed_retn             ' undocumented  ' RETN
{ed 56}    word    zed_im_1                             ' IM   1
{ed 57}    word    zed_ld_a_i                           ' LD   A,I

{ed 58}    word    ed_in_e_bc                           ' IN   E,(C)
{ed 59}    word    ed_out_bc_e                          ' OUT  (C),E
{ed 5a}    word    zed_adc_hl_de                        ' ADC  HL,DE
{ed 5b}    word    zed_ld_de_abs16                      ' LD   DE,(abs16)
{ed 5c}    word    zed_neg              ' undocumented  ' NEG
{ed 5d}    word    zed_reti             ' undocumented  ' RETI
{ed 5e}    word    zed_im_2                             ' IM   2
{ed 5f}    word    zed_ld_a_r                           ' LD   A,R

{ed 60}    word    ed_in_h_bc                           ' IN   H,(C)
{ed 61}    word    ed_out_bc_h                          ' OUT  (C),H
{ed 62}    word    ed_sbc_hl_hl                         ' SBC  HL,HL
{ed 63}    word    zed_ld_abs16_hl_2                    ' LD   (abs16),HL
{ed 64}    word    zed_neg              ' undocumented  ' NEG
{ed 65}    word    zed_retn             ' undocumented  ' RETN
{ed 66}    word    zed_im_0             ' undocumented  ' IM   0
{ed 67}    word    zed_rrd_m                            ' RRD  (HL)

{ed 68}    word    ed_in_l_bc                           ' IN   L,(C)
{ed 69}    word    ed_out_bc_l                          ' OUT  (C),L
{ed 6a}    word    zed_adc_hl_hl                        ' ADC  HL,HL
{ed 6b}    word    zed_ld_hl_abs16_2                    ' LD   HL,(abs16)
{ed 6c}    word    zed_neg              ' undocumented  ' NEG
{ed 6d}    word    zed_reti             ' undocumented  ' RETI
{ed 6e}    word    zed_im_0             ' undocumented  ' IM   0
{ed 6f}    word    zed_rld_m                            ' RLD  (HL)

{ed 70}    word    ed_in_0_bc           ' undocumented  ' IN   (C)
{ed 71}    word    ed_out_bc_0          ' undocumented  ' OUT  (C),0
{ed 72}    word    zed_sbc_hl_sp                        ' SBC  HL,SP
{ed 73}    word    zed_ld_abs16_sp                      ' LD   (abs16),SP
{ed 74}    word    zed_neg              ' undocumented  ' NEG
{ed 75}    word    zed_retn             ' undocumented  ' RETN
{ed 76}    word    zed_im_1             ' undocumented  ' IM   1
{ed 77}    word    break                ' undocumented  ' invalid

{ed 78}    word    ed_in_a_bc                           ' IN   A,(C)
{ed 79}    word    ed_out_bc_a                          ' OUT  (C),A
{ed 7a}    word    zed_adc_hl_sp                        ' ADC  HL,SP
{ed 7b}    word    zed_ld_sp_abs16                      ' LD   SP,(abs16)
{ed 7c}    word    zed_neg              ' undocumented  ' NEG
{ed 7d}    word    zed_reti             ' undocumented  ' RETI
{ed 7e}    word    zed_im_2             ' undocumented  ' IM   2
{ed 7f}    word    break                ' undocumented  ' invalid

{ed 80}    word    zed_bcde_cnt         ' fake opcode   ' BCDE = CNT
{ed 81}    word    break
{ed 82}    word    break
{ed 83}    word    break
{ed 84}    word    break
{ed 85}    word    break
{ed 86}    word    break
{ed 87}    word    break

{ed 88}    word    break
{ed 89}    word    break
{ed 8a}    word    break
{ed 8b}    word    break
{ed 8c}    word    break
{ed 8d}    word    break
{ed 8e}    word    break
{ed 8f}    word    break

{ed 90}    word    break
{ed 91}    word    break
{ed 92}    word    break
{ed 93}    word    break
{ed 94}    word    break
{ed 95}    word    break
{ed 96}    word    break
{ed 97}    word    break

{ed 98}    word    break
{ed 99}    word    break
{ed 9a}    word    break
{ed 9b}    word    break
{ed 9c}    word    break
{ed 9d}    word    break
{ed 9e}    word    break
{ed 9f}    word    break

{ed a0}    word    zed_ldi                              ' LDI
{ed a1}    word    zed_cpi                              ' CPI
{ed a2}    word    zed_ini                              ' INI
{ed a3}    word    zed_outi                             ' OUTI
{ed a4}    word    break
{ed a5}    word    break
{ed a6}    word    break
{ed a7}    word    break

{ed a8}    word    zed_ldd                              ' LDD
{ed a9}    word    zed_cpd                              ' CPD
{ed aa}    word    zed_ind                              ' IND
{ed ab}    word    zed_outd                             ' OUTD
{ed ac}    word    break
{ed ad}    word    break
{ed ae}    word    break
{ed af}    word    break

{ed b0}    word    zed_ldir                             ' LDIR
{ed b1}    word    zed_cpir                             ' CPIR
{ed b2}    word    zed_inir                             ' INIR
{ed b3}    word    zed_otir                             ' OTIR
{ed b4}    word    break
{ed b5}    word    break
{ed b6}    word    break
{ed b7}    word    break

{ed b8}    word    zed_lddr                             ' LDDR
{ed b9}    word    zed_cpdr                             ' CPDR
{ed ba}    word    zed_indr                             ' INDR
{ed bb}    word    zed_otdr                             ' OTDR
{ed bc}    word    break
{ed bd}    word    break
{ed be}    word    break
{ed bf}    word    break

'***************************************************************************************************************
opcodes_xy_cb                                        {dd/fd cb disp8 xx}
{dd/fd cb 00-07} word    z_rlc_mxy
{dd/fd cb 08-0f} word    z_rrc_mxy
{dd/fd cb 10-17} word    z_rl_mxy
{dd/fd cb 18-1f} word    z_rr_mxy
{dd/fd cb 20-27} word    z_sla_mxy
{dd/fd cb 28-2f} word    z_sra_mxy
{dd/fd cb 30-37} word    z_sli_mxy
{dd/fd cb 38-3f} word    z_srl_mxy
{dd/fd cb 40-47} word    z_bit_0_mxy
{dd/fd cb 48-4f} word    z_bit_1_mxy
{dd/fd cb 50-57} word    z_bit_2_mxy
{dd/fd cb 58-5f} word    z_bit_3_mxy
{dd/fd cb 60-67} word    z_bit_4_mxy
{dd/fd cb 68-6f} word    z_bit_5_mxy
{dd/fd cb 70-77} word    z_bit_6_mxy
{dd/fd cb 78-7f} word    z_bit_7_mxy
{dd/fd cb 80-87} word    z_res_0_mxy
{dd/fd cb 88-8f} word    z_res_1_mxy
{dd/fd cb 90-97} word    z_res_2_mxy
{dd/fd cb 98-9f} word    z_res_3_mxy
{dd/fd cb a0-a7} word    z_res_4_mxy
{dd/fd cb a8-af} word    z_res_5_mxy
{dd/fd cb b0-b7} word    z_res_6_mxy
{dd/fd cb b8-bf} word    z_res_7_mxy
{dd/fd cb c0-c7} word    z_set_0_mxy
{dd/fd cb c8-cf} word    z_set_1_mxy
{dd/fd cb d0-d7} word    z_set_2_mxy
{dd/fd cb d8-df} word    z_set_3_mxy
{dd/fd cb e0-e7} word    z_set_4_mxy
{dd/fd cb e8-ef} word    z_set_5_mxy
{dd/fd cb f0-f7} word    z_set_6_mxy
{dd/fd cb f8-ff} word    z_set_7_mxy

'***************************************************************************************************************
' Table of functions to execute after the DD/FD CB <disp8> xx opcodes
' The "invalid" opcodes 0-5 and 7 of each set of 8 opcodes actually execute the same
' as the #6 slot, i.e. the rotate, shift, bit clear or bit set in (IX/IY+disp8), but the
' result is also transferred into one of the 8 bit registers afterwards.
' This is not true for the BIT opcodes, though.
postop_table                                         {dd/fd cb disp8 xx ...}
{dd/fd cb <disp8> xx} word    alu_to_b
{dd/fd cb <disp8> xx} word    alu_to_c
{dd/fd cb <disp8> xx} word    alu_to_d
{dd/fd cb <disp8> xx} word    alu_to_e
{dd/fd cb <disp8> xx} word    alu_to_h
{dd/fd cb <disp8> xx} word    alu_to_l
{dd/fd cb <disp8> xx} word    fetch
{dd/fd cb <disp8> xx} word    alu_to_a
'***************************************************************************************************************

'###############################################################################################################
'###############################################################################################################
'###############################################################################################################
'###############################################################################################################
'###############################################################################################################
'###############################################################################################################

'===============================================================================================================
' COG code...
'===============================================================================================================
                        orgh    $2000
                        org     0
'***************************************************************************************************************
cog_code
entry
'                        call    #_debug
'                        call    #_debug_head
                        call    #_debug_head
' ----------------------------------------------------------------------------------------------
          mov     _spare_, #60                                  ' number of instructions to execute
fetch
          djnz    _spare_, #gofetch
.loop     call    #_HubMonitor
          jmp     #.loop
'***************************************************************************************************************
gofetch
          call    #_debug_regs
                        call    #rd_opcode                      ' fetch opcode and dispatch through table_00
                        mov     opcode, alu
                        shl     alu, #2
                        add     alu, table_00
fetch_1                 rdlong  vector3, alu
          call    #_debug_opc
                        mov     vector, vector3                 '\ extract first vector (12-bits)
                        and     vector, maskFFF                 '/   and mask to 12-bits
                        call    vector                          ' dispatch
' ----------------------------------------------------------------------------------------------
                        mov     vector, vector3                 ' 
                        shr     vector, #12                     '\ shift to 2nd vector (10-bits)
                        and     vector, mask3FF                 '/   and mask to 10-bits
                        call    vector                          ' dispatch
' ----------------------------------------------------------------------------------------------
                        shr     vector3, #22                    ' shift to 3rd vector (10-bits)
                        call    vector3                         ' dispatch
' ----------------------------------------------------------------------------------------------
                        jmp     #fetch                          ' should never return here!!!
'***************************************************************************************************************

'***************************************************************************************************************
'       Memory access functions (Z80 mem)
'***************************************************************************************************************
rd_byte_hl              mov     ea, H                           ' alu <-- mem[HL]    (read byte)                   
                        shl     ea, #8
                        or      ea, L
                        jmp     #rd_byte
'***************************************************************************************************************
rd_opcode               mov     ea, PC                          ' alu <-- mem[PC++]  (read byte)
                        add     PC, #1
                        and     PC, low_word
' ----------------------------------------------------------------------------------------------
rd_byte                 or      ea, ram_base                    '                    (add Z80_MEM hub base addr)
              _RET_     rdbyte  alu, ea                         ' alu <-- mem[ea]    (read byte)
'***************************************************************************************************************
rd_opword               mov     ea, PC                          ' alu <-- mem[PC++]  (read word)
                        add     PC, #2
                        and     PC, low_word                    ' mask any overflow
                        jmp     #rd_word
'***************************************************************************************************************
pop_alu                 mov     ea, SP                          ' alu <-- mem[SP++]  (pop word)
                        add     SP, #2
                        and     SP, low_word                    ' mask any overflow
' ----------------------------------------------------------------------------------------------
rd_word                 call    #rd_byte                        ' alu <-- mem[ea++]  (read word) - clobbers t1
                        mov     t1, alu
                        add     ea, #1
                        and     ea, low_word                    ' mask any overflow
                        call    #rd_byte
                        shl     alu, #8
              _RET_     or      alu, t1
'***************************************************************************************************************
wr_byte                 or      ea, ram_base                    '                    (add Z80-mem hub base addr)
              _RET_     wrbyte  alu, ea                         ' mem[ea] <-- alu    (write byte)
'***************************************************************************************************************

'***************************************************************************************************************
call_cond               call    #rd_opword                      ' CALL if the condition is true
                if_z    jmp     #fetch                          ' j if false, else fall thru to CALL
' ----------------------------------------------------------------------------------------------
rst_                    mov     alu, opcode                     ' Call a restart vector
                        and     alu, #$38
' ----------------------------------------------------------------------------------------------
call_uncond             xor     alu, PC                         ' alu <--> PC
                        xor     PC, alu
                        xor     alu, PC
' ----------------------------------------------------------------------------------------------
push_reg                call    #push_alu                       ' mem[SP++] <-- alu  (push alu & j to fetch)
                        jmp     #fetch
'***************************************************************************************************************
push_alu                sub     SP, #2                          ' mem[SP++] <-- alu  (push word onto stack)
                        and     SP, low_word                    ' mask any overflow
                        mov     ea, SP
' ----------------------------------------------------------------------------------------------
wr_word                 call    #wr_byte                        ' mem[ea] <-- alu    (write alu word to mem[ea])
                        add     ea, #1
                        and     ea, low_word                    ' mask any overflow
                        shr     alu, #8
                        call    #wr_byte
                        ret
'***************************************************************************************************************
{{
wr_port                 mov     t1, #io_cmd_out                 ' Port[ea] <-- alu   (write alu byte to Port[ea]) 
                        shl     t1, #16
                        or      t1, ea
                        shl     t1, #8
                        or      t1, alu
                        wrlong  t1, io_command
.wait                   rdlong  t1, io_command                  '...and wait for it to be completed.
                        test    t1, topbyte     WZ
                if_nz   jmp     #.wait
                        ret
}}
wr_port                 mov     t1, #io_cmd_out                 ' Port[ea] <-- alu   (write alu byte to Port[ea]) 
                        shl     t1, #16
                        or      t1, ea
                        shl     t1, #8
                        or      t1, alu

                        call    #_debug_wr_port

                        mov     t1, #0                          ' signal done
                        ret
'***************************************************************************************************************
rd_port                 mov     t1, #io_cmd_in                  ' alu <-- Port{ea]   (read Port[ea] byte into alu)
                        shl     t1, #16
                        or      t1, ea
                        shl     t1, #8

rd_port_cmd             wrlong  t1, io_command
.wait                   rdlong  t1, io_command                  '...and wait for it to be completed.
                        test    t1, topbyte     WZ
                if_nz   jmp     #.wait
                        mov     alu, t1
              _RET_     and     alu, #$ff
'***************************************************************************************************************
mxy                     call    #rd_opcode                      ' ea  = IX/IY+disp8
                        shl     alu, #24
                        sar     alu, #24
                        or      XY, ram_base                    '                    (add Z80_MEM hub base addr)
                        rdword  ea, XY
                        add     ea, alu
              _RET_     and     ea, low_word
'***************************************************************************************************************
imm8_to_alu             call    #rd_opcode                      ' alu <-- mem8[PC++]   (immediate)
                        ret
'***************************************************************************************************************
imm16_to_alu            call    #rd_opword                      ' alu <-- mem16[PC++]  (immediate)
                        ret
'***************************************************************************************************************
dec16                   sub     alu, #2                         ' Decrement 16 bit register pair
inc16                   add     alu, #1                         ' Increment 16 bit register pair
              _RET_     and     alu, low_word                   ' mask any overflow
'***************************************************************************************************************
b_to_alu      _RET_     mov     alu, B                          ' alu <-- B
'***************************************************************************************************************
c_to_alu      _RET_     mov     alu, C                          ' alu <-- C
'***************************************************************************************************************
d_to_alu      _RET_     mov     alu, D                          ' alu <-- D
'***************************************************************************************************************
e_to_alu      _RET_     mov     alu, E                          ' alu <-- E
'***************************************************************************************************************
h_to_alu      _RET_     mov     alu, H                          ' alu <-- H
'***************************************************************************************************************
l_to_alu      _RET_     mov     alu, L                          ' alu <-- L
'***************************************************************************************************************
m_to_alu                call    #rd_byte_hl                     ' alu <-- HL
                        ret
'***************************************************************************************************************
a_to_alu      _RET_     mov     alu, A                          ' alu <-- A
'***************************************************************************************************************
bc_to_alu               mov     alu, B                          ' alu <-- BC
                        shl     alu, #8
              _RET_     or      alu, C
'***************************************************************************************************************
de_to_alu               mov     alu, D                          ' alu <-- DE
                        shl     alu, #8
              _RET_     or      alu, E
'***************************************************************************************************************
hl_to_alu               mov     alu, H                          ' alu <-- HL
                        shl     alu, #8
              _RET_     or      alu, L
'***************************************************************************************************************
af_to_alu               mov     alu, A                          ' alu <-- AF
                        shl     alu, #8
              _RET_     or      alu, F
'***************************************************************************************************************
sp_to_alu     _RET_     mov     alu, SP                         ' alu <-- SP
'***************************************************************************************************************
xy_to_alu               or      XY, ram_base                    '                    (add Z80_MEM hub base addr)
              _RET_     rdword  alu, XY                         ' alu <-- IX/IY
'***************************************************************************************************************
hxy_to_alu              or      HXY, ram_base                   '                    (add Z80_MEM hub base addr)
              _RET_     rdbyte  alu, HXY                        ' alu <-- HX/HY
'***************************************************************************************************************
lxy_to_alu              or      LXY, ram_base                   '                    (add Z80_MEM hub base addr)
              _RET_     rdbyte  alu, LXY                        ' alu <-- LX/LY
'***************************************************************************************************************
mxy_to_alu              call    #mxy                            ' alu <-- IX/IY+disp8
                        call    #rd_byte
                        ret
'***************************************************************************************************************
port_to_alu             call    #rd_port                        ' alu <-- Port[ea]
                        ret
'***************************************************************************************************************
imm8_to_ea              call    #rd_opcode                      ' ea  <-- immediate mem[PC++]
              _RET_     mov     ea, alu
'***************************************************************************************************************
imm16_to_ea             call    #rd_opword                      ' ea  <-- mem16[PC++]  (immediate)
              _RET_     mov     ea, alu
'***************************************************************************************************************
bc_to_ea                mov     ea, B                           ' alu <-- BC
                        shl     ea, #8
              _RET_     or      ea, C
'***************************************************************************************************************
de_to_ea                mov     ea, D                           ' alu <-- DE
                        shl     ea, #8
              _RET_     or      ea, E
'***************************************************************************************************************
hl_to_ea                mov     ea, H                           ' alu <-- HL
                        shl     ea, #8
              _RET_     or      ea, L
'***************************************************************************************************************
mxy_to_ea               call    #mxy                            ' ea  <-- IX/IY+disp8  (transfer address)
                        ret
'***************************************************************************************************************
rd_byte_ea              call    #rd_byte                        ' alu <-- mem[ea]
                        ret
'***************************************************************************************************************
rd_word_ea              call    #rd_word                        ' alu <-- mem16[ea++]
                        ret
'***************************************************************************************************************
test_flag_0             mov     tmp, vector3                    ' Test a flag vector[31..22] in the F register
                        shr     tmp, #22
              _RET_     andn    tmp, F          WZ
'***************************************************************************************************************
test_flag_1             mov     tmp, vector3                    ' Test a flag vector[31..22] in the F register
                        shr     tmp, #22
              _RET_     and     tmp, F          WZ
'***************************************************************************************************************

'***************************************************************************************************************
pop_bc                  call    #pop_alu                        ' alu <-- mem[SP++]
' ----------------------------------------------------------------------------------------------
alu_to_bc               mov     C, alu                          ' BC  <-- alu
                        and     C, #$ff
                        shr     alu, #8
' ----------------------------------------------------------------------------------------------
alu_to_b                mov     B, alu                          ' B   <-- alu
                        jmp     #fetch
'***************************************************************************************************************
alu_to_c                mov     C, alu                          ' C   <-- alu
                        jmp     #fetch
'***************************************************************************************************************
pop_de                  call    #pop_alu                        ' alu <-- mem[SP++]
' ----------------------------------------------------------------------------------------------
alu_to_de               mov     E, alu                          ' DE  <-- alu
                        and     E, #$ff
                        shr     alu, #8
' ----------------------------------------------------------------------------------------------
alu_to_d                mov     D, alu                          ' D   <-- alu
                        jmp     #fetch
'***************************************************************************************************************
alu_to_e                mov     E, alu                          ' E   <-- alu
                        jmp     #fetch
'***************************************************************************************************************
pop_hl                  call    #pop_alu                        ' alu <-- mem[SP++]    (Pop HL off stack)
' ----------------------------------------------------------------------------------------------
alu_to_hl               mov     L, alu                          ' HL  <-- alu
                        and     L, #$ff
                        shr     alu, #8
' ----------------------------------------------------------------------------------------------
alu_to_h                mov     H, alu                          ' H   <-- alu
                        jmp     #fetch
'***************************************************************************************************************
alu_to_l                mov     L, alu                          ' L   <-- alu
                        jmp     #fetch
'***************************************************************************************************************
alu_to_m                call    #wr_byte                        ' mem[HL] <-- alu
                        jmp     #fetch
'***************************************************************************************************************
pop_af                  call    #pop_alu                        ' alu <-- mem[SP++]
' ----------------------------------------------------------------------------------------------
alu_to_af               mov     F, alu                          ' AF  <-- alu
                        and     F, #$ff
                        shr     alu, #8
' ----------------------------------------------------------------------------------------------
alu_to_a                mov     A, alu                          ' A   <-- alu
                        jmp     #fetch
'***************************************************************************************************************
alu_to_hxy              or      HXY, ram_base                   '                    (add Z80_MEM hub base addr)
                        wrbyte  alu, HXY                        ' HX/HY <-- alu
                        jmp     #fetch
'***************************************************************************************************************
alu_to_lxy              or      LXY, ram_base                   '                    (add Z80_MEM hub base addr)
                        wrbyte  alu, LXY                        ' LX/LY <-- alu
                        jmp     #fetch
'***************************************************************************************************************
alu_to_port             call    #wr_port                        ' Port[ea] <-- alu
                        jmp     #fetch
'***************************************************************************************************************
alu_to_sp               mov     SP, alu                         ' SP  <-- alu
                        jmp     #fetch
'***************************************************************************************************************
ret_cond      if_z      jmp     #fetch                          ' j if false(Z)
' ----------------------------------------------------------------------------------------------
ret_                    call    #pop_alu                        ' alu <-- mem[SP++]
' ----------------------------------------------------------------------------------------------
alu_to_pc               mov     PC, alu                         ' PC  <-- alu
                        jmp     #fetch
'***************************************************************************************************************
pop_xy                  call    #pop_alu                        ' alu <-- mem[SP++]
' ----------------------------------------------------------------------------------------------
alu_to_xy               or      XY, ram_base                    '                    (add Z80_MEM hub base addr)
                        wrword  alu, XY                         ' mem[IX/IY] <-- alu
                        jmp     #fetch                                      
'***************************************************************************************************************
wr_byte_ea              call    #wr_byte                        ' mem[ea] <-- alu 
                        jmp     #fetch
'***************************************************************************************************************
wr_word_ea              call    #wr_word                        ' mem16[ea++] <-- alu
                        jmp     #fetch
'***************************************************************************************************************
jr_uncond               call    #rd_opcode                      ' Jump relative
                        jmp     #jr_xx
'***************************************************************************************************************
djnz_                   sub     B, #1                           ' Decrement B and jump relative if non-zero
                        and     B, #$ff         WZ
' ----------------------------------------------------------------------------------------------
jr_cond                 call    #rd_opcode                      ' JR if condition = true(NZ)
                if_z    jmp     #fetch
' ----------------------------------------------------------------------------------------------
jr_xx                   shl     alu, #24                        ' Jump relative 
                        sar     alu, #24
                        add     PC, alu
                        and     PC, low_word
                        jmp     #fetch
'***************************************************************************************************************
jp_cond                 call    #rd_opword                      ' JP if condition = true(NZ)
                if_nz   mov     PC, alu
                        jmp     #fetch
'***************************************************************************************************************
' Compute an 8 bit decrement and set the flags
alu_dec                 sets    szhv_flags_cmp1, #$0f           ' compare auxiliary value
                        sets    szhv_flags_cmp2, #$7f           ' compare overflow value
                        or      F, #NF                          ' set negative flag
                        sub     alu, #1                         ' decrement alu
                        jmp     #szhv_flags
'***************************************************************************************************************
' Compute an 8 bit increment and set the flags
alu_inc                 sets    szhv_flags_cmp1, #$00           ' compare auxiliary value
                        sets    szhv_flags_cmp2, #$80           ' compare overflow value
                        andn    F, #NF                          ' reset negative flag
                        add     alu, #1
szhv_flags              mov     aux, alu
                        and     aux, #$0f
szhv_flags_cmp1         cmp     aux, #$00/$0f   WZ              ' carry from bit 3 to 4?
                        muxz    F, #HF                          ' set or clear half carry
szhv_flags_cmp2         cmp     alu, #$80/$7f   WZ              ' overflow to other sign?
                        muxz    F, #VF                          ' set or clear overflow
                        test    alu, #$80       WC              ' bit #7 set?
                        muxc    F, #SF                          ' set or clear sign
                        and     alu, #$ff       WZ              ' result is zero?
              _RET_     muxz    F, #ZF                          ' set or clear zero
'***************************************************************************************************************
' Compute sign, zero, auxiliary, overflow and carry flags after an addition or subtraction
szhvc_flags             xor     aux, tmp                        ' aux = result ^ operand ^ argument
                        xor     aux, alu
                        test    aux, #HF        WC              ' H flag change?
                        muxc    F, #HF
                        test    aux, #$100      WC
                        muxc    F, #(CF | VF)
                        test    aux, #$80       WC
                if_c    xor     F, #VF
                        and     alu, #$ff       WZ
                        muxz    F, #ZF
                        test    alu, #$80       WC
              _RET_     muxc    F, #SF
'***************************************************************************************************************
' Compute logic/shift/rotate and the flags
' alu:     [7][6][5][4][3][2][1][0]  carry: [c]
' before:   h  g  f  e  d  c  b  a           i
' rlc:      g  f  e  d  c  b  a  h           h     rotate left
' rrc:      a  h  g  f  e  d  c  b           a     rotate right
' rl:       g  f  e  d  c  b  a  i           h     rotate left  thru carry
' rr:       i  h  g  f  e  d  c  b           a     rotate right thru carry
' sla:      g  f  e  d  c  b  a  0           h     shift  left  arithmetic
' sra:      h  h  g  f  e  d  c  b           a     shift  right arithmetic
' sli:      g  f  e  d  c  b  a  1           h     shift  left  inverted
' sri:      0  h  g  f  e  d  c  b           a     shift  right logical
'***************************************************************************************************************
alu_rlc                 shl     alu, #1                         '                               (rotate left)
                        test    alu, #$100      WC              ' get bit #7 into zero flag
                        muxc    alu, #$01                       ' get carry in bit #0
                        jmp     #szp_flags_ff
'***************************************************************************************************************
alu_rrc                 shr     alu, #1         WC              '                               (rotate right)
                        muxc    alu, #$80                       ' set bit #7 from carry
                        jmp     #szp_flags_ff
'***************************************************************************************************************
alu_rl                  test    F, #CF          WC              ' get current C flag in carry   (rotate left  thru carry)
                        rcl     alu, #1                         ' alu in bits 8..1, carry in bit 0
                        test    alu, #$100      WC              ' test new carry
                        jmp     #szp_flags_ff
'***************************************************************************************************************
alu_rr                  test    F, #CF          WZ              ' get current C flag in carry   (rotate right thru carry)
                        shr     alu, #1         WC              ' alu in bits 6..0, bit #0 in carry
                        muxnz   alu, #$80                       ' set bit #7 from former carry
                        jmp     #szp_flags_ff
'***************************************************************************************************************
alu_sla                 shl     alu, #1                         ' alu in bits 8..1              (shift  left  arithmetic)
                        test    alu, #$100      WC              ' test former bit #7
                        jmp     #szp_flags_ff
'***************************************************************************************************************
alu_sra                 test    alu, #$01       WC              '                               (shift  right arithmetic)
                        shl     alu, #24                        ' alu in bits 31..24
                        sar     alu, #25                        ' alu in bits 6..0, bit #0 in carry
                        jmp     #szp_flags_ff
'***************************************************************************************************************
alu_sli                 shl     alu, #1                         ' alu in bits 31..25            (shift  left  inverted)
                        test    alu, #$100      WC              ' test former bit #7
                        or      alu, #$01                       ' set bit #0
                        jmp     #szp_flags_ff
'***************************************************************************************************************
alu_srl                 shr     alu, #1         WC              ' alu in bits 6..0              (shift  right logical)
' ----------------------------------------------------------------------------------------------
' Compute sign, zero and parity flags after logic/shift/rotate instruction
szp_flags_ff            mov     F, #0                           ' clear all flags
                        muxc    F, #CF                          ' set C flag on carry
                        and     alu, #$ff       WZ, WC          ' get zero and parity flags
szp_flags               muxz    F, #ZF                          ' if zero set Z flag
                        muxnc   F, #PF                          ' if even parity set the P flag
                        test    alu, #$80       WC              ' get sign in prop's carry flag
              _RET_     muxc    F, #SF                          ' set S flag
'***************************************************************************************************************
alu_add                 andn    F, #CF                          ' clear carry                   (Add tmp to accumulator)
' ----------------------------------------------------------------------------------------------
alu_adc                 test    F, #CF          WC              ' get the C flag into carry     (Add tmp+carry to accumulator)
                        mov     tmp, alu                        ' 2nd operand to tmp
                        mov     F, #0                           ' clear all flags
                        mov     alu, A
                        mov     aux, A
                        addx    alu, tmp                        ' add the tmp value with carry
                        call    #szhvc_flags
                        jmp     #alu_to_a
'***************************************************************************************************************
alu_sub                 andn    F, #CF                          ' clear carry                   (Sub tmp from accumulator)
' ----------------------------------------------------------------------------------------------
alu_sbc                 test    F, #CF          WC              ' get the C flag into carry     (Add tmp+carry from accum.)
                        mov     tmp, alu                        ' 2nd operand to tmp                                      
                        mov     F, #NF                          ' clear all flags but the N flag
                        mov     alu, A
                        mov     aux, A
                        subx    alu, tmp                        ' subtract the tmp value with carry
                        call    #szhvc_flags
                        jmp     #alu_to_a
'***************************************************************************************************************
alu_and                 mov     tmp, alu                        ' 2nd operand to tmp            (Logical AND accum. with tmp)
                        mov     F, #HF                          ' clear all flags but H flag
                        mov     alu, A
                        and     alu, tmp        WZ, WC          ' AND with the tmp value
                        call    #szp_flags
                        jmp     #alu_to_a
'***************************************************************************************************************
' Note: removing the bits in alu from A will turn the following XOR into an OR
alu_or                  andn    A, alu                          '                               (Logical OR accum. with tmp)
' ----------------------------------------------------------------------------------------------
alu_xor                 mov     tmp, alu                        ' 2nd operand to tmp            (Logical XOR accum. with tmp)
                        mov     F, #0                           ' clear all flags
                        mov     alu, A
                        xor     alu, tmp        WZ, WC          ' xor with the tmp value
                        call    #szp_flags
                        jmp     #alu_to_a
'***************************************************************************************************************
alu_cp                  mov     tmp, alu                        ' 2nd operand to tmp            (Compare accum. with tmp)
                        mov     F, #NF                          ' clear all flags but the N flag
                        mov     alu, A
                        mov     aux, A
                        sub     alu, tmp                        ' subtract the tmp value
                        call    #szhvc_flags 
                        jmp     #fetch
'***************************************************************************************************************
alu_bit                 and     alu, tmp        WZ              ' mask bit                     (Compute bit test flags)
                        muxz    F, #ZF                          ' set Z flag from ~bit
                        test    alu, #$80       WC              ' test sign bit
                        muxc    F, #SF                          ' set S flag depending on carry
                        andn    F, #NF                          ' clear N flag
                        or      F, #HF                          ' set H flag
                        jmp     #fetch
'***************************************************************************************************************
rlca_         {07}      shl     A, #1
                        test    A, #$100        WC              ' check carry
                        and     A, #$ff                         ' keep bits 7..0
                        muxc    A, #$01                         ' set low bit from carry
                        and     F, #(SF | ZF | PF)              ' keep S, Z and P flags
                        muxc    F, #CF                          ' set C flag from carry
                        jmp     #fetch
'***************************************************************************************************************
ex_af_af2     {08}      mov     ea, regptr
                        add     ea, #(@F2_reg - @C_reg)         ' offset to F2, A2               '????????
                        rdbyte  tmp, ea                         ' fetch F2
                        wrbyte  F, ea                           ' write F
                        mov     F, tmp
                        add     ea, #1
                        rdbyte  tmp, ea                         ' fetch A2
                        wrbyte  A, ea                           ' write A
                        mov     A, tmp
                        jmp     #fetch
'***************************************************************************************************************
rrca_         {0f}      shr     A, #1           WC              ' rotate right and check carry
                        muxc    A, #$80                         ' set bit #7 from carry flag
                        and     F, #(SF | ZF | PF)              ' keep S, Z and P flags
                        muxc    F, #CF                          ' set C from carry flag
                        jmp     #fetch
'***************************************************************************************************************
rla_          {17}      shl     A, #1
                        test    A, #$100        WZ              ' test bit #7
                        and     A, #$ff                         ' keep bits 7..0
                        test    F, #CF          WC              ' get C flag to carry
                        muxnz   F, #CF                          ' set C from zero flag
                        muxc    A, #$01                         ' set bit #0 from previous C flag
                        and     F, #(SF | ZF | PF | CF)         ' keep S, Z, P and C flags
                        jmp     #fetch
'***************************************************************************************************************
rra_          {1f}      shr     A, #1           WC              ' rotate right and check carry
                        test    F, #CF          WZ              ' get C flag
                        muxc    F, #CF                          ' set C flag from carry flag
                        muxnz   A, #$80                         ' set bit #7 from zero flag
                        and     F, #(SF | ZF | PF | CF)         ' keep S, Z, P and C flags
                        jmp     #fetch
'***************************************************************************************************************
daa_          {27}      mov     alu, A
                        mov     tmp, alu
                        mov     aux, alu                        ' save for auxiliary flag test
                        and     tmp, #15                        ' isolate low nibble of accumulator
                        mov     daa_adj, #$06                   ' +$06
                        test    F, #NF          WZ              ' test negative flag
                        negnz   daa_adj, daa_adj                ' -$06
                        test    F, #HF          WZ
                        cmp     tmp, #$09+1     WC              ' aux isn't set; is nibble > $9 ?
        if_nc_or_nz     add     A, daa_adj                      ' aux was set or nibble is > $9 => add $06 or -$06
                        shl     daa_adj, #4                     ' +$60 or -$60
                        test    F, #CF          WZ
                        cmp     alu, #$99+1     WC              ' carry isn't set; is data > $99 ?
        if_nc_or_nz     add     A, daa_adj                      ' carry was set or data is > $99 => add $60 or -$60
                        and     F, #(NF | CF)                   ' preserve old CF and NF
                        cmp     alu, #$99+1     WC
        if_nc           or      F, #CF                          ' set carry if result > $99
                        xor     aux, A
                        and     aux, #HF        WZ              ' if bit #4 of result changed
                        or      F, aux                          ' set the auxiliary carry flag
                        and     A, #$ff         WZ, WC          ' get zero and parity flags
                        muxz    F, #ZF                          ' set Z on zero
                        muxnc   F, #PF                          ' clear P on parity
                        test    A, #$80         WZ              ' get sign bit #7
                        muxnz   F, #SF                          ' set S on sign
                        jmp     #fetch
'***************************************************************************************************************
cpl_          {2f}      xor     A, #$ff                          ' xor a (ones complement)
                        or      F, #(HF | NF)                   ' set the H and N flags
                        jmp     #fetch
'***************************************************************************************************************
scf_          {37}      or      F, #CF                          ' set the C flag
                        andn    F, #(HF | NF)                   ' clear the H and N flags
                        jmp     #fetch
'***************************************************************************************************************
ccf_          {3f}      test    F, #CF          WC              ' get the C flag into the carry
                        muxc    F, #HF                          ' set the H flag if C was set
                        xor     F, #CF                          ' toggle the C flag
                        andn    F, #NF                          ' clear N flag
                        jmp     #fetch
'***************************************************************************************************************
halt_         {76}      jmp     #break
'***************************************************************************************************************
ex_sp_hl      {e3}      mov     tmp, H
                        shl     tmp, #8
                        or      tmp, L
                        mov     ea, SP
                        call    #rd_word
                        xor     alu, tmp
                        xor     tmp, alu
                        xor     alu, tmp
                        mov     ea, SP
                        call    #wr_word
                        mov     L, tmp
                        and     L, #$ff
                        mov     H, tmp
                        shr     H, #8
                        jmp     #fetch
'***************************************************************************************************************
ex_de_hl      {eb}      xor     L, E
                        xor     E, L
                        xor     L, E
                        xor     H, D
                        xor     D, H
                        xor     H, D
                        jmp     #fetch
'***************************************************************************************************************
di_           {f3}      andn    IFF, #(IFF1 | IFF2)
                        jmp     #fetch
'***************************************************************************************************************
ei_           {fb}      or      IFF, #(IFF1 | IFF2)
                        jmp     #fetch
'***************************************************************************************************************
io_command              long    0-0                             ' io command address
io_break                long    io_cmd_break << 24 | C_reg << 8

table_00                long    opcodes_00
table_cb                long    opcodes_cb
table_ed                long    opcodes_ed
table_xy                long    opcodes_xy                      ' {dd/fd xx}
table_xy_cb             long    opcodes_xy_cb                   ' {dd/fd cb disp8 xx}
postop_ptr              long    postop_table

regptr                  long    C_reg                           ' C register address
XY                      long                                    ' either IX or IY register address
LXY                     long    0                               ' dito
HXY                     long    0                               ' dito + 1
low_word                long    $0000ffff

C                       long    0
B                       long    0
E                       long    0
D                       long    0
L                       long    0
H                       long    0
F                       long    ZF                              ' reset value is zero flag set
A                       long    0
R                       long    0                               ' refresh register
R2                      long    0                               ' bit #7 of last write to R
IFF                     long    IFF1 | IFF2                     ' interrupt flip-flops (enabled)
SP                      long    0                               ' stack pointer
PC                      long    0                               ' program counter

vector3                 long    0
mask3FF                 long    $3FF                            ' mask 10-bit address
maskFFF                 long    $FFF                            ' mask 12-bit address
topbyte                 long    $FF00_0000                      ' mask top byte
ram_base                long    Z80_MEM                         ' hub locn of 64KB Z80 memory
'***************************************************************************************************************
                        fit     $1e0
                        
                        org     $1e0
                        long    0[16]           ' allow 16 longs for lmm variables (for the rom monitor code) 
'***************************************************************************************************************
                        fit     $1f0
                        org     $1f0
opcode                  long    0                               ' most recent opcode
alu                     long    0                               ' arithmetic logic unit (sort of :)
ea                                                              '\ effective address
daa_adj                 long    0                               '/ adjustment value for DAA
tmp                     long    0                               ' second operand temp
t1                                                              '\ temporary 1
aux                     long    0                               '/ auxiliary flag temp
postop                                                          '\ operation after DD/FD CB disp8 xy
vector                  long    0                               '/ opcode vector
lmm_pc                  long    0                               ' LMM program counter
_spare_                 long    0                               ' -spare-
                        fit     $1F8
''  ---------------------------------------------------------------------------------     
''  IJMP3  = $1F0  ' INT3  interrupt call   address     
''  IRET3  = $1F1  ' INT3  interrupt return address     
''  IJMP2  = $1F2  ' INT2  interrupt call   address     
''  IRET2  = $1F3  ' INT2  interrupt return address     
''  IJMP1  = $1F4  ' INT1  interrupt call   address     
''  IRET1  = $1F5  ' INT1  interrupt return address     
''  PA     = $1F6  ' CALLD/CALLPA/LOC                   
''  PB     = $1F7  ' CALLD/CALLPB/LOC                   
''  PTRA   = $1F8  ' pointer A to hub RAM               
''  PTRB   = $1F9  ' pointer B to hub RAM               
''  DIRA   = $1FA  ' P31..P0  Output Enables            
''  DIRB   = $1FB  ' P63..P32 Output Enables            
''  OUTA   = $1FC  ' P31..P0  Output States             
''  OUTB   = $1FD  ' P63..P32 Output States             
''  INA    = $1FE  ' P31..P0  Input  States  *  (debug interrupt call   address)
''  INB    = $1FF  ' P63..P32 Input  States  ** (debug interrupt return address)
''  ---------------------------------------------------------------------------------

'***************************************************************************************************************
                        fit     $200
                        
'===============================================================================================================
' LUT code...
'===============================================================================================================
                        org     $200                            ' LUT
'***************************************************************************************************************
lut_code
'***************************************************************************************************************
add_hl_bc     {09}      mov     t1, C
                        mov     tmp, B
                        andn    F, #NF
                        add     L, t1
                        test    L, #$100        WC
                        and     L, #$ff
                        mov     aux, H
                        addx    H, tmp
                        test    H, #$100        WC
                        muxc    F, #CF
                        and     H, #$ff
                        xor     aux, H
                        xor     aux, tmp
                        test    aux, #HF        WC
                        muxc    F, #HF
                        jmp     #fetch
'***************************************************************************************************************
add_hl_de     {19}      mov     t1, E
                        mov     tmp, D
                        andn    F, #NF
                        add     L, t1
                        test    L, #$100        WC
                        and     L, #$ff
                        mov     aux, H
                        addx    H, tmp
                        test    H, #$100        WC
                        muxc    F, #CF
                        and     H, #$ff
                        xor     aux, H
                        xor     aux, tmp
                        test    aux, #HF        WC
                        muxc    F, #HF
                        jmp     #fetch
'***************************************************************************************************************
add_hl_hl     {29}      mov     t1, L
                        mov     tmp, H
                        andn    F, #NF
                        add     L, t1
                        test    L, #$100        WC
                        and     L, #$ff
                        mov     aux, H
                        addx    H, tmp
                        test    H, #$100        WC
                        muxc    F, #CF
                        and     H, #$ff
                        xor     aux, H
                        xor     aux, tmp
                        test    aux, #HF        WC
                        muxc    F, #HF
                        jmp     #fetch
'***************************************************************************************************************
add_hl_sp     {39}      mov     tmp, SP
                        shr     tmp, #8
                        mov     t1, SP
                        and     t1, #$ff
                        andn    F, #NF
                        add     L, t1
                        test    L, #$100        WC
                        and     L, #$ff
                        mov     aux, H
                        addx    H, tmp
                        test    H, #$100        WC
                        muxc    F, #CF
                        and     H, #$ff
                        xor     aux, H
                        xor     aux, tmp
                        test    aux, #HF        WC
                        muxc    F, #HF
                        jmp     #fetch
'***************************************************************************************************************
exx_          {d9}      mov     ea, regptr

                        add     ea, #(@C2_reg - @C_reg)         ' offset to C2                   '?????????
                        rdbyte  alu, ea
                        wrbyte  C, ea
                        mov     C, alu

                        add     ea, #1
                        rdbyte  alu, ea
                        wrbyte  B, ea
                        mov     B, alu

                        add     ea, #1
                        rdbyte  alu, ea
                        wrbyte  E, ea
                        mov     E, alu

                        add     ea, #1
                        rdbyte  alu, ea
                        wrbyte  D, ea
                        mov     D, alu

                        add     ea, #1
                        rdbyte  alu, ea
                        wrbyte  L, ea
                        mov     L, alu

                        add     ea, #1
                        rdbyte  alu, ea
                        wrbyte  H, ea
                        mov     H, alu
                        jmp     #fetch
'***************************************************************************************************************
cb_xx         {cb}      call    #rd_opcode
                        mov     opcode, alu                     ' preserve opcode
                        shl     alu, #1
                        add     alu, table_cb
                        test    opcode, #%11000000      WZ      ' first quarter?  $00-3F
        if_z            rdword  lmm_pc, alu                     '\ yes, dispatch now
        if_z            jmp     #lmm_pc                         '/
                        mov     alu, opcode
                        and     alu, #7
                        test    opcode, #%01000000      WZ
                        test    opcode, #%10000000      WC      
                        add     alu, #$40
        if_c            add     alu, #$08                       ' if $8x
        if_c_and_nz     add     alu, #$08                       ' if $Cx
                        shl     alu, #1
                        add     alu, table_cb
                        shr     opcode, #3                      ' bit number to 2..0            '??????????
                        and     opcode, #7                      ' mask bit number
                        mov     tmp, #1
                        shl     tmp, opcode                     ' make bit mask for BIT/RES/SET
                        rdword  lmm_pc, alu                     '\ dispatch now
                        jmp     #lmm_pc                         '/
'***************************************************************************************************************
dd_xx         {dd}      mov     XY, regptr
                        add     XY, #(@IX_reg - @C_reg)                                          '??????????????????
                        mov     HXY, LXY
                        add     HXY, #1
                        call    #rd_opcode
                        mov     opcode, alu
                        cmp     alu, #$cb               WZ
                if_z    jmp     #xy_cb                          ' dd+cb                 
                        shl     alu, #2
                        add     alu, table_xy
                        jmp     #fetch_1
'***************************************************************************************************************
ed_xx         {ed}      call    #rd_opcode
                        cmp     alu, #$40               WZ, WC
                if_b    jmp     #fetch
                        cmp     alu, #$c0               WZ, WC
                if_ae   jmp     #fetch
                        sub     alu, #$40
                        shl     alu, #1
                        add     alu, table_ed
                        rdword  lmm_pc, alu                     ' fetch and...
                        jmp     #lmm_pc                         ' ...despatch now
'***************************************************************************************************************
fd_xx         {fd}      mov     XY, regptr
                        add     XY, #(@IY_reg - @C_reg)                                          '??????????????????
                        mov     HXY, LXY
                        add     HXY, #1
                        call    #rd_opcode
                        mov     opcode, alu
                        cmp     alu, #$cb               WZ
                if_z    jmp     #xy_cb                          ' fd+cb                 
                        shl     alu, #2
                        add     alu, table_xy
                        jmp     #fetch_1
'***************************************************************************************************************
'             {dd/fd cb disp8 xx}
xy_cb                   call    #mxy
                        mov     tmp, ea                         ' save effective address
                        call    #rd_opcode                      ' read 4th opcode byte
                        mov     ea, tmp
                        mov     opcode, alu
                        mov     postop, alu                     ' additional operation after the write back
                        and     postop, #7                      ' 1 of 8
                        shl     postop, #1                      ' * 2
                        add     postop, postop_ptr              ' one of the post operations
                        rdword  postop, postop
                        mov     aux, opcode
                        shr     aux, #3                         ' discard lower three bits
                        shl     aux, #1                         ' * 2
                        add     aux, table_xy_cb                ' dispatch table offset
                        call    #rd_byte                        ' get memory operand in alu
                        rdword  lmm_pc, aux                     ' fetch and...    
                        jmp     #lmm_pc                         ' ...despatch now 
'***************************************************************************************************************
'               OPCODES CB xx
'***************************************************************************************************************
cb_rlc_b      {cb 00}   mov     alu, B
                        call    #alu_rlc 
                        jmp     #alu_to_B
'***************************************************************************************************************
cb_rlc_c      {cb 01}   mov     alu, C
                        call    #alu_rlc
                        jmp     #alu_to_C
'***************************************************************************************************************
cb_rlc_d      {cb 02}   mov     alu, D
                        call    #alu_rlc
                        jmp     #alu_to_D
'***************************************************************************************************************
cb_rlc_e      {cb 03}   mov     alu, E
                        call    #alu_rlc
                        jmp     #alu_to_E
'***************************************************************************************************************
cb_rlc_h      {cb 04}   mov     alu, H
                        call    #alu_rlc
                        jmp     #alu_to_H
'***************************************************************************************************************
cb_rlc_l      {cb 05}   mov     alu, L
                        call    #alu_rlc
                        jmp     #alu_to_L
'***************************************************************************************************************
cb_rlc_m      {cb 06}   call    #rd_byte_hl
                        call    #alu_rlc
                        jmp     #alu_to_m
'***************************************************************************************************************
cb_rlc_a      {cb 07}   mov     alu, A
                        call    #alu_rlc
                        jmp     #alu_to_A
'***************************************************************************************************************
cb_rrc_b      {cb 08}   mov     alu, B
                        call    #alu_rrc
                        jmp     #alu_to_B
'***************************************************************************************************************
cb_rrc_c      {cb 09}   mov     alu, C
                        call    #alu_rrc
                        jmp     #alu_to_C
'***************************************************************************************************************
cb_rrc_d      {cb 0a}   mov     alu, D
                        call    #alu_rrc
                        jmp     #alu_to_D
'***************************************************************************************************************
cb_rrc_e      {cb 0b}   mov     alu, E
                        call    #alu_rrc
                        jmp     #alu_to_E
'***************************************************************************************************************
cb_rrc_h      {cb 0c}   mov     alu, H
                        call    #alu_rrc
                        jmp     #alu_to_H
'***************************************************************************************************************
cb_rrc_l      {cb 0d}   mov     alu, L
                        call    #alu_rrc
                        jmp     #alu_to_L
'***************************************************************************************************************
cb_rrc_m      {cb 0e}   call    #rd_byte_hl
                        call    #alu_rrc
                        jmp     #alu_to_m
'***************************************************************************************************************
cb_rrc_a      {cb 0f}   mov     alu, A
                        call    #alu_rrc
                        jmp     #alu_to_A
'***************************************************************************************************************
cb_rl_b       {cb 10}   mov     alu, B
                        call    #alu_rl
                        jmp     #alu_to_B
'***************************************************************************************************************
cb_rl_c       {cb 11}   mov     alu, C
                        call    #alu_rl
                        jmp     #alu_to_C
'***************************************************************************************************************
cb_rl_d       {cb 12}   mov     alu, D
                        call    #alu_rl
                        jmp     #alu_to_D
'***************************************************************************************************************
cb_rl_e       {cb 13}   mov     alu, E
                        call    #alu_rl
                        jmp     #alu_to_E
'***************************************************************************************************************
cb_rl_h       {cb 14}   mov     alu, H
                        call    #alu_rl
                        jmp     #alu_to_H
'***************************************************************************************************************
cb_rl_l       {cb 15}   mov     alu, L
                        call    #alu_rl
                        jmp     #alu_to_L
'***************************************************************************************************************
cb_rl_m       {cb 16}   call    #rd_byte_hl
                        call    #alu_rl
                        jmp     #alu_to_m
'***************************************************************************************************************
cb_rl_a       {cb 17}   mov     alu, A
                        call    #alu_rl
                        jmp     #alu_to_A
'***************************************************************************************************************
cb_rr_b       {cb 18}   mov     alu, B
                        call    #alu_rr
                        jmp     #alu_to_B
'***************************************************************************************************************
cb_rr_c       {cb 19}   mov     alu, C
                        call    #alu_rr
                        jmp     #alu_to_C
'***************************************************************************************************************
cb_rr_d       {cb 1a}   mov     alu, D
                        call    #alu_rr
                        jmp     #alu_to_D
'***************************************************************************************************************
cb_rr_e       {cb 1b}   mov     alu, E
                        call    #alu_rr
                        jmp     #alu_to_E
'***************************************************************************************************************
cb_rr_h       {cb 1c}   mov     alu, H
                        call    #alu_rr
                        jmp     #alu_to_H
'***************************************************************************************************************
cb_rr_l       {cb 1d}   mov     alu, L
                        call    #alu_rr
                        jmp     #alu_to_L
'***************************************************************************************************************
cb_rr_m       {cb 1e}   call    #rd_byte_hl
                        call    #alu_rr
                        jmp     #alu_to_m
'***************************************************************************************************************
cb_rr_a       {cb 1f}   mov     alu, A
                        call    #alu_rr
                        jmp     #alu_to_A
'***************************************************************************************************************
cb_sla_b      {cb 20}   mov     alu, B
                        call    #alu_sla
                        jmp     #alu_to_B
'***************************************************************************************************************
cb_sla_c      {cb 21}   mov     alu, C
                        call    #alu_sla
                        jmp     #alu_to_C
'***************************************************************************************************************
cb_sla_d      {cb 22}   mov     alu, D
                        call    #alu_sla
                        jmp     #alu_to_D
'***************************************************************************************************************
cb_sla_e      {cb 23}   mov     alu, E
                        call    #alu_sla
                        jmp     #alu_to_E
'***************************************************************************************************************
cb_sla_h      {cb 24}   mov     alu, H
                        call    #alu_sla
                        jmp     #alu_to_H
'***************************************************************************************************************
cb_sla_l      {cb 25}   mov     alu, L
                        call    #alu_sla
                        jmp     #alu_to_L
'***************************************************************************************************************
cb_sla_m      {cb 26}   call    #rd_byte_hl
                        call    #alu_sla
                        jmp     #alu_to_m
'***************************************************************************************************************
cb_sla_a      {cb 27}   mov     alu, A
                        call    #alu_sla
                        jmp     #alu_to_A
'***************************************************************************************************************
cb_sra_b      {cb 28}   mov     alu, B
                        call    #alu_sra
                        jmp     #alu_to_B
'***************************************************************************************************************
cb_sra_c      {cb 29}   mov     alu, C
                        call    #alu_sra
                        jmp     #alu_to_C
'***************************************************************************************************************
cb_sra_d      {cb 2a}   mov     alu, D
                        call    #alu_sra
                        jmp     #alu_to_D
'***************************************************************************************************************
cb_sra_e      {cb 2b}   mov     alu, E
                        call    #alu_sra
                        jmp     #alu_to_E
'***************************************************************************************************************
cb_sra_h      {cb 2c}   mov     alu, H
                        call    #alu_sra
                        jmp     #alu_to_H
'***************************************************************************************************************
cb_sra_l      {cb 2d}   mov     alu, L
                        call    #alu_sra
                        jmp     #alu_to_L
'***************************************************************************************************************
cb_sra_m      {cb 2e}   call    #rd_byte_hl
                        call    #alu_sra
                        jmp     #alu_to_m
'***************************************************************************************************************
cb_sra_a      {cb 2f}   mov     alu, A
                        call    #alu_sra
                        jmp     #alu_to_A
'***************************************************************************************************************
cb_sli_b      {cb 30}   mov     alu, B
                        call    #alu_sli
                        jmp     #alu_to_B
'***************************************************************************************************************
cb_sli_c      {cb 31}   mov     alu, C
                        call    #alu_sli
                        jmp     #alu_to_C
'***************************************************************************************************************
cb_sli_d      {cb 32}   mov     alu, D
                        call    #alu_sli
                        jmp     #alu_to_D
'***************************************************************************************************************
cb_sli_e      {cb 33}   mov     alu, E
                        call    #alu_sli
                        jmp     #alu_to_E
'***************************************************************************************************************
cb_sli_h      {cb 34}   mov     alu, H
                        call    #alu_sli
                        jmp     #alu_to_H
'***************************************************************************************************************
cb_sli_l      {cb 35}   mov     alu, L
                        call    #alu_sli
                        jmp     #alu_to_L
'***************************************************************************************************************
cb_sli_m      {cb 36}   call    #rd_byte_hl
                        call    #alu_sli
                        jmp     #alu_to_m
'***************************************************************************************************************
cb_sli_a      {cb 37}   mov     alu, A
                        call    #alu_sli
                        jmp     #alu_to_A
'***************************************************************************************************************
cb_srl_b      {cb 38}   mov     alu, B
                        call    #alu_srl
                        jmp     #alu_to_B
'***************************************************************************************************************
cb_srl_c      {cb 39}   mov     alu, C
                        call    #alu_srl
                        jmp     #alu_to_C
'***************************************************************************************************************
cb_srl_d      {cb 3a}   mov     alu, D
                        call    #alu_srl
                        jmp     #alu_to_D
'***************************************************************************************************************
cb_srl_e      {cb 3b}   mov     alu, E
                        call    #alu_srl
                        jmp     #alu_to_E
'***************************************************************************************************************
cb_srl_h      {cb 3c}   mov     alu, H
                        call    #alu_srl
                        jmp     #alu_to_H
'***************************************************************************************************************
cb_srl_l      {cb 3d}   mov     alu, L
                        call    #alu_srl
                        jmp     #alu_to_L
'***************************************************************************************************************
cb_srl_m      {cb 3e}   call    #rd_byte_hl
                        call    #alu_srl
                        jmp     #alu_to_m
'***************************************************************************************************************
cb_srl_a      {cb 3f}   mov     alu, A
                        call    #alu_srl
                        jmp     #alu_to_A
'***************************************************************************************************************
cb_bit_n_b    {cb 40/48/50/58/60/68/70/78}
                        mov     alu, B
                        jmp     #alu_bit
'***************************************************************************************************************
cb_bit_n_c    {cb 41/49/50/59/61/69/71/79}
                        mov     alu, C
                        jmp     #alu_bit
'***************************************************************************************************************
cb_bit_n_d    {cb 42/4a/52/5a/62/6a/72/7a}
                        mov     alu, D
                        jmp     #alu_bit
'***************************************************************************************************************
cb_bit_n_e    {cb 43/4b/53/5b/63/6b/73/7b}
                        mov     alu, E
                        jmp     #alu_bit
'***************************************************************************************************************
cb_bit_n_h    {cb 44/4c/54/5c/64/6c/74/7c}
                        mov     alu, H
                        jmp     #alu_bit
'***************************************************************************************************************
cb_bit_n_l    {cb 45/4d/55/5d/65/6d/75/7d}
                        mov     alu, L
                        jmp     #alu_bit
'***************************************************************************************************************
cb_bit_n_m    {cb 46/4e/56/5e/66/6e/76/7e}
                        call    #rd_byte_hl
                        jmp     #alu_bit
'***************************************************************************************************************
cb_bit_n_a    {cb 47/4f/57/5f/67/6f/77/7f}
                        mov     alu, A
                        jmp     #alu_bit
'***************************************************************************************************************
cb_res_n_b    {cb 80/88/90/98/a0/a8/b0/b8}
                        andn    B, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_res_n_c    {cb 81/89/91/99/a1/a9/b1/b9}
                        andn    C, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_res_n_d    {cb 82/8a/92/9a/a2/aa/b2/ba}
                        andn    D, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_res_n_e    {cb 83/8b/93/9b/a3/ab/b3/bb}
                        andn    E, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_res_n_h    {cb 84/8c/94/9c/a4/ac/b4/bc}
                        andn    H, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_res_n_l    {cb 85/8d/95/9d/a5/ad/b5/bd}
                        andn    L, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_res_n_m    {cb 86/8e/96/9e/a6/ae/b6/be}
                        call    #rd_byte_hl
                        andn    alu, tmp
                        jmp     #alu_to_m
'***************************************************************************************************************
cb_res_n_a    {cb 87/8f/97/9f/a7/af/b7/bf}
                        andn    A, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_set_n_b    {cb c0/c8/d0/d8/e0/e8/f0/f8}
                        or      B, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_set_n_c    {cb c1/c9/d1/d9/e1/e9/f1/f9}
                        or      C, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_set_n_d    {cb c2/ca/d2/da/e2/ea/f2/fa}
                        or      D, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_set_n_e    {cb c3/cb/d3/db/e3/eb/f3/fb}
                        or      E, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_set_n_h    {cb c4/cc/d4/dc/e4/ec/f4/fc}
                        or      H, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_set_n_l    {cb c5/cd/d5/dd/e5/ed/f5/fd}
                        or      L, tmp
                        jmp     #fetch
'***************************************************************************************************************
cb_set_n_m    {cb c6/ce/d6/de/e6/ee/f6/fe}
                        call    #rd_byte_hl
                        or      alu, tmp
                        jmp     #alu_to_m
'***************************************************************************************************************
cb_set_n_a    {cb c7/cf/d7/df/e7/ef/f7/ff}
                        or      A, tmp
                        jmp     #fetch
'***************************************************************************************************************
'               OPCODES ED xx
'***************************************************************************************************************
bc_into_ea    _RET_     mov     ea, C                           ' ea  <-- C/BC
'***************************************************************************************************************
hl_into_ea              mov     ea, H                           ' ea  <-- HL
                        shl     ea, #8
              _RET_     or      ea, L
'***************************************************************************************************************
ed_in_b_bc    {ed 40}   call    #bc_into_ea                     ' B   <-- port[C/BC]
                        call    #rd_port
                        jmp     #alu_to_b
'***************************************************************************************************************
ed_in_c_bc    {ed 48}   call    #bc_into_ea                     ' C   <-- port[C/BC]
                        call    #rd_port
                        jmp     #alu_to_c
'***************************************************************************************************************
ed_in_d_bc    {ed 50}   call    #bc_into_ea                     ' D   <-- port[C/BC]
                        call    #rd_port
                        jmp     #alu_to_d
'***************************************************************************************************************
ed_in_e_bc    {ed 58}   call    #bc_into_ea                     ' E   <-- port[C/BC]
                        call    #rd_port
                        jmp     #alu_to_e
'***************************************************************************************************************
ed_in_h_bc    {ed 60}   call    #bc_into_ea                     ' H   <-- port[C/BC]
                        call    #rd_port
                        jmp     #alu_to_h
'***************************************************************************************************************
ed_in_l_bc    {ed 68}   call    #bc_into_ea                     ' L   <-- port[C/BC]
                        call    #rd_port
                        jmp     #alu_to_l
'***************************************************************************************************************
ed_in_0_bc    {ed 70}   call    #bc_into_ea                     ' 0   <-- port[C/BC]
                        call    #rd_port
                        jmp     #fetch
'***************************************************************************************************************
ed_in_a_bc    {ed 78}   call    #bc_into_ea                     ' A   <-- port[C/BC]
                        call    #rd_port
                        jmp     #alu_to_a
'***************************************************************************************************************
ed_out_bc_b   {ed 41}   call    #bc_into_ea                     ' port[C/BC] <-- B
                        mov     alu, B
                        call    #wr_port
                        jmp     #fetch
'***************************************************************************************************************
ed_out_bc_c   {ed 49}   call    #bc_into_ea                     ' port[C/BC] <-- C
                        mov     alu, C
                        call    #wr_port
                        jmp     #fetch
'***************************************************************************************************************
ed_out_bc_d   {ed 51}   call    #bc_into_ea                     ' port[C/BC] <-- D
                        mov     alu, D
                        call    #wr_port
                        jmp     #fetch
'***************************************************************************************************************
ed_out_bc_e   {ed 59}   call    #bc_into_ea                     ' port[C/BC] <-- E
                        mov     alu, E
                        call    #wr_port
                        jmp     #fetch
'***************************************************************************************************************
ed_out_bc_h   {ed 61}   call    #bc_into_ea                     ' port[C/BC] <-- H
                        mov     alu, H
                        call    #wr_port
                        jmp     #fetch
'***************************************************************************************************************
ed_out_bc_l   {ed 69}   call    #bc_into_ea                     ' port[C/BC] <-- L
                        mov     alu, L
                        call    #wr_port
                        jmp     #fetch
'***************************************************************************************************************
ed_out_bc_0   {ed 71}   call    #bc_into_ea                     ' port[C/BC] <-- 0
                        mov     alu, #0
                        call    #wr_port
                        jmp     #fetch
'***************************************************************************************************************
ed_out_bc_a   {ed 79}   call    #bc_into_ea                     ' port[C/BC] <-- A
                        mov     alu, A
                        call    #wr_port
                        jmp     #fetch
'***************************************************************************************************************
ed_sbc_hl_bc  {ed 42}   mov     t1, C                           ' HL = HL-BC-CF & set FLAGS
                        mov     tmp, B
                        test    F, #CF          WC              ' get C into prop's carry
                        subx    L, t1                           ' subtract the LSB tmp value with carry
                        test    L, #$100        WC              ' get LSB carry
                        and     L, #$ff
                        mov     F, #NF                          ' clear all flags but N flag for SBC
                        mov     alu, H
                        mov     aux, H
                        subx    alu, tmp                        ' subtract the MSB tmp value with carry
                        call    #szhvc_flags
                if_z    cmp     L, #0           WZ
                if_nz   andn    F, #ZF                          ' clear Z flag if L is non zero
                        jmp     #alu_to_h
'***************************************************************************************************************
ed_sbc_hl_de  {ed 52}   mov     t1, E                           ' HL = HL-DE-CF & set FLAGS
                        mov     tmp, D
                        test    F, #CF          WC              ' get C into prop's carry
                        subx    L, t1                           ' subtract the LSB tmp value with carry
                        test    L, #$100        WC              ' get LSB carry
                        and     L, #$ff
                        mov     F, #NF                          ' clear all flags but N flag for SBC
                        mov     alu, H
                        mov     aux, H
                        subx    alu, tmp                        ' subtract the MSB tmp value with carry
                        call    #szhvc_flags
                if_z    cmp     L, #0           WZ
                if_nz   andn    F, #ZF                          ' clear Z flag if L is non zero
                        jmp     #alu_to_h
'***************************************************************************************************************
ed_sbc_hl_hl  {ed 62}   mov     t1, L                           ' HL = HL-HL-CF & set FLAGS
                        mov     tmp, H
                        test    F, #CF          WC              ' get C into prop's carry
                        subx    L, t1                           ' subtract the LSB tmp value with carry
                        test    L, #$100        WC              ' get LSB carry
                        and     L, #$ff
                        mov     F, #NF                          ' clear all flags but N flag for SBC
                        mov     alu, H
                        mov     aux, H
                        subx    alu, tmp                        ' subtract the MSB tmp value with carry
                        call    #szhvc_flags
                if_z    cmp     L, #0           WZ
                if_nz   andn    F, #ZF                          ' clear Z flag if L is non zero
                        jmp     #alu_to_h
'***************************************************************************************************************
                        fit     $400
'***************************************************************************************************************


'===============================================================================================================
' HUBEXEC code (load new cog code...)
'===============================================================================================================
              orgh      $3000
''+------[ Load new COG code ]-------------------------------------------------+
reload_cog      setq    #512-16-1                       '\ re-load COG 
                rdlong  0, ##@cog_code                 '/          
''+------[ Start user program ]------------------------------------------------+
                jmp     #entry                          ' jump to user program
''+------[ Program identifier (must be in hub) ]-------------------------------+
hubidstring     byte    "Z80 Emulation: P2-qz80-rr031",13,10,0
                alignl
''+----------------------------------------------------------------------------+

'===============================================================================================================
' HUBEXEC code (DEBUG code...)
'===============================================================================================================
_debug
{{
''+----------------------------------------------------------------------------+
''+ Dump COG memory                                                            +
''+----------------------------------------------------------------------------+
              mov       lmm_f,            #_LIST+_ADDR2 ' list w addr2
              mov       lmm_p,            #C            ' fm addr
              mov       lmm_p2,           #ram_base     ' to addr
              call      #_HubList
''+----------------------------------------------------------------------------+
''+ Dump HUB memory                                                            +
''+----------------------------------------------------------------------------+
              mov       lmm_f,            #_LIST+_ADDR2 ' list w addr2
                                                        ' $1_xxxxx tricks to force hub :)
              mov       lmm_p,            ##C_reg       ' fm addr
              mov       lmm_p2,           ##PC_reg      ' to addr
              call      #_HubList
''+----------------------------------------------------------------------------+
}}
              ret
''+----------------------------------------------------------------------------+

_debug_wr_port

              call      #_hubTxCR
              call      #_debug_head
              call      #_debug_regs
              mov       lmm_x, t1
              call      #_hubHex8
              call      #_hubTxCR
'              call      #_hubTxCR
'              ret

              mov       lmm_p, D                ' (DE) points to string, '$' terminated
              shl       lmm_p, #8
              or        lmm_p, E
              mov       lmm_x, lmm_p
              call      #_hubHex8
              call      #_hubTxCR
' now display string
              mov       lmm_c, #40              ' limit string length
              or        lmm_p, ##Z80_MEM
.loop         rdbyte    lmm_x, lmm_p            ' get byte from (DE)
              add       lmm_p, #1               ' DE++
              cmp       lmm_x, #"$"       WZ
        if_nz call      #_hubTx                 ' write byte from string
        if_nz djnz      lmm_c, #.loop
              call      #_hubTxCR
              ret

              
_debug_head
              mov     lmm_f, #_TXSTRING+0       ' send string, $00 terminated
              mov     lmm_p, ##@_reg_hdg        ' must be in hub!
              call    #_HubTxString
              ret

_debug_regs              
              mov     lmm_f, #_HEX_+4                   ' 4 nibbles
              mov     lmm_x, PC
              call    #_hubHex
              mov     lmm_x, #" "
              call    #_hubTx

              mov     lmm_f, #_HEX_+2                   ' 2 nibbles
              mov     lmm_x, B
              call    #_hubHex
              mov     lmm_x, C
              call    #_hubHex
              mov     lmm_x, #" "
              call    #_hubTx

              mov     lmm_x, D
              call    #_hubHex
              mov     lmm_x, E
              call    #_hubHex
              mov     lmm_x, #" "
              call    #_hubTx

              mov     lmm_x, H
              call    #_hubHex
              mov     lmm_x, L
              call    #_hubHex
              mov     lmm_x, #" "
              call    #_hubTx

              mov     lmm_x, A
              call    #_hubHex
              mov     lmm_x, #" "
              call    #_hubTx

              mov     lmm_x, F
              call    #_hubHex
              mov     lmm_x, #" "
              call    #_hubTx

              mov     lmm_x, R
              call    #_hubHex
              mov     lmm_x, #" "
              call    #_hubTx

              mov     lmm_x, R2
              call    #_hubHex
              mov     lmm_x, #" "
              call    #_hubTx

              mov     lmm_x, IFF
              call    #_hubHex
              mov     lmm_x, #" "
              call    #_hubTx

              mov     lmm_f, #_HEX_+4                   ' 4 nibbles
              mov     lmm_x, SP
              call    #_hubHex

              mov       lmm_x, ##(" " + "="<<8 + " "<<16)
              call      #_hubTx

''              call    #_hubTxCR

              ret

_debug_opc
' display alu
              mov     lmm_f, #_HEX_+4                   ' 4 nibbles 
              mov     lmm_x, alu
              call    #_hubHex
              mov     lmm_x, #" "
              call    #_hubTx

' display ea (=PC)
              mov       lmm_f, #_HEX_+4                 ' 4 nibbles
              mov       lmm_x, ea                       ' current PC (before PC++)
              call      #_hubHex
              mov       lmm_x, ##(":" + " "<<8)
              call      #_hubTx

' display 4 opcode(s)
              rdlong    lmm_x, ea                       ' get current opcode(s)
              mov       lmm_f, #_HEX_+_REV_+_SP+0
              call      #_hubHex
              mov       lmm_x, #" "
              call      #_hubTx

' display vector3 (long)
              mov       lmm_x, vector3
              call      #_hubHex8                       ' the vector long
              mov       lmm_x, ##(":" + " "<<8)
              call      #_hubTx

' decode & display the 3 vectors: 12b 10b 10b
              mov       lmm_f, #_HEX_+3                 ' 3 nibble vectors
              mov       lmm_x, vector3                  ' 1st vector (12-bits)
              and       lmm_x, maskFFF
              call      #_hubHex
              mov       lmm_x, #" "
              call      #_hubTx
              mov       lmm_x, vector3                  ' 2nd vector & isolate 10b
              shr       lmm_x, #12           
              and       lmm_x, mask3FF
              call      #_hubHex
              mov       lmm_x, #" "
              call      #_hubTx
              mov       lmm_x, vector3                  ' 3rd vector
              shr       lmm_x, #22
              call      #_hubHex
              mov       lmm_x, ##(" " + " "<<8)
              call      #_hubTx

              mov       lmm_f, #_HEX_+4                 ' 4 nibbles
              mov       lmm_x, PC                       ' PC++
              call      #_hubHex

              call      #_hubTxCR
              ret


_reg_hdg      byte      "PC   B C  D E  H L  A  F  R  R2 IF SP     alu  ea    OpCode(s)    Vector3   V-1 V-2 V-3  PC++",13,10,0
              alignl
              
              
'===============================================================================================================
' Z80 code (64KB)...
'===============================================================================================================
              orgh      Z80_MEM
                                                '                               BDOS... 
{0000}        byte      $C3, $00, $01           ' JP nnnn     $0100             warm boot               imm16_to_alu, alu_to_pc
{0003}        byte      $00[2]                  ' NOP * 2                       intel io??
{0005}        byte      $C3, $00, $F0           ' JP bios                       bdos: (stack below)     imm16_to_alu, alu_to_pc
{0008}        byte      $00[16-8]               ' INC C * 8     
{0010}        byte      $C3, $00, $00           ' JP nnnn                                               imm16_to_alu, alu_to_pc



              orgh      Z80_MEM + $0100
              file      "zmac/zout/zexdoc.cim"          ' binary file starting at $0100


              orgh      Z80_MEM + $F000                 ' bios start, with stack below
bios
              byte      $00                             ' nop filler
              byte      $D3, $00                        ' OUT (nn),A                                    calls debug msg
              byte      $00                             ' nop filler
              byte      $C9                             ' RET
              byte      $00                             ' nop filler
                
              orgh      Z80_MEM + $1_0000               ' fill to end of Z80 64KB