#!/usr/bin/env python

from PIL import Image, ImageDraw, ImageFont, ImageOps
import pyqrcode
import argparse
import glob
import yaml
import json
import requests
import re
import sys
import os
import time
import pyscreen
import serial
import datetime
import struct
import yaml

shield_fnt = ImageFont.truetype('/Library/Fonts/Arial.ttf', 14)
case_fnt = ImageFont.truetype('/Library/Fonts/Arial.ttf', 30)
case_fnt_larger = ImageFont.truetype('/Library/Fonts/Arial.ttf', 40)
case_fnt_small = ImageFont.truetype('/Library/Fonts/Arial.ttf', 20)
case_fnt_bold = ImageFont.truetype('/Library/Fonts/Arial Bold.ttf', 30)
case_fnt_l = ImageFont.truetype('/Library/Fonts/Arial.ttf', 60)

parser = argparse.ArgumentParser(description = 'Deploy particle firmware')
parser.add_argument('-c','--core', type=str, required=True)
parser.add_argument('-s','--shield', type=str, required=True)

args = parser.parse_args()

#make the gen folder if it doesn't exist
if(not os.path.isdir('./gen')):
    os.mkdir('./gen')


print("")
print("Printing stickers...")
def print_small(particle_id,shield_id):

    img = Image.new('L', (100, 450), "white")
    qr_basewidth = 80
    big_code = pyqrcode.create('F:' + particle_id+':'+shield_id, error='L', version=4)
    big_code.png('code.png', scale=12, module_color=[0, 0, 0, 128], background=[0xff, 0xff, 0xff, 0xff])
    qr_img = Image.open('code.png','r')
    qr_img = qr_img.convert("RGBA")
    qr_wpercent = (qr_basewidth/float(qr_img.size[0]))
    qr_hsize = int((float(qr_img.size[1])*float(qr_wpercent)))
    qr_img = qr_img.resize((qr_basewidth,qr_hsize), Image.ANTIALIAS)
    qr_img_w, qr_img_h = qr_img.size
    qr_img=qr_img.rotate(90,expand=1)
    img.paste(qr_img,(0,150))#,qr_img_w,qr_img_h))
    img.paste(qr_img,(0,370))#,qr_img_w,qr_img_h))

    txt = Image.new('L',(225,15),"white")
    d = ImageDraw.Draw(txt)
    d.text( (0,0), particle_id, font=shield_fnt, fill=0)
    w=txt.rotate(90, expand=1)
    img.paste(w, (70,220))
    img.paste(w, (70,0))

    txt = Image.new('L',(225,15),"white")
    d = ImageDraw.Draw(txt)
    d.text( (0,0), shield_id, font=shield_fnt, fill=0)
    w=txt.rotate(90, expand=1)
    img.paste(w, (85,220))
    img.paste(w, (85,0))


    img.save('./gen/small_logo_gen.png')
    os.system('brother_ql_create --model QL-800 ./gen/small_logo_gen.png -r 90 > ./gen/small_'+str(particle_id) + ":" +
                str(shield_id) +'.bin')
    os.system('brother_ql_print --backend pyusb ./gen/small_'+str(particle_id) + ":" + str(shield_id) + '.bin')

def print_case(particle_id, shield_id):
    img = Image.new('L', (420, 630), "white")
    qr_basewidth = 350
    big_code = pyqrcode.create('F:' + particle_id + ":" + shield_id, error='L', version=4)
    big_code.png('code.png', scale=12, module_color=[0, 0, 0, 128], background=[0xff, 0xff, 0xff, 0xff])
    qr_img = Image.open('code.png','r')
    qr_img = qr_img.convert("RGBA")
    qr_wpercent = (qr_basewidth/float(qr_img.size[0]))
    qr_hsize = int((float(qr_img.size[1])*float(qr_wpercent)))
    qr_img = qr_img.resize((qr_basewidth,qr_hsize), Image.ANTIALIAS)
    qr_img_w, qr_img_h = qr_img.size
    qr_img=qr_img.rotate(90,expand=1)
    img.paste(qr_img,(35,45))#,qr_img_w,qr_img_h))

    txt = Image.new('L',(500,35), "white")
    d = ImageDraw.Draw(txt)
    d.text( (0,0), 'F:' + particle_id, font=case_fnt_small, fill=0)
    w,h = d.textsize('F:' + particle_id, font=case_fnt_small)
    img.paste(txt,((420-w)/2,425))

    txt2 = Image.new('L',(500,35),"white")
    d = ImageDraw.Draw(txt2)
    d.text( (0,0), shield_id, font=case_fnt_larger, fill=0)
    w,h = d.textsize(shield_id, font=case_fnt_larger)
    img.paste(txt2, ((420-w)/2,380))

    txt7 = Image.new('L',(500,35),"white")
    d = ImageDraw.Draw(txt7)
    d.text( (0,0), "DO NOT UNPLUG", font=case_fnt_bold, fill=0)
    w,h = d.textsize("DO NOT UNPLUG", font=case_fnt_bold)
    img.paste(txt7, ((420-w)/2,470))

    txt10 = Image.new('L',(500,35),"white")
    d = ImageDraw.Draw(txt10)
    d.text( (0,0), "KEEP OUTLET ON", font=case_fnt_bold, fill=0)
    w,h = d.textsize("KEEP OUTLET ON", font=case_fnt_bold)
    img.paste(txt10, ((420-w)/2,505))

    txt8 = Image.new('L',(500,35),"white")
    d = ImageDraw.Draw(txt8)
    d.text( (0,0), "Call Kwame for questions:", font=case_fnt, fill=0)
    w,h = d.textsize("Call Kwame for questions:", font=case_fnt)
    img.paste(txt8, ((420-w)/2,560))

    txt9 = Image.new('L',(500,35),"white")
    d = ImageDraw.Draw(txt9)
    d.text( (0,0), "+233 024-653-6896", font=case_fnt, fill=0)
    w,h = d.textsize("+233 024-653-6896", font=case_fnt)
    img.paste(txt9, ((420-w)/2,595))

    txt3 = Image.new('L',(500,80),"white")
    d = ImageDraw.Draw(txt3)
    d.text( (0,0), "DumsorWatch", font=case_fnt_larger, fill=0)
    w,h = d.textsize("DumsorWatch", font=case_fnt_larger)
    img.paste(txt3, ((420-w)/2,0))

    img.save('./gen/case_logo_gen.png')
    os.system('brother_ql_create --model QL-800 ./gen/case_logo_gen.png -r 90 > ./gen/'+str(particle_id) + ":" + str(shield_id)+'.bin')
    os.system('brother_ql_print --backend pyusb ./gen/'+str(particle_id) + ":" + str(shield_id) + '.bin')

print_case(args.core,args.shield)
print_small(args.core,args.shield)
