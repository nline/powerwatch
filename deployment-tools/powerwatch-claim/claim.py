#! /usr/bin/env python3

import os
import sys
import requests
import argparse
import json

parser = argparse.ArgumentParser(description = 'Deploy particle firmware')
parser.add_argument('-p','--product', type=str, required=True,action='append')

args = parser.parse_args()

#get the key from the particle file/login
key_file = open(os.environ['HOME'] + '/.particle/particle.config.json','r')
particle_key = None
try:
    keys = json.loads(key_file.read())
    particle_key = keys['access_token']
except:
    print("Install particle CLI and login with 'particle login' to generate a key file. Exiting.")
    sys.exit(1)

#get a list of devices in the product
print('Getting list of devices in product...')
r = requests.get("https://api.particle.io/v1/products/" + args.product[0] + "/devices?access_token=" + particle_key + '&perPage=1000&sortAttr=firmwareVersion&sortDir=desc')

resp = json.loads(r.text)
if 'ok' in resp:
    if(resp['ok'] is False):
        print('Getting devices failed: ' + resp['error'])
else:
    print('Getting devices succeeded')

devices = json.loads(r.text)
print()

#now iterate through the product locking each device ID to the new version
print('Claiming devices')
for device in devices['devices']:
    #print(device)
    #r = os.popen("particle device add " + device).read()
    #if(r.find("Successfully") == -1):
    #    print("Claiming device failed:" + r)
    #else:
    #    print("Claimed device " + device)
    r = requests.post("https://api.particle.io/v1/devices",
        data = {'id': device['id'], 'access_token':particle_key})
    resp = json.loads(r.text)
    if 'ok' in resp:
        if(resp['ok'] is False):
            if 'error' in resp:
                print('claiming device ' + device['id'] + ' failed: ' + resp['error'])
            elif 'errors' in resp:
                print('claiming device ' + device['id'] + ' failed: ' + resp['errors'][0])
        else:
            print('claiming device ' + device['id'] + ' succeeded')
    else:
        print('claiming device failed. bad response')

print()
