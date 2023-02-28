#!/usr/bin/env python3
import argparse
import sys
import socket
from random import randint
import struct

from scapy.all import sendp, get_if_list, get_if_hwaddr, bind_layers
from scapy.all import Packet
from scapy.all import Ether, IP, UDP, TCP
from scapy.fields import *
from time import sleep

class Test(Packet):
    fields_desc = [BitField("X",0,16),
                  BitField("Y", 0, 16),
		  BitField("add",0,16),
		  BitField("packets",0,16),
		  BitField("current_distance",0,16)]
bind_layers(IP, Test, proto=0x45)

def main():
    iface = "enp71s0np0"
    addr="10.0.1.10"

    print("sending on interface %s to %s" % (iface, str(addr)))
    adicional = 2
    div = 1
    i=0;
    while True:
        input("Press enter to send a packet...")
        ptX = randint(0,100)
        ptY = randint(0,100)
        pkt = Ether(src=get_if_hwaddr(iface), dst='00:15:4d:13:63:3c') / IP(dst=addr,proto=0x45) / Test(X=ptX,Y=ptY,add=adicional)
        sendp(pkt, iface=iface, verbose=False)

if __name__ == '__main__':
    main()




