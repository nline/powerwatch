#include "mbed.h"
#include <stdio.h>
#include <math.h>
#include "board.h"

DigitalOut LED(PB_1);
I2CSlave i2c(PA_10, PA_9);

AnalogIn l_lv(PA_3);
AnalogIn n_lv(PA_1);
AnalogIn l_hv(PA_0);
AnalogIn n_hv(PA_4);

float l_lv_buf[200] = {0};
float n_lv_buf[200] = {0};
float l_hv_buf[200] = {0};
float n_hv_buf[200] = {0};
uint16_t voltage_pt = 0;

float l_l;
float n_l;
float l_h;
float n_h;

Ticker sample;

void sample_adc() {
    l_lv_buf[voltage_pt] = l_lv.read();
    n_lv_buf[voltage_pt] = n_lv.read();
    l_hv_buf[voltage_pt] = l_hv.read();
    n_hv_buf[voltage_pt] = n_hv.read();
    voltage_pt++;
    if(voltage_pt >= 200) {
        voltage_pt = 0;
    }
}

void average_lv_waveform(float* lv_waveform) {
    //find the voltage of the wave
    float max_lv_v = 0;
    for(uint16_t i = 0; i < 200; i++) {
        if(abs(l_lv_buf[i] - n_lv_buf[i]) > max_lv_v) {
            max_lv_v = abs(l_lv_buf[i] - n_lv_buf[i]);
            l_l = l_lv_buf[i];
            n_l = n_lv_buf[i];
        }
    }

    float nl = n_l*(3)*(953/3.74) - (1.5*(953/3.74));
    float ll = l_l*(3)*(953/3.74) - (1.5*(953/3.74));
    *lv_waveform = abs(ll-nl);
}

void average_hv_waveform(float* hv_waveform) {
    //find the voltage of the wave
    float max_hv_v = 0;
    for(uint16_t i = 0; i < 200; i++) {
        if(abs(l_hv_buf[i] - n_hv_buf[i]) > max_hv_v) {
            max_hv_v = abs(l_hv_buf[i] - n_hv_buf[i]);
            l_h = l_hv_buf[i];
            n_h = n_hv_buf[i];
        }
    }

    float nl = n_h*(3)*(953/1.0) - (1.5*(953/1.0));
    float ll = l_h*(3)*(953/1.0) - (1.5*(953/1.0));
    *hv_waveform = abs(ll-nl);
}



int main(void) {
    //Set the i2c address
    i2c.address(0x1A);

    sample.attach_us(&sample_adc, 300);
    
    while(1) {
        int i = i2c.receive();
        float lv_wave;
        float hv_wave;
        int32_t val[6];
        uint8_t point = 0x19;

        switch (i) {
        case I2CSlave::ReadAddressed:
            average_lv_waveform(&lv_wave);
            average_hv_waveform(&hv_wave);
            val[0] = (int32_t)(lv_wave);
            val[1] = (int32_t)(l_l*1000);
            val[2] = (int32_t)(n_l*1000);
            val[3] = (int32_t)(hv_wave);
            val[4] = (int32_t)(l_h*1000);
            val[5] = (int32_t)(n_h*1000);
            i2c.write((char *)(val), 24);
        break;
        case I2CSlave::WriteAddressed:
            i2c.read((char *)(&point), 1);
        break;
        }
    }
}
