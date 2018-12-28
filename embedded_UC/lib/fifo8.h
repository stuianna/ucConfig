/**
    @addtogroup COMMON
* @{
*/

/**
    @addtogroup FIFO8
    @brief 8-Bit circular FIFO software buffer. 
    @details 

    @author Stuart Ianna
    @version 0.11
    @date December 2018
    @copyright GNU GPLv3
    @warning None
    @bug None
    @todo
        

    @par Verified Compilers
    - arm-none-eabi-gcc (15:4.9.3+svn231177-1) 4.9.3 20150529 (prerelease)

    @par Example - Auto sending via USART transmit
    @include example_fifo8_auto.c

    @par Example - Triggerd reading via USART receive
    @include example_fifo8_trigger.c

 * @{
 */

/*! @file fifo8.h
    @brief Header file for 8-bit circular FIFO software buffer.
*/

#ifndef FIFO8_H
#define FIFO8_H

#include <stdio.h>

/*! 
    @brief Operating modes for the buffer.
    @details These values are used with function 
*/
typedef enum{

    FIFO8_AUTO, 	        //!<Fifo contents flushed to output function ASAP
    FIFO8_TRIGGER,  	    //!<Fifo content flushed to output function on call FIFO_flush()
    FIFO8_DUMP,	        //!<Fifo contents flushed to output function when buffer is full
}fifo8_mode_t;

/*! 
    @brief Error codes returned by function calls
    @details 
*/
typedef enum{

    E_FIFO8_NOERROR,         //!<No error.
    E_FIFO8_NOMODE,          //!<The FIFO mode doesn't exist.
    E_FIFO8_NONBINARY, 	    //!<The size of the FIFO buffer is not a power of 2.
    E_FIFO8_FULL,  	        //!<The FIFO buffer is full.
    E_FIFO8_EMPTY,	        //!<The FIFO buffer is empty.
}fifo8_error_t;

/*! 
    @brief Data structure for FIFO8 perihperal interface.
    @details 

    This structure containts variables needed by each instance of of the FIFO8 buffer.

    Values contained in this structure do not need to be modified manually, they are managed by function calls.

    A new instance of this structure must be initialized by FIFO8_setup().
*/
typedef struct{

	uint8_t *buffer;                //!<Pointer to the FIFO8 memory buffer.
	uint16_t head;                  //!<The current write position in the FIFO8 memory buffer
	uint16_t tail;                  //!<The current read position in the FIFO8 memory buffer
	uint16_t mask;                  //!<Bit mask used for FIFO8 buffer memory wrapping
	uint8_t idle;                   //!<Used to determine if the FIFO8 is already outputing bytes
	fifo8_mode_t mode;              //!<The FIFO8 mode of operation
	void (*out)(uint8_t byte);      //!<Pointer to the output function used by read function FIFO_get()
}FIFO8;

/*! 
    @brief Initializes the FIFO8 buffer.
    @details This must be called before any other function of the FIFO8 buffer will work.
    @param target Pointer to the FIFO8 object to initialize.
    @param mode The mode of the buffer. 
    @param buffer Pointer to the first byte of memory to be used for the buffer.
    @param size The size of the buffer (Must be a power or 2).
    @param output Pointer to the output function called when FIFO8_get() is used.
    @return E_FIFO8_NOERROR if no error, E_FIFO_8_NOMODE if the passed mode doesn't exist or E_FIFO8_NONBINARY
    if the memory buffer isn't a power of 2..
*/
fifo8_error_t FIFO8_init(FIFO8 *target, fifo8_mode_t mode, uint8_t *buffer, uint16_t size, void (*output)(uint8_t byte));
/*! 
    @brief Adds a character to the buffer.
    @details If the FIFO8 mode is #FIFO8_AUTO, and this is the first byte in the fifo, this will start the output
    stream as well.
    @param target Pointer to the FIFO8 object to add a byte to.
    @param byte The byte to add to the buffer.
    @return E_FIFO8_FULL if the buffer is full, E_FIFO8_NOERROR if it is not.
*/
fifo8_error_t FIFO8_put(FIFO8 *target, uint8_t byte);
/*! 
    @brief Takes a character from the buffer.
    @details The character taken from the buffer is passed to the output function set in FIFO8_init()
    @param target Pointer to the FIFO8 object to take a byte from.
    @return E_FIFO8_EMPTY if the buffer is empty, E_FIFO8_NOERROR if it is not.
*/
fifo8_error_t FIFO8_get(FIFO8 *target);
/*! 
    @brief Return the size in bytes of the FIFO8 buffer.
    @details 
    @param target Pointer to the FIFO8 object to get the size of.
    @return The size of the FIFO8 buffer in bytes.
*/
uint16_t FIFO8_size(FIFO8 *target);
/*! 
    @brief Flush all contents of the FIFO8 to the output function.
    @details This function recursibly calles FIFO8_get() until the memory buffer is empty.
    @param target Pointer to the FIFO8 object to get the size of.
    @return E_FIFO_EMPTY if called when the buffer is already empty, E_FIFO_NOERROR if not.
*/
fifo8_error_t FIFO8_flush(FIFO8 *target);

/*! 
    @brief Reset the fifo buffer and set its size to 0
*/

uint8_t FIFO8_pop(FIFO8 *target);

/**@}*/
/**@}*/
#endif
