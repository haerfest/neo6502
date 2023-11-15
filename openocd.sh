#!/usr/bin/env sh
#
# Starts OpenOCD as a bridge between GDB on a Raspberry Pi 4 and the
# RP2040 microcontroller on the Neo6502 when connected as follows:
#
#  Pi 4
#  GPIO
#
#  1  2
#  .  .
#  .  .
#  .  .
#  .  .
#  .  .         Neo6502 
#  .  .          BUS 1
#  .  .
#  . 18 ---\/--- . 2  GND
#  . 20 ---/\--- . 4  SWDIO
#  . 22 -------- . 6  SWCLK
#  .  .          . .
#  .  .          . .

openocd \
    -f interface/raspberrypi-swd.cfg \
    -f target/rp2040.cfg
