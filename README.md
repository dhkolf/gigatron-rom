Software for Gigatron ROM
=========================

Files
=====
```
Core/theloop.py                 Video/audio/io/interpreter loops
                                Built-in vCPU applications and data
                                Execute theloop.py to build ROM files
Core/compilegcl.py              Tool to compile vCPU applications for use with
                                the Arduino Loader
FileFormat.txt                  Description of vCPU object file format
GCL-language.txt                Gigatron Control Language and vCPU explanation
gcl1.ebnf                       Formal definition
EBNF.xhtml                      Railroad diagram
gtemu.c                         Executable instruction set definition
LICENSE                         2-Clause BSD License
Makefile                        Marcel's Makefile
README.md                       This file...
LoaderTest/LoaderTest.ino       Arduino sketch for "Loader" demonstration
Terminal/Terminal.ino           Arduino sketch for "Terminal" (under development)
```

Files processed by theloop.py
=============================
```
Core/Reset.gcl                  System reset code
Core/gcl0x.py                   Module: GCL to vCPU compiler
Core/asm.py                     Module: Assembler functions
Core/font.py                    Module: Gigatron font definition
Apps/*.gcl                      Application code
Images/Baboon-160x120.rgb       Raw RGB image file (source: Baboon-160x120.png)
Images/Jupiter-160x120.rgb      Raw RGB image file (source: Jupiter-160x120.png)
Images/Parrot-160x120.rgb       Raw RGB image file (source: Parrot-160x120.png)
```

Files generated by theloop.py
=============================
```
theloop.asm                     Annotated disassembly, with labels and comments
theloop.sym                     Symbol table for the ROM file
theloop.0.rom                   ROM file for 28C256 #0 (breadboard)
theloop.1.rom                   ROM file for 28C256 #1 (breadboard)
theloop.2.rom                   ROM file for 27C1024 (PCB versions)
```

Memory map (RAM)
================
```
              0 1          47 48     127 128 129         (vSP)       255
             +-+-------------+-----------+-+-------------+--------------+
page 0       |0| System vars | User vars |1| User vars <-|-> vCPU stack |
             +-+-------------+-----------+-+------+------+--+-----------+
page 1       | Video table                     239| vReset  | Channel 1 |
             +------------------------------------+---------+-----------+
page 2       |0                                    240   249| Channel 2 |
             |                                              +-----------+
page 3       | User vCPU code and/or data                   | Channel 3 |
             |                                              +-----------+
page 4       |                                              | Channel 4 |
             |                                              +-----------+
page 5-6     |                                              250      255|
             |                                                          |
             |                                                          |
             +----------------------------------------------------------+
page 7       | Sound tables                                             |
             +--------------------------------+-------------------------+
page 8-127   |0                            159|160                   255|
             | 120 lines of 160 pixels        |  Extra space for user   |
             | Default video memory           |  code and/or data       |
             =                                =                         =
             |                                |                         |
             +--------------------------------+-------------------------+
page 128-255 | Not used in the 32K system: mirror of page 0-127         |
             +----------------------------------------------------------+
              0                                                      255

Address   Name          Description
--------  ------------- -----------
0000      0             Constant
0001      memSize       Number of RAM pages detected at hard reset (64kB=0)
0002      channel       Sound channel update on current scanline
0003      sample        Accumulator for synthesizing next sound sample
0004      bootCount     0 for cold boot, >0 for warm boot
0005      bootCheck     Checksum of bootCount
0006-0008 entropy       Randomness from SRAM boot and input, updated each frame
0009      videoY        Counts up from 0 to 238 in steps of 2 (odd in vBlank)
000a      frameX        Starting byte within page for pixel burst
000b      frameY        Page of current pixel row (updated by videoA)
000c      nextVideo     Jump offset to scan line handler (videoA, B, C...)
000d      videoDorF     Handler for every 4th line (videoD or videoF)
000e      frameCount    Continuous video frame counter
000f      serialRaw     New raw serial read
0010      serialLast    Previous serial read
0011      buttonState   Clearable button state
0012      resetTimer    After 2 seconds of holding 'Start', do a soft reset
0013      xout          Memory cache for XOUT register
0014      xoutMask      The blinkenlights and sound on/off state
0015      vTicks        Interpreter ticks are units of 2 clocks
0016-0017 vPC           Interpreter program counter, points into RAM
0018-0019 vAC           Interpreter accumulator, 16-bits
001a-001b vLR           Return address, for returning after CALL
001c      vSP           Stack pointer
001d      vTmp          Scratch storage location for vCPU
001e      vReturn       Return address (L) from vCPU into the loop (H is fixed)
001f-0020 reserved      Reserved for ROM extensions
0021      romType       0x1c for ROMv1 release
0022-0023 sysFn         Address for SYS function call
0024-002b sysArgs       Arguments for SYS functions
002c      soundTimer    Countdown timer for playing sound
002d      ledTimer      Number of ticks until next LED change
002e      ledState      Current LED state
002f      ledTempo      Next value for ledTimer after LED state change
0030-007f -             Program variables
0080      1             Constant
0081-.... -             Program variables
....-00ff <stack>
0100-01ef videoTable
01f0-01f9 vReset
01fa      wavA[1]       Sound channel 1
01fb      wavX[1]
01fc      keyL[1]
01fd      keyH[1]
01fe      oscL[1]
01ff      oscH[1]
0200-02f9 -             vCPU code/data (standard start address)
02fa      wavA[2]       Sound channel 2
02fb      wavX[2]
02fc      keyL[2]
02fd      keyH[2]
02fe      oscL[2]
02ff      oscH[2]
0300-03f9 -             vCPU code/data
03fa      wavA[3]       Sound channel 3
03fb      wavX[3]
03fc      keyL[3]
03fd      keyH[3]
03fe      oscL[3]
03ff      oscH[3]
0400-04f9 -             vCPU code/data
04fa      wavA[4]       Sound channel 4
04fb      wavX[4]
04fc      keyL[4]
04fd      keyH[4]
04fe      oscL[4]
04ff      oscH[4]
0500-05ff -             vCPU code/data
0600-06ff -             vCPU code/data
0700-07ff soundTable
0800-089f pixel line 0
08a0-08ff -             vCPU code/data
...
7f00-7f9f pixel line 119
7fa0-7fff -             vCPU code/data
--------  ------------- -----------
```

Memory map (ROM)
================
```
             +----------------------------------------------------------+
page 0       | Boot and reset sequences                                 |
             +----------------------------------------------------------+
page 1       | Video and audio loop (visible lines)                     |
             |                                                          |
page 2       | Video and audio loop (vertical blank)                    |
             +----------------------------------------------------------+
page 3       | vCPU interpreter loop (primary page)                     |
             |                                                          |
page 4       | vCPU extended instructions and SYS functions             |
             +----------------------------------------------------------+
page 5       | Shift tables                                             |
             |                                                          |
page 6       | SYS functions (LSRW and ohers)                           |
             +----------------------------------------------------------+
page 7-8     | Font table                                               |
             +----------------------------------------------------------+
page 9       | Notes table                                              |
             +----------------------------------------------------------+
page 10      | Inversion table                                          |
             +----------------------------------------------------------+
page 11-214  | ROM tables: Embedded high-resolution images (packed)     |
             |                                                          |
page 215-255 | ROM files: Embedded vCPU applications (serial)           |
             +----------------------------------------------------------+
              0                                                      255
```
