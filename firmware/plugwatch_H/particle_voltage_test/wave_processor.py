#!/usr/bin/env python3

import json
import base64
import struct
import matplotlib.pyplot as plt

f = open('voltage_wave.txt', 'r')

l = []
n = []

for line in f.readlines():
    if(len(line) > 0 and line[0] == 'd'):
        data = json.loads(line[6:])
        data = data['data'].encode('ascii')
        decoded = base64.b64decode(data)
        for i in range(0,len(decoded),2):
            value = struct.unpack("<H",decoded[i:i+2])[0]
            value = value*(-3.3/4096)*953 + 1.5*953
            if(i % 4 == 0):
                l.append(value)
            else:
                n.append(value)

plt.plot(l,label='L')
plt.plot(n,label='N')
plt.legend()
plt.show()
