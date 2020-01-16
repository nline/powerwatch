#include "mbed.h"
#include <stdio.h>
#include <math.h>
#include "board.h"

DigitalOut LED(PB_1);
bool led_state = false;
I2CSlave i2c(PA_10, PA_9);

AnalogIn l_lv(PA_3);
AnalogIn n_lv(PA_1);
AnalogIn l_hv(PA_0);
AnalogIn n_hv(PA_4);

float l_lv_buf[2][300] = {0};
float n_lv_buf[2][300] = {0};
float l_hv_buf[2][300] = {0};
float n_hv_buf[2][300] = {0};
uint16_t voltage_pt = 0;
uint16_t ending_voltage_pt = 0;
uint8_t buffer = 0;
bool l_higher = true;
bool process_waveform_flag = false;
uint8_t buffer_to_process = 0;

float l_l;
float n_l;
float l_h;
float n_h;

Ticker sample;
Ticker second;
bool one_second = false;
uint8_t minute_counter = 0;
bool one_minute = false;

//These are the buffers that store the by minute, by second and waveform voltages

//60 minutes worth of averages
float lv_minute[60] = {0};
float hv_minute[60] = {0};

//60 seconds worth of averages
float lv_second[60] = {0};
float hv_second[60] = {0};

//up to 60 waveforms worth of averages
float lv_waveform[60] = {0};
float hv_waveform[60] = {0};
uint8_t waveform_pointer;

int32_t lv_int32_minute[60] = {0};
int32_t outage_int32[60] = {0};

void convert_lv_int32() {
    for(uint8_t i = 0; i < 60; i++) {
        lv_int32_minute[i] = (int32_t)lv_minute[i]*1000;
    }
}

void convert_outage_int32() {

}

//Okay the strategy is sample periodically into a sufficiently large sample buffer
//when a zero crossing is detected (i.e. the polarities of the leads switch)
//set a flag and store into the other double buffer
//in the main loop process that and place it in the waveform buffer
//set a timer for 1 secondand one minute that set flags
//when those flags are set in the main loop copy forware to keep propagating info

void sample_adc() {
    l_lv_buf[buffer][voltage_pt] = l_lv.read();
    n_lv_buf[buffer][voltage_pt] = n_lv.read();
    l_hv_buf[buffer][voltage_pt] = l_hv.read();
    n_hv_buf[buffer][voltage_pt] = n_hv.read();

    if(l_lv_buf[buffer][voltage_pt] > n_lv_buf[buffer][voltage_pt] && !l_higher) {
        l_higher = true;
        buffer = 1;
        process_waveform_flag = true;
        buffer_to_process = 0;
        ending_voltage_pt = voltage_pt;
        voltage_pt = 0;
    } else if (l_lv_buf[buffer][voltage_pt] < n_lv_buf[buffer][voltage_pt] && l_higher) {
        l_higher = false;
        buffer = 0;
        process_waveform_flag = true;
        buffer_to_process = 1;
        ending_voltage_pt = voltage_pt;
        voltage_pt = 0;
    } else {
        voltage_pt++;
        if(voltage_pt >= 300) {
            voltage_pt = 0;
        }
    }
}

float average_lv_waveform(uint8_t buffer, uint16_t end_point) {
    //find the voltage of the wave
    float max_lv_v = 0;
    for(uint16_t i = 0; i < end_point; i++) {
        if(abs(l_lv_buf[buffer][i] - n_lv_buf[buffer][i]) > max_lv_v) {
            max_lv_v = abs(l_lv_buf[buffer][i] - n_lv_buf[buffer][i]);
            l_l = l_lv_buf[buffer][i];
            n_l = n_lv_buf[buffer][i];
        }
    }

    float nl = n_l*(3)*(953/3.74) - (1.5*(953/3.74));
    float ll = l_l*(3)*(953/3.74) - (1.5*(953/3.74));
    return abs(ll-nl);
}

float average_hv_waveform(uint8_t buffer, uint16_t end_point) {
    //find the voltage of the wave
    float max_hv_v = 0;
    for(uint16_t i = 0; i < end_point; i++) {
        if(abs(l_hv_buf[buffer][i] - n_hv_buf[buffer][i]) > max_hv_v) {
            max_hv_v = abs(l_hv_buf[buffer][i] - n_hv_buf[buffer][i]);
            l_h = l_hv_buf[buffer][i];
            n_h = n_hv_buf[buffer][i];
        }
    }

    float nl = n_h*(3)*(953/1.0) - (1.5*(953/1.0));
    float ll = l_h*(3)*(953/1.0) - (1.5*(953/1.0));
    return abs(ll-nl);
}

void second_timer() {
    one_second = true;
    minute_counter++;

    if(minute_counter >= 60) {
        one_minute = true;
        minute_counter = 0;
    }

    led_state = ~led_state;
    LED = led_state;
}

int main(void) {
    //Set the i2c address
    i2c.address(0x1A);

    sample.attach_us(&sample_adc, 100);
    second.attach_us(&second_timer, 1000000);
    
    while(1) {
        if(process_waveform_flag) {
            lv_waveform[waveform_pointer] = average_lv_waveform(buffer, ending_voltage_pt);
            hv_waveform[waveform_pointer] = average_hv_waveform(buffer, ending_voltage_pt);
            waveform_pointer++;
            process_waveform_flag = false;
        }

        if(one_second) {
            for(uint8_t i = 59; i > 0; i--) {
                lv_second[i] = lv_second[i-1];
                hv_second[i] = hv_second[i-1];
            }
            
            float lv_accum = 0;
            float hv_accum = 0;
            for(uint8_t i = 0; i < waveform_pointer; i++) {
                lv_accum += lv_waveform[i];
                hv_accum += hv_waveform[i];
            }

            lv_second[0] = lv_accum/(waveform_pointer-1);
            hv_second[0] = hv_accum/(waveform_pointer-1);
            waveform_pointer = 0;
            one_second = false;
        }

        if(one_minute) {
            for(uint8_t i = 59; i > 0; i--) {
                lv_minute[i] = lv_minute[i-1];
                hv_minute[i] = hv_minute[i-1];
            }
            
            float lv_accum = 0;
            float hv_accum = 0;
            for(uint8_t i = 0; i < 60; i++) {
                lv_accum += lv_second[i];
                hv_accum += hv_second[i];
            }

            lv_minute[0] = lv_accum/60;
            hv_minute[0] = hv_accum/60;
            one_minute = false;
        }

        int i = i2c.receive();
        uint8_t point = 0x19;

        switch (i) {
        case I2CSlave::ReadAddressed:
            if(point == 0x20) {
                convert_lv_int32();
                i2c.write((char *)lv_int32_minute, 60);
            } else if (point == 0x30) {
                convert_outage_int32();
                i2c.write((char *)outage_int32, 60);
            } else {
                i2c.write("0", 1);
            }
        break;
        case I2CSlave::WriteAddressed:
            i2c.read((char *)(&point), 1);
        break;
        }
    }
}
