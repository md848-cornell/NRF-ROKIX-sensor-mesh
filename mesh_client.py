#!/usr/bin/python3 

import os 
import subprocess as sp 
import telnetlib
import time
import re

JLINK_EXE = '/opt/SEGGER/JLink/JLinkExe'
JLINK_CLIENT = '/opt/SEGGER/JLink/JLinkRTTClient'
DEBUG = False

class RTT:

    def __init__(self):
        #####
        if False:
            self.exe = sp.Popen([JLINK_EXE] + \
                    "-autoconnect 1 -device NRF52840_XXAA -if SWD -speed 4000".split(), \
                    stdout=sp.PIPE, \
                    stdin=sp.PIPE, \
                    stderr=sp.PIPE, \
                    )
            print('started exe')
            time.sleep(1)
                
        #####
        self.client = telnetlib.Telnet() 
        self.client.open("127.0.0.1", 19021)
        print('connected client')

    def read_client(self):
        return str(self.client.read_some().decode())

    def read_sensor(self):
        raw_output = False
        self.done = False
        s = ''
        while not self.done:
            s += self.read_client()
            lines = s.split('\n')
            for i in range(len(lines)-1):
                line = lines[i]
                if 'BSENSE msg' in line:
                    print(self.parse_line(line))
                    #print(line)
                else:
                    if DEBUG: print(line)
            s = lines[-1]

    def parse_line(self,line):
        # example line
        #<t:    6908230>, main.c,  381, current: BSENSE msg - Node 0x0104 (TTL:30) - sensor data: 378 79 -16383
        p = '<t: [ 0-9]+>, main.c, [ 0-9]+, current: BSENSE msg - Node (0x[0-9A-F]+) \(TTL:([0-9]+)\) - sensor data: ([\-0-9]+) ([\-0-9]+) ([\-0-9]+)' 
        r = re.findall(p,line)
        return r

    def exit():
        try:
            self.exe.kill()
            self.client.close()
        except Exception as e:
            print(e)


def main():
    try:
        rtt = RTT()
        rtt.read_sensor()
    except Exception as e:
        print(e)

if __name__ == '__main__':
    main()
