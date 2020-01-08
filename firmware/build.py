#!/usr/bin/env python3
import sys
import yaml
import subprocess
import argparse

parser = argparse.ArgumentParser(description = 'Compile firmware with product definition variable')
parser.add_argument('-f','--firmware',type=str,required=True)
parser.add_argument('-p','--product',type=str,required=False)
parser.add_argument('-o','--output',type=str,required=False)

args = parser.parse_args()

#write the product_id header file with the two defines
if args.product:
    header = open(args.firmware + '/src/product_id.h', 'w')
    header.write("#define PRODUCT " + str(args.product) + '\n')
    header.close()
else:
    pass

#build the firmware
if args.output:
    subprocess.call(['particle',
                    'compile',
                    'electron',
                    args.firmware,
                    '--target',
                    '0.7.0',
                    '--saveTo',
                    args.output])
else:
    subprocess.call(['particle',
                    'compile',
                    'electron',
                    args.firmware,
                    '--target',
                    '0.7.0',
                    '--saveTo',
                    args.firmware + '/' + args.firmware.split('/')[-1] + '.bin'])


#erase that header file
header = open(args.firmware + '/src/product_id.h', 'w')
header.close()
