#include "mbed.h"
#include <stdio.h>

#include "board.h"

DigitalOut LED(PB_1);
I2CSlave i2c(PA_10, PA_9);

AnalogIn l_lv(PA_3);
AnalogIn n_lv(PA_1);
AnalogIn l_hv(PA_0);
AnalogIn n_hv(PA_2);

float l_lv_buf[200] = {0};
float n_lv_buf[200] = {0};
float l_hv_buf[200] = {0};
float n_hv_buf[200] = {0};
uint16_t voltage_pt = 0;

float second_lv[50] = {0};
uint16_t second_lv_pt = 0;
float second_hv[50] = {0};
uint16_t second_hv_pt = 0;

float minute_lv[60] = {0};
uint16_t minute_lv_pt = 0;
float minute_hv[60] = {0};
uint16_t minute_hv_pt = 0;

Timer twowave;
Timer second;
Timer comm;

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

void average_waveform(float* lv_waveform, float* hv_waveform) {
    //find the voltage of the wave
    float max_lv_v = 0;
    float min_lv_v = 0;
    float max_hv_v = 0;
    float min_hv_v = 0;
    for(uint16_t i = 0; i < 200; i++) {
        if(l_lv_buf[i] - n_lv_buf[i] > max_lv_v) {
            max_lv_v = l_lv_buf[i] - n_lv_buf[i];
        }

        if(l_lv_buf[i] - n_lv_buf[i] < min_lv_v) {
            min_lv_v = l_lv_buf[i] - n_lv_buf[i];
        }
        
        if(l_hv_buf[i] - n_hv_buf[i] > max_hv_v) {
            max_hv_v = l_hv_buf[i] - n_hv_buf[i];
        }

        if(l_hv_buf[i] - n_hv_buf[i] < min_hv_v) {
            min_hv_v = l_hv_buf[i] - n_hv_buf[i];
        }
    }

    float lv_v = max_lv_v - min_lv_v;
    float hv_v = max_hv_v - min_hv_v;

    *lv_waveform = lv_v/764.438;
    *hv_waveform = hv_v/2859; 
}

void average_second(float* lv_second, float* hv_second) {
    float tot = 0;
    for(uint8_t i = 0; i < 50; i++) {
        tot += second_lv[i];
    }

    *lv_second = tot/50;

    tot = 0;
    for(uint8_t i = 0; i < 50; i++) {
        tot += second_hv[i];
    }

    *hv_second = tot/50;
}

void average_minute(float* lv_minute, float* hv_minute) {
    float tot = 0;
    for(uint8_t i = 0; i < 60; i++) {
        tot += minute_lv[i];
    }

    *lv_minute = tot/50;

    tot = 0;
    for(uint8_t i = 0; i < 60; i++) {
        tot += minute_hv[i];
    }

    *hv_minute = tot/50;
}


int main(void) {
    //Set the i2c address
    i2c.address(0x1A);

    sample.attach_us(&sample_adc, 100);
    
    twowave.start();
    comm.start();
    second.start();
    while(1) {
        // every ~two wave periods calculate the voltage and frequency
        if(twowave.read_ms() > 20) {
           
            //average the rolling waveform buffer
            float wave_lv_voltage = 0;
            float wave_hv_voltage = 0;
            average_waveform(&wave_lv_voltage, &wave_hv_voltage);

            //store the voltage in the second pointer
            second_lv[second_lv_pt] = wave_lv_voltage;
            second_lv_pt++;
            second_hv[second_hv_pt] = wave_hv_voltage;
            second_hv_pt++;

            if(second_lv_pt >= 50) {
                second_lv_pt = 0;
            }

            if(second_hv_pt >= 50) {
                second_hv_pt = 0;
            }

            //reset the wave pointer and the timer
            twowave.reset();
        }

        if(second.read_ms() > 1000) {
            float second_lv_voltage = 0;
            float second_hv_voltage = 0;
            average_second(&second_lv_voltage, &second_hv_voltage);

            minute_lv[minute_lv_pt] = second_lv_voltage;
            minute_lv_pt++;
            minute_hv[minute_hv_pt] = second_hv_voltage;
            minute_hv_pt++;

            if(minute_lv_pt >= 60) {
                minute_lv_pt = 0;
            }

            if(minute_hv_pt >= 60) {
                minute_hv_pt = 0;
            }

            second.reset();
        }

        int i = i2c.receive();
        float lv_wave;
        float hv_wave;
        float lv_s;
        float hv_s;
        float lv_m;
        float hv_m;
        float vbuf[6];

        switch (i) {
        case I2CSlave::ReadAddressed:
            average_waveform(&lv_wave, &hv_wave);
            average_second(&lv_s, &hv_s);
            average_minute(&lv_m, &hv_m);

            vbuf[0] = lv_wave;
            vbuf[1] = hv_wave;
            vbuf[2] = lv_s;
            vbuf[3] = hv_s;
            vbuf[4] = lv_m;
            vbuf[5] = hv_m;
            i2c.write((char *)vbuf, 24);
        break;
        case I2CSlave::WriteAddressed:
        break;
        }
    }
}
