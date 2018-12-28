/*!
    @file fifo8.c
    @brief Source file for fifo8.h
    @author Stuart Ianna
    @version 1.0
    @date June 2018
    @copyright GNU GPLv3
    @warning None
    @bug 

    @details

    @par Verified Compilers
    - arm-none-eabi-gcc (15:4.9.3+svn231177-1) 4.9.3 20150529 (prerelease)
*/

#include "fifo8.h"


fifo8_error_t FIFO8_put(FIFO8 *target, uint8_t byte){

    //The fifo is full when the head is one position behind the tale
	if((target->head == ((target->tail-1) & target->mask))){

		return E_FIFO8_FULL;
	}

    //Add a byte than increase the position
	target->buffer[target->head] = byte;
	target->head++;

    //Wrap around if at end
	target->head = (target->head & target->mask);

    //Start dumping characters if need
	if((target->idle == 1) && (target->mode == FIFO8_AUTO)){

		FIFO8_get(target);
	}

    return E_FIFO8_NOERROR;

}

fifo8_error_t FIFO8_get(FIFO8 *target){

    //The fifo is empty when the head and tail are at the same position.
	if(target->head == target->tail){

		target->idle = 1;
		return E_FIFO8_EMPTY;
	}

	target->idle = 0;

    //Get a byte and wrap around if need
	target->out(target->buffer[target->tail]);
	target->tail++;
	target->tail = (target->tail & target->mask);

    return E_FIFO8_NOERROR;
}

fifo8_error_t FIFO8_init(FIFO8 *target, fifo8_mode_t mode, uint8_t *buffer, uint16_t size, void (*output)(uint8_t byte)){

    //First check if the size is a power of 2
    if(!(size && !(size & (size - 1)))){

        return E_FIFO8_NONBINARY;
    }

    //Check if the mode exists
    if((mode != FIFO8_AUTO) && (mode != FIFO8_TRIGGER) && (mode != FIFO8_DUMP)){

        return E_FIFO8_NOMODE;
    }

    //All good, set up inital values.
	target->buffer = buffer;
	target->head = 0;
	target->tail = 0;
	target->mask = size - 1;
	target->idle = 1;
	target->mode = mode;
    target->out = output;

    return E_FIFO8_NOERROR;
}

uint16_t FIFO8_size(FIFO8 *target){

	return (target->head - target->tail) & target->mask;
}

fifo8_error_t FIFO8_flush(FIFO8 *target){
	
	uint8_t test = 0;

    //The idle bit is used to determine if the fifo is empty
	while(!test){

		FIFO8_get(target);
		test = target->idle;
	}

    return E_FIFO8_NOERROR;
}
uint8_t FIFO8_pop(FIFO8 *target){

    uint8_t byte;
    //The fifo is empty when the head and tail are at the same position.
	if(target->head == target->tail){

		target->idle = 1;
		return E_FIFO8_EMPTY;
	}

	target->idle = 0;

    //Get a byte and wrap around if need
	byte = (target->buffer[target->tail]);
	target->tail++;
	target->tail = (target->tail & target->mask);

    return byte;

}

