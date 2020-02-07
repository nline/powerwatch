#include "mbed.h"
#include <stdio.h>

#include "board.h"

Serial pc(PB_6, PB_7);
DigitalIn i2c1(PA_9);
DigitalIn i2c2(PA_10);
DigitalOut LED(PB_1);

Timer t;

FlashIAP flash;

int main(void) {
    //After boot if both I2C lines are low for one second then we are trying
    //to firmware update, otherwise jump to the main application

    LED = 1;
    t.start();
    while(t.read_ms() < 1000) {
        if(i2c1 == 1 || i2c2 == 1) {
            //start the application
            mbed_start_application(POST_APPLICATION_ADDR);
        }
    }

    //okay we should go into bootloader mode
    //Wait for serial bytes and write them to memory at the start location
    LED = 0;

    flash.init();

    const uint32_t page_size = flash.get_page_size();
    char *page_buffer = new char[page_size];
    uint32_t addr = POST_APPLICATION_ADDR;
    uint32_t next_sector = addr + flash.get_sector_size(addr);
    bool sector_erased = false;
    uint32_t page_offset = 0;
    uint32_t total_bytes = 0;
    uint32_t program_size = 0;

    //set the data for this page to zero
    memset(page_buffer, 0, sizeof(char) * page_size);

    //read the program size
    uint8_t buf[4];
    for(uint8_t i = 0; i < 4; i++) {
        while(!pc.readable());
        buf[i] = pc.getc();
    }

    memcpy(&program_size, buf, 4);
    printf("Program size is %d", program_size);

    while (true) {
            //wait for serial bytes
            if(pc.readable()) {
                char byte = pc.getc();
                page_buffer[page_offset] = byte;
                page_offset++;
                total_bytes++;

                //have we written the full page?
                if(page_offset == page_size || total_bytes == program_size) {

                    //zero the page offset
                    page_offset = 0;

                    // Erase this page if it hasn't been erased
                    if (!sector_erased) {
                        flash.erase(addr, flash.get_sector_size(addr));
                        sector_erased = true;
                    }

                    // Program page
                    flash.program(page_buffer, addr, page_size);
                    
                    //increment addr and track page
                    addr += page_size;
                    if (addr >= next_sector) {
                        next_sector = addr + flash.get_sector_size(addr);
                        sector_erased = false;
                    }

                    //set the back to zero
                    memset(page_buffer, 0, sizeof(char) * page_size);

                    if(total_bytes == program_size) {

                        delete[] page_buffer;
                        flash.deinit();
                        printf("Done flashing program");
                        mbed_start_application(POST_APPLICATION_ADDR);
                    }
                }

        }
    }
}
