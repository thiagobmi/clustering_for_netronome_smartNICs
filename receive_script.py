#!/usr/bin/env python3
import sys
import struct

from scapy.all import sniff, sendp, hexdump, get_if_list, get_if_hwaddr, bind_layers
from scapy.all import Packet, IPOption
from scapy.all import IP, UDP, Raw, Ether
from scapy.layers.inet import _IPOption_HDR
from scapy.fields import *
from enum import Enum

class Test(Packet):
    fields_desc = [BitField("X",0,16),
                  BitField("Y", 0, 16),
		  BitField("add",0,16),
		  BitField("packets",0,16),
		  BitField("current_distance",0,16)]
bind_layers(IP, Test, proto=0x45)

def handle_pkt(pkt):
    #print("got a packet")
    pkt.show2()
#    hexdump(pkt)
    sys.stdout.flush()


def main():
    iface = "enp71s0np1"
    print("sniffing on %s" % iface)
    sys.stdout.flush()
    sniff(iface = iface,
          prn = lambda x: handle_pkt(x))


if __name__ == '__main__':
    main()

