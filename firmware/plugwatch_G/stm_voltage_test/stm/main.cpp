#include "mbed.h"
#include <stdio.h>
#include <math.h>
#include "board.h"

DigitalOut LED(PB_1);
I2CSlave i2c(PA_10, PA_9);

AnalogIn l_lv(PA_3);
AnalogIn n_lv(PA_1);

float l_lv_buf[200] = {0};
float n_lv_buf[200] = {0};
uint16_t voltage_pt = 0;

float l;
float n;

Ticker sample;

void sample_adc() {
    l_lv_buf[voltage_pt] = l_lv.read();
    n_lv_buf[voltage_pt] = n_lv.read();
    voltage_pt++;
    if(voltage_pt >= 200) {
        voltage_pt = 0;
    }
}

void average_waveform(float* lv_waveform) {
    //find the voltage of the wave
    float max_lv_v = 0;
    for(uint16_t i = 0; i < 200; i++) {
        if(abs(l_lv_buf[i] - n_lv_buf[i]) > max_lv_v) {
            max_lv_v = abs(l_lv_buf[i] - n_lv_buf[i]);
            l = l_lv_buf[i];
            n = n_lv_buf[i];
        }
    }

    float nl = n*(3)*(953/3.74) - (1.5*(953/3.74));
    float ll = l*(3)*(953/3.74) - (1.5*(953/3.74));
    *lv_waveform = abs(ll-nl);
}

int main(void) {
    //Set the i2c address
    i2c.address(0x1A);

    sample.attach_us(&sample_adc, 300);
    
    while(1) {
        int i = i2c.receive();
        float lv_wave;
        int32_t val[3];
        uint8_t point = 0x19;

        switch (i) {
        case I2CSlave::ReadAddressed:
            average_waveform(&lv_wave);
            val[0] = (int32_t)(lv_wave);
            val[1] = (int32_t)(l*1000);
            val[2] = (int32_t)(n*1000);
            i2c.write((char *)(val), 12);
        break;
        case I2CSlave::WriteAddressed:
            i2c.read((char *)(&point), 1);
        break;
        }
    }
}
