
System Design:
==============

  -------------------------------------------------------
 |                                                       |
 |    ------------------           ------------------    |
  --> | NoC Switch | 1 |  -------> | NoC Switch | 0 | ---
      ------------------           ------------------
           /        \                 /         \
       -------   -------           -------   =========
       | ETH |   | H2S |           | S2H |   || FB1 || <---- Functional Block
       -------   -------           -------   ========= 


Tools
=====


xps       hardware development for xilinx FPGA
xsdk      software development for xilinx FPGA 
impact    download bit files to FPGA
dow       script to download bit files and OS kernel image to FPGA
ise       simulation of hardware implementation (e.g. with testbenches)
analyzer  chipscope: tool for hardware debugging
minicom   terminal to connect with the FPGA
ostinato  generate arbitrary Internet traffic


Run EmbedNet
============
 1. Switch on power of FPGA board
 2. Wait until network interface LEDs light up
 3. download the bitstream, e.g. dow system.bit
 4. download the kernel image, e.g. dow simpleImage.generic
 5. connect to the FPGA using minicom, e.g. minicom -D /dev/ttyUSB0
