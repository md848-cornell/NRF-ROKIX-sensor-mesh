#!/usr/bin/python3 

import os 
import subprocess as sp 
import telnetlib
import time
import re

import traceback


import plotshape
from plotshape import *

JLINK_EXE = '/opt/SEGGER/JLink/JLinkExe'
JLINK_CLIENT = '/opt/SEGGER/JLink/JLinkRTTClient'
DEBUG = 0

class RTT:

    def __init__(self, simulate=False):
        #####
        if simulate:
            print('ready')
        else:
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

    def plot_sensor(self, pos):
        nodes = {}
        shp = ShapePlot()


        raw_output = False
        self.done = False
        s = ''
        while not self.done:
            s += self.read_client()
            lines = s.split('\n')
            for i in range(len(lines)-1):
                line = lines[i]
                if 'BSENSE msg' in line:
                    r = self.parse_line(line)
                    r = r[0]
                    if r != None:
                        print(r)
                        addr = r[0]
                        if addr not in nodes.keys():
                            nodes[addr] = [pos[0], [0, 0, 0]]
                            pos = pos[1:]
                        nodes[addr][1][0] = float(r[2])
                        nodes[addr][1][1] = float(r[3])
                        nodes[addr][1][2] = float(r[4])

                else:
                    if DEBUG: print(line)
            s = lines[-1]
            vecs = []
            points = []
            for addr in nodes.keys():
                points += [nodes[addr][0]]
                vecs += [nodes[addr][1]]
            #print('p:',points)
            #print('v:',vecs)

            if len(points) == len(vecs) and len(vecs) > 0:
                try:
                    shp.plot(vecs, points)
                except Exception as e:
                    pass
                    #print(e)

    def plot_sensor_test(self, pos):
        nodes = {}
        shp = ShapePlot()

        raw_output = False
        self.done = False
        s = ''
        while not self.done:

            f = open('example_mesh_data.txt','r')
            lines = f.readlines() 
            f.close()
            for i in range(len(lines)):
                line = lines[i]
                r = eval(line)[0]
                addr = r[0]
                if addr not in nodes.keys():
                    print('adding node:' , addr, pos[0])
                    nodes[addr] = [pos[0], [0, 0, 0]]
                    pos = pos[1:]
              
                nodes[addr][1][0] = float(r[2])
                nodes[addr][1][1] = float(r[3])
                nodes[addr][1][2] = float(r[4])
                print('set vector:', addr, nodes[addr][0], nodes[addr][1])

                if DEBUG: print(line)
                time.sleep(0.1)

                s = lines[-1]
                vecs = []
                points = []
                for addr in nodes.keys():
                    points += [nodes[addr][0]]
                    vecs += [nodes[addr][1]]
                try:
                    shp.plot(vecs, points)
                except Exception as e:
                    print(e)
                    traceback.print_exc()




def main():
    positions = [\
            [8,0,0],\
            [-8,0,0]]
    rtt = RTT()
    rtt.plot_sensor(positions)

if __name__ == '__main__':
    main()
