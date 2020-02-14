#! /usr/bin/env python3

from sty import fg, bg, ef, rs
import os
import sys
import requests
import argparse
import json
import yaml
import psycopg2

config_file = open('test-config.json','r')
config = yaml.safe_load(config_file)


def print_intro(config):
    print()
    print()
    print("############################")
    print("Powerwatch testing script")
    print("domain: " + config['postgresHost'])
    print("############################")
    print()
    print()
    print("Waiting for particle ID. Please enter or scan particle ID....")

if __name__ == '__main__':

    #get the key from the particle file/login
    key_file = open(os.environ['HOME'] + '/.particle/particle.config.json','r')
    particle_key = None
    try:
        keys = json.loads(key_file.read())
        particle_key = keys['access_token']
    except:
        print("Install particle CLI and login with 'particle login' to generate a key file. Exiting.")
        sys.exit(1)

    print_intro(config)

    state = "WAIT_FOR_ID"
    while True:
        device_id = input("Enter the device ID to test: ").strip()

        r = requests.get("https://api.particle.io/v1/devices/" + device_id +"?access_token=" + particle_key)
        resp = json.loads(r.text)
        if 'ok' in resp:
            if(resp['ok'] is False):
                print('Getting devices failed: ' + resp['error'])
                continue

        r = requests.get("https://api.particle.io/v1/products/" + str(resp['product_id']) + "/devices?access_token=" + particle_key + '&perPage=10000&sortAttr=firmwareVersion&sortDir=desc')
        devices = json.loads(r.text)

        #find the device that mateches our device
        final_device = None
        for device in devices['devices']:
            if device['id'] == device_id:
                final_device = device
                break

      
        r = requests.post("https://aeradminapi.aeris.com/AerAdmin_WS_5_0/rest/devices/details?apiKey=" + config['aerisToken'],
             json = {'accountID':23963,
                    'email':'josh@nline.io',
                    'ICCID': resp['iccid']
                    })
        sim = json.loads(r.text)

        #now query the database
        connection = psycopg2.connect(dbname=config['postgresDatabase'], user=config['postgresUser'], host=config['postgresHost'], password=config['postgresPassword'])
        cursor = connection.cursor();

        #update the postgres devices table
        #check to see if the device already exists
        cursor.execute("SELECT time, core_id, is_powered, last_unplug_millis, last_plug_millis, sd_present, sd_log_size," + 
                        "grid_voltage, grid_frequency, shield_id, state_of_charge, cell_voltage, die_temperature  from powerwatch where core_id = %s AND time < NOW() AND time > NOW() - INTERVAL '12 hours' ORDER BY time ASC",(device_id,))
        result = cursor.fetchall()

        print()
        if resp['product_id'] == 10804:
            print("Product ID: {}{}{}".format(bg.li_green,resp['product_id'],bg.rs))
        else:
            print("Product ID: {}{}{}".format(bg.li_red,resp['product_id'],bg.rs))

        if final_device['firmware_version'] == 208:
            print("Firmware: {}{}{}".format(bg.li_green,final_device['firmware_version'],bg.rs))
        else:
            print("Firmware: {}{}{}".format(bg.li_red,final_device['firmware_version'],bg.rs))

        print("ICCID: " + resp['iccid'])

        if sim['deviceAttributes'][0]['serviceName'] == 'GRIDWATCH_ROW':
            print("Cell service: {}{}{}".format(bg.li_green,sim['deviceAttributes'][0]['serviceName'],bg.rs))
        else:
            print("Cell service: {}{}{}".format(bg.li_red,sim['deviceAttributes'][0]['serviceName'],bg.rs))

        if sim['deviceAttributes'][0]['deviceStatus'] == 'Bill':
            print("Cell Status: {}{}{}".format(bg.li_green,sim['deviceAttributes'][0]['deviceStatus'],bg.rs))
        else:
            print("Cell Status: {}{}{}".format(bg.li_red,sim['deviceAttributes'][0]['deviceStatus'],bg.rs))

        print()
        print("Data returned in last 12 hours:")
        print("###############################")

        if len(result) > 350:
            print("Packets received: {}{}/360{} expected".format(bg.li_green,len(result),bg.rs))
        else:
            print("Packets received: {}{}/360{} expected".format(bg.li_red,len(result),bg.rs))
        print()

       
        sd_present = True
        for r in result:
            if r[5] == False:
                sd_present = False

        if sd_present:
            print("SD card: {}PRESENT{}".format(bg.li_green,bg.rs))
        else:
            print("SD card: {}ABSENT - ERROR{}".format(bg.li_red,bg.rs))

        diff = 0
        count = 0
        r1 = -1
        for r in result:
            if(r1 == -1):
                r1 = r[6]
            else:
                count += 1
                if(r[6] >= r1):
                    diff += r[6] - r1
                r1 = r[6]

        if diff/count > 150 and diff/count < 250:
            print("SD increased by: {}{}{} bytes per packet".format(bg.li_green,int(diff/count),bg.rs))
        else:
            print("SD increased by: {}{}{} bytes per packet".format(bg.li_red,int(diff/count),bg.rs))

        avg_soc = 0
        minimum_soc = 10000
        maximum_soc = 0
        avg_cv = 0
        minimum_cv = 10000
        maximum_cv = 0
        avg_dt = 0
        minimum_dt = 10000
        maximum_dt = 0

        avg_v = 0
        minimum_v = 10000
        maximum_v = 0
        avg_f = 0
        minimum_f = 10000
        maximum_f = 0
        powered = 0
        count = 0
        for r in result:
            avg_soc += r[10]
            avg_cv += r[11]
            dt = int(r[12]) >> 7
            avg_dt += dt
            if r[10] < minimum_soc:
                minimum_soc = r[10]
            if r[10] > maximum_soc:
                maximum_soc = r[10]
            if r[11] < minimum_cv:
                minimum_cv = r[11]
            if r[11] > maximum_cv:
                maximum_cv = r[11]
            if dt < minimum_dt:
                minimum_dt = dt
            if dt > maximum_dt:
                maximum_dt = dt

            if r[2]:
                powered += 1
                avg_v += r[7]
                avg_f += r[8]
                if r[7] < minimum_v:
                    minimum_v = r[7]
                if r[7] > maximum_v:
                    maximum_v = r[7]
                if r[8] < minimum_f:
                    minimum_f = r[8]
                if r[8] > maximum_f:
                    maximum_f = r[8]
        
        print()
        if(avg_dt/len(result) < 40 and avg_dt/len(result) > 15):
            print("{}Accelerometer Temp{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_green,bg.rs,avg_dt/len(result), minimum_dt, maximum_dt))
        else:
            print("{}Accelerometer Temp{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_red,bg.rs,avg_dt/len(result), minimum_dt, maximum_dt))

        print()
        if(avg_soc/len(result) < 100 and avg_soc/len(result) > 50 and avg_cv/len(result) > 3.7 and avg_cv/len(result) < 4.15):
            print("{}State of Charge{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_green,bg.rs,avg_soc/len(result), minimum_soc, maximum_soc))
            print("{}Battery Voltage{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_green,bg.rs,avg_cv/len(result), minimum_cv, maximum_cv))
        else:
            print("{}State of Charge{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_red,bg.rs,avg_soc/len(result), minimum_soc, maximum_soc))
            print("{}Battery Voltage{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_red,bg.rs,avg_cv/len(result), minimum_cv, maximum_cv))

        print()
        print("Powered: " + str(powered) + "/" + str(len(result)) + " packets")

        if(avg_v/powered/1.414 < 250 and avg_v/powered/1.414 > 200 and maximum_v/1.414 < 260 and minimum_v/1.414 >180):
            print("{}Voltage{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_green,bg.rs,avg_v/powered/1.414, minimum_v/1.414, maximum_v/1.414))
        else:
            print("{}Voltage{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_red,bg.rs,avg_v/powered/1.414, minimum_v/1.414, maximum_v/1.414))

        if(avg_f/powered > 49 and avg_f/powered < 51 and maximum_f < 55 and minimum_f > 45):
            print("{}Frequency{} \taverage: {:.2f} \t\tmin: {:.2f} \tmax: {:.2f}".format(bg.li_green,bg.rs,avg_f/powered, minimum_f, maximum_f))
        else:
            print("{}Frequency{} \taverage: {:.2f} \t\tmin: {:.2f} \tmax: {:.2f}".format(bg.li_red,bg.rs,avg_f/powered, minimum_f, maximum_f))

        print_intro(config)

