#!/usr/bin/env python3

from sty import fg, bg, ef, rs
import os
import sys
import requests
import argparse
import json
import yaml
import psycopg2
import math
from google.cloud import secretmanager
import warnings
warnings.filterwarnings("ignore", "Your application has authenticated using end user credentials")

parser = argparse.ArgumentParser(description = 'Test powerwatch firmware')
parser.add_argument('-d','--deployment',type=str,required=False)

voltage = 120

def print_intro(loction, device_id):
    print()
    print()
    print("############################")
    print("Powerwatch testing script")
    print("Device ID: " + device_id)
    print("Device Location: " + location)
    print("############################")
    print()
    print()

def getTestCredentials(location):
    ## Create the Secret Manager client. Suppress warning
    client = secretmanager.SecretManagerServiceClient()

    try:
        response = client.access_secret_version(request={"name": "projects/powerwatch-backend/secrets/" + location + "-values/versions/latest"})
    except:
        print("{}".format(bg.li_red))
        print("Error accessing database credentials for devices in " + location)
        print("{}".format(bg.rs))
        raise RuntimeError

    #parse as yaml
    payload = response.payload.data.decode("UTF-8")
    location_dct = yaml.safe_load(payload)

    try:
        response = client.access_secret_version(request={"name": "projects/powerwatch-backend/secrets/aeris-token/versions/latest"})
    except:
        print("{}".format(bg.li_red))
        print("Error accessing aeris token credentials")
        print("{}".format(bg.rs))
        raise RuntimeError

    #parse as yaml
    aerisToken = response.payload.data.decode("UTF-8").strip()

    return location_dct['powerwatch-timescale']['database']['readOnlyUsername'], location_dct['powerwatch-timescale']['database']['readOnlyPassword'], location_dct['powerwatch-postgres-poster']['particle']['authToken'], location_dct['powerwatch-postgres-poster']['particleConfig']['product_ids'], aerisToken
    

if __name__ == '__main__':

    args = parser.parse_args()

    while True:
        print()
        print("Waiting for device ID. Please enter or scan device ID....")
        #receive input and get the domain and device ID for this query
        received = input("Enter the device ID to test: ").strip()
        if received.find(':') != -1:
            device_id = received.split(':')[1]
            location = None
            if len(received.split('.')) > 1:
                location = received.split('.')[0]
            elif args.deployment:
                location = args.deployment
            
            if location is None:
                print("Device location not encoded in QR Code. Please input location with flag './test.py -d [deployment_locaiont]'")
                continue

            if device_id is None:
                print("Invalid device ID. Retry")
                continue
        else:
            print("Invalid device ID. Retry")
            continue

        print_intro(location, device_id)

        #get the secret values for this location
        try:
            user, password, particle_key, product_ids, aerisToken = getTestCredentials(location)
        except RuntimeError as e:
            continue

        #login to the database
        connection = psycopg2.connect(dbname='powerwatch', user=user, host='timescale.' + location + '.powerwatch.io', password=password)
        cursor = connection.cursor();

        r = requests.post("https://api.particle.io/v1/devices",
            data = {'id': device_id, 'access_token':particle_key})
        resp = json.loads(r.text)
        if 'ok' in resp:
            if(resp['ok'] is False):
                if 'error' in resp:
                    print('{}Claiming device failed'.format(bg.li_red,bg.rs))
                elif 'errors' in resp:
                    print('{}Claiming device failed'.format(bg.li_red,bg.rs))
            else:
                pass
        else:
            print('{}Claiming device failed. Bad response{}'.format(bg.li_red,bg.rs))

        r = requests.get("https://api.particle.io/v1/devices/" + device_id +"?access_token=" + particle_key)
        resp = json.loads(r.text)
        if 'error' in resp:
            print("{}Error getting device information - maybe it hasn't connected to the cloud?{}".format(bg.li_red,bg.rs))
            print("{}".format(bg.li_red))
            print(resp)
            print("{}".format(bg.rs))
            continue

        if 'iccid' not in resp:
            print("{}Error - device has never connected to cloud - check the antenna.{}".format(bg.li_red,bg.rs))
            continue

        r = requests.get("https://api.particle.io/v1/products/" + str(resp['product_id']) + "/devices?access_token=" + particle_key + '&perPage=10000&sortAttr=firmwareVersion&sortDir=desc')
        devices = json.loads(r.text)

        #find the device that mateches our device
        final_device = None
        for device in devices['devices']:
            if device['id'] == device_id:
                final_device = device
                break

        r = requests.post("https://aeradminapi.aeris.com/AerAdmin_WS_5_0/rest/devices/details?apiKey=" + aerisToken,
             json = {'accountID':23963,
                    'email':'josh@nline.io',
                    'ICCID': resp['iccid']
                    })
        sim = json.loads(r.text)


        #update the postgres devices table
        #check to see if the device already exists
        cursor.execute("SELECT DISTINCT time, core_id, is_powered, last_unplug_millis, last_plug_millis, sd_present, sd_log_size," + 
                        "grid_voltage, grid_frequency, shield_id, state_of_charge, cell_voltage, die_temperature, x_acceleration, y_acceleration, z_acceleration, dequeue_size  from powerwatch where core_id = %s AND time < NOW() AND time > NOW() - INTERVAL '2 hours' AND time_source = 'particle' ORDER BY time ASC",(device_id,))
        result = cursor.fetchall()

        print()
        if resp['product_id'] in product_ids:
            print("Product ID: {}{}{}".format(bg.li_green,resp['product_id'],bg.rs))
        else:
            print("Product ID: {}{}{}".format(bg.li_red,resp['product_id'],bg.rs))

        #if final_device['firmware_version'] == config['firmwareVersion']:
        #    print("Firmware: {}{}{}".format(bg.li_green,final_device['firmware_version'],bg.rs))
        #else:
        #    print("Firmware: {}{}{}".format(bg.li_red,final_device['firmware_version'],bg.rs))

        if final_device['owner'] == "nklugman@berkeley.edu":
            print("Owner: {}{}{}".format(bg.li_green,final_device['owner'],bg.rs))
        else:
            print("Owner: {}{}{}".format(bg.li_red,final_device['owner'],bg.rs))


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

        if len(result) == 0 or len(result) == 1:
            print("{}No recent packets{}".format(bg.li_red,bg.rs))
            continue

        if len(result) > 50:
            print("Packets received last 12h: {}{}/60{} expected".format(bg.li_green,len(result),bg.rs))
        else:
            print("Packets received last 12h: {}{}/60{} expected".format(bg.li_red,len(result),bg.rs))
       
        if result[-1][16] is None:
            print("{}Error reading dequeque size. No recent packets{}".format(bg.li_red,bg.rs))
            continue

        dequeue_size = int(result[-1][16]/170)
        if dequeue_size + len(result) > 50:
            print("Current dequeue size: {}{}/{}{} expected".format(bg.li_green,dequeue_size,60-len(result),bg.rs))
        else:
            print("Current dequeue size: {}{}/{}{} expected".format(bg.li_red,dequeue_size,60-len(result),bg.rs))
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

        x = result[-1][13]
        y = result[-1][14]
        z = result[-1][15]
        pitch = (math.atan2(-1*x,math.sqrt(y**2 + z**2)) * 180/math.pi)
        sign = 1
        if z <= 0:
            sign = -1
        roll = (math.atan2(y, sign * math.sqrt(z**2 + 0.001*x**2)) * 180/math.pi) 

        if(pitch > -5 and pitch < 5 and roll < -5 and roll > -35):
            print("{}Last accel orientation{}\tpitch: {:.2f}\troll: {:.2f}".format(bg.li_green,bg.rs,pitch,roll))
        else:
            print("{}Last accel orientation{}\tpitch: {:.2f}\troll: {:.2f}".format(bg.li_red,bg.rs,pitch,roll))

        if roll < -135 and roll > -136 and pitch > 35 and pitch < 36:
            print("{}Accelerometer I2C bus hung{}".format(bg.li_red,bg.rs))
            continue

        print()
        if(avg_soc/len(result) < 100 and avg_soc/len(result) > 50 and avg_cv/len(result) > 3.7 and avg_cv/len(result) < 4.15):
            print("{}State of Charge{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_green,bg.rs,avg_soc/len(result), minimum_soc, maximum_soc))
            print("{}Battery Voltage{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_green,bg.rs,avg_cv/len(result), minimum_cv, maximum_cv))
        else:
            print("{}State of Charge{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_red,bg.rs,avg_soc/len(result), minimum_soc, maximum_soc))
            print("{}Battery Voltage{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_red,bg.rs,avg_cv/len(result), minimum_cv, maximum_cv))

        print()
        print("Powered: " + str(powered) + "/" + str(len(result)) + " packets")

        if(powered == 0):
            print("{}Insufficient messages in powered state to check voltage sensing circuit{}".format(bg.li_red,bg.rs))
        else:
            if(avg_v/powered/1.414 < voltage+20 and avg_v/powered/1.414 > voltage-20 and maximum_v/1.414 < voltage+40 and minimum_v/1.414 > voltage-40):
                print("{}Voltage{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_green,bg.rs,avg_v/powered/1.414, minimum_v/1.414, maximum_v/1.414))
            else:
                print("{}Voltage{} \taverage: {:.2f} \tmin: {:.2f} \tmax: {:.2f}".format(bg.li_red,bg.rs,avg_v/powered/1.414, minimum_v/1.414, maximum_v/1.414))

            if((avg_f/powered > 49 and avg_f/powered < 51 and maximum_f < 55 and minimum_f > 45) or (avg_f/powered > 59 and avg_f/powered < 61 and maximum_f < 65 and minimum_f > 55)):
                print("{}Frequency{} \taverage: {:.2f} \t\tmin: {:.2f} \tmax: {:.2f}".format(bg.li_green,bg.rs,avg_f/powered, minimum_f, maximum_f))
            else:
                print("{}Frequency{} \taverage: {:.2f} \t\tmin: {:.2f} \tmax: {:.2f}".format(bg.li_red,bg.rs,avg_f/powered, minimum_f, maximum_f))

        #close the connection
        connection.close()
