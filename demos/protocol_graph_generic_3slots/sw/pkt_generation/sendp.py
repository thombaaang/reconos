#!/usr/bin/env python

from scapy.all import *
import binascii
import logging


##############################
# parameters

interface = "en1"
pkt_len = 64
hash_load = 'abababababababab'
pps_start = 10
pps_stop = 100
pps_factor = 1.005



##############################

# suppress warnings
logging.getLogger("scapy").setLevel(logging.ERROR)
logging.getLogger("sendpfast").setLevel(logging.ERROR)


# generate packet

header = Ether()
header.dst = "11:11:11:11:11:11"
header.src = "00:00:00:00:00:00"
header.type = 0xacdc

hash = Padding()
hash.load = binascii.unhexlify(hash_load)
#hash.load = '\xab' * 8

pad = Padding()
pad.load = '\x00' * (pkt_len - len(header/hash))

pkt = header/hash/pad

#print pkt.show()

# send pkt once
#sendp(pkt, iface=interface)

# send pkt multiple time with high pps
#sendpfast(pkt, pps=200, loop=4, iface=interface)

# save pkt to pcap file

# send pkt from pcap file with looping forever e.g. with "tcpreplay -l0 -p 1000 -i"en1" -q test.pcap 2>/dev/null"
# the "2>/dev/null" part suppresses all warnings (the printing of many warnings could influence the performance)
# if tcpreplay is used with a file with timing info and the -p (pps) option is not set it replays the packets in 
# real time! -> this allows it to create a custom pps curve
# the replay with tcpreplay does work if loop=1, with infinite looping (loop=0) it produces a bulk load of about
# 200'000 packets (but in fact it should only send about 460 packts in 20 secs)
# by manually start tcpreplay several times a nice graph in wireshark can be produces (also filter for eth.addr=0:0:0:0:0:0)
pktdump = PcapWriter("exp_growth.pcap", append=True, sync=True)

pps_now = float(pps_start)
pkt.time = 0.0
pktdump.write(pkt)

while pps_now < pps_stop:
  pkt.time = pkt.time + 1.0/pps_now
  #print pkt.time, 1.0/pps_now, pps_now
  pktdump.write(pkt)
  pps_now = pps_factor*pps_now

  


# alternative way to write a pcap file (no append option)
#wrpcap("test2.pcap", pkt)
