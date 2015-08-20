
System Design:
==============

             --------------------------------------------------------------------------------------
            |                                                                                      |
Global addr |           0001                         0000                          0010            |
            |    ------------------           ------------------            ------------------     |
             --> | NoC Switch | 1 |  -------> | NoC Switch | 0 |  ------->  | NoC Switch | 2 |  ---
                 ------------------           ------------------            ------------------
Local addr        00  /     01 \              00  /     01  \              00  /       01  \
                  -------   -------           -------   =========          =========   =========
                  | ETH |   | H2S |           | S2H |   || FB0 ||          || FB1 ||   || FB2 ||
                  -------   -------           -------   =========          =========   =========
NoC addr             4         5                 0          1                  8           9       

NoC addr = concatenate(global_addr, local_addr)   
Example: H2S block: concatenate(0001, 01) = 000101 = 5


Tools & Commands
================


xps       hardware development for xilinx FPGA
xsdk      software development for xilinx FPGA 
impact    download bit files to FPGA
dow       script to download bit files and OS kernel image to FPGA
ise       simulation of hardware implementation (e.g. with testbenches)
analyzer  chipscope: tool for hardware debugging
minicom   terminal emulation to connect with the Linux on the FPGA
ostinato  generate arbitrary network traffic


Start the EmbedNet
==================
 1. Switch on power of FPGA board
 2. Wait until network interface LEDs light up
 3. download the bitstream, e.g. 
    dow system.bit
 4. download the kernel image, e.g. 
    dow simpleImage.generic
 5. connect to the FPGA using minicom, e.g. 
    sudo minicom -D /dev/ttyUSB0
 6. Connect the FSLs (Fast Simplex Links) and set up the memory, e.g.
    ./load_fsl.sh
    ./load_getpgd.sh
 7. Start the software (which sets up the protocol stacks and starts the measurements), e.g.
    ./generic_fb -f<config file> <other arguments>
