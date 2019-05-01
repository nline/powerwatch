#!/usr/bin/env python3
import sys
import yaml
import subprocess
import argparse

parser = argparse.ArgumentParser(description = 'Deploy particle firmware')
parser.add_argument('-n','--name',type=str,required=False)

args = parser.parse_args()



with open("build_settings.yaml", 'r') as stream:
    data = yaml.load(stream)

    #now for each build in the build list
    for build in data['build']:

        if args.name != None:
            if build['name'] != args.name:
                continue

        #write the product_id header file with the two defines
        header = open(build['folder'] + '/src/product_id.h', 'w')
        header.write("#define PRODUCT " + str(build['product_id']) + '\n')
        header.write('#define DEPLOYMENT "' + build['name'] + '"\n')
        header.write('#define AERIS_APN "' + build['apn'] + '"\n')
        header.close()

        #build the firmware
        subprocess.call(['particle',
                        'compile',
                        'electron',
                        build['folder'],
                        '--target',
                        '0.7.0',
                        '--saveTo',
                        build['output']+'/'+build['name']+'_'+str(build['product_id'])+'.bin'])

        #erase that header file
        header = open(build['folder'] + '/src/product_id.h', 'w')
        header.close()
