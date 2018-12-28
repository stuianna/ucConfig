/*!
    @file flashWrite.c
    @brief Source file for flashWrite.h
    @author Stuart Ianna
    @version 0.1
    @date December 2018
    @copyright GNU GPLv3
    @warning None
    @bug 

    @details

    @par Verified Compilers
    - arm-none-eabi-gcc (15:4.9.3+svn231177-1) 4.9.3 20150529 (prerelease)
*/

#include "flashWrite.h"

static void (*out)(uint8_t,uint16_t);
static uint8_t (*in)(uint16_t);

void FLASHWRITE_setOutput(void (*out_fun)(uint8_t,uint16_t)){

    out = out_fun;
    return;
}

void FLASHWRITE_setInput(uint8_t (*in_fun)(uint16_t)){

    in = in_fun;
    return;
}

v_fp_u8_u16 FLASHWRITE_getOutput(void){

    if(out == NULL){

        return NULL;
    }
    return out;
}

u8_fp_u16 FLASHWRITE_getInput(void){

    if(in == NULL){

        return NULL;
    }
    return in;
}

uint16_t FLASHWRITE_write_u8(uint8_t data, uint16_t address){

    out((uint8_t)data,address++);
    return address;
}

uint16_t FLASHWRITE_read_u8(uint8_t *data, uint16_t address){

    *data = in(address++);
    return address;
}

uint16_t FLASHWRITE_write_8(int8_t data, uint16_t address){

    out(data,address++);
    return address;
}

uint16_t FLASHWRITE_read_8(int8_t *data, uint16_t address){

    *data = (int8_t)in(address++);
    return address;
}

uint16_t FLASHWRITE_write_c(char data, uint16_t address){

    out(data,address++);
    return address;
}

uint16_t FLASHWRITE_read_c(char *data, uint16_t address){

    *data = in(address++);
    return address;
}

uint16_t FLASHWRITE_write_u16(uint16_t data, uint16_t address){

    out((uint8_t)(data>>8),address++);
    out((uint8_t)data,address++);
    return address;
}

uint16_t FLASHWRITE_read_u16(uint16_t *data, uint16_t address){

    *data = (uint16_t)(in(address++)<<8);
    *data |= in(address++);
    return address;
}

uint16_t FLASHWRITE_write_16(int16_t data, uint16_t address){

    out((uint8_t)(data>>8),address++);
    out((uint8_t)(data),address++);
    return address;
}

uint16_t FLASHWRITE_read_16(int16_t *data, uint16_t address){

    *data = (uint16_t)(in(address++)<<8);
    *data |= in(address++);
    return address;
}
uint16_t FLASHWRITE_write_u32(uint32_t data, uint16_t address){

    out((uint8_t)(data>>24),address++);
    out((uint8_t)(data>>16),address++);
    out((uint8_t)(data>>8),address++);
    out((uint8_t)data,address++);
    return address;
}

uint16_t FLASHWRITE_read_u32(uint32_t *data, uint16_t address){

    *data = (uint32_t)(in(address++)<<24);
    *data |= (uint32_t)(in(address++)<<16);
    *data |= (uint32_t)(in(address++)<<8);
    *data |= in(address++);
    return address;
}

uint16_t FLASHWRITE_write_32(int32_t data, uint16_t address){

    out((uint8_t)(data>>24),address++);
    out((uint8_t)(data>>16),address++);
    out((uint8_t)(data>>8),address++);
    out((uint8_t)(data),address++);
    return address;
}

uint16_t FLASHWRITE_read_32(int32_t *data, uint16_t address){

    *data = (uint32_t)(in(address++)<<24);
    *data |= (uint32_t)(in(address++)<<16);
    *data |= (uint32_t)(in(address++)<<8);
    *data |= in(address++);
    return address;
}

uint16_t FLASHWRITE_write_float(float f_data, uint16_t address){

    int32_t data; 
    for(int i = 0; i<MAX_DEC;i++){

        f_data *= 10;
    }

    data = (int32_t)f_data;

    out((uint8_t)(data>>24),address++);
    out((uint8_t)(data>>16),address++);
    out((uint8_t)(data>>8),address++);
    out((uint8_t)(data),address++);
    return address;
}

uint16_t FLASHWRITE_read_float(float *f_data, uint16_t address){


    int32_t data; 
    data = (uint32_t)(in(address++)<<24);
    data |= (uint32_t)(in(address++)<<16);
    data |= (uint32_t)(in(address++)<<8);
    data |= in(address++);

    *f_data = data;

    for(int i = 0; i<MAX_DEC;i++){

        *f_data /= 10;
    }

    return address;
}
