/**
    @addtogroup COMMON
* @{
*/

/**
    @addtogroup flashWrite
    @brief Functions for converting data types (float, uint16_t..) to byte streams for usage with EEPROM memory.
    @details This module is compatable with version C11 of C standard. It implements the _Generic keyword
    so common flash_put() and flash_get() functions can be used to read and write flash data from multiple data types.

    Two main functions exist for usage:
    - **flash_put()** -> Write given data to flash 
    - **flash_get()** -> Read given data from flash

    Current limitations
    - Maximum float size is limiting to int32 max divided by 10 ^ (MAX_DEC).
    - Float decimal places are limited by Macro #MAX_DEC.

    @author Stuart Ianna
    @version 0.1
    @date December 2018
    @copyright GNU GPLv3
    @warning Both the read and write function do not implement a delay between byte read/write operations. Many EEPROM require a set time to complete these operations (~10ms). A delay or should be used in the main program.
    @bug None
    @todo 
        
    @par Verified Compilers
    - arm-none-eabi-gcc (15:4.9.3+svn231177-1) 4.9.3 20150529 (prerelease)

    @par Example 

    @include example_flashWrite.c

 * @{
 */

/*! @file flashWrite.h
    @brief Header file for flashWrite library.
*/

#ifndef FLASH_WRITE_H
#define FLASH_WRITE_H

#include <stdio.h>

/*! 
    @brief Function pointer typedef for void function with uint8_t parameter
*/
typedef void(*v_fp_u8_u16)(uint8_t,uint16_t);
typedef uint8_t(*u8_fp_u16)(uint16_t);

/*! 
    @brief Flash a given datatype to the output stream.
*/
#define flash_put(DATA,ADDRESS) _Generic((DATA),               \
                            char:     FLASHWRITE_write_c,      \
                            uint8_t:  FLASHWRITE_write_u8,     \
                            int8_t:   FLASHWRITE_write_8,      \
                            uint16_t: FLASHWRITE_write_u16,    \
                            int16_t:  FLASHWRITE_write_16,     \
                            uint32_t: FLASHWRITE_write_u32,    \
                            int32_t:  FLASHWRITE_write_32,     \
                            float:    FLASHWRITE_write_float,  \
                            default:  FLASHWRITE_write_u8      \
                                                          )(DATA,ADDRESS)
/*! 
    @brief Read a given datatype from the input stream.
*/
#define flash_get(DATA,ADDRESS) _Generic((DATA),              \
                            char*:     FLASHWRITE_read_c,     \
                            uint8_t*:  FLASHWRITE_read_u8,    \
                            int8_t*:   FLASHWRITE_read_8,     \
                            uint16_t*: FLASHWRITE_read_u16,   \
                            int16_t*:  FLASHWRITE_read_16,    \
                            uint32_t*: FLASHWRITE_read_u32,   \
                            int32_t*:  FLASHWRITE_read_32,    \
                            float*:    FLASHWRITE_read_float, \
                            default:  FLASHWRITE_read_u8      \
                                                         )(DATA,ADDRESS)

/*! 
    @brief Maximum number of decimal places a float will contain.
*/
#define MAX_DEC 4


/*! 
    @brief Set the target output stream for flash_put functions
    @details This function must be called before flash_put() will work
    @warning Undefined behaviour will occur if flash_put() is used before the
    output function is set.
    @param out Pointer to the output stream function.
    @return none.
*/
void FLASHWRITE_setOutput(void (*out)(uint8_t,uint16_t));
/*! 
    @brief Set the target input stream for flash_get functions
    @details This function must be called before flash_get() will work.
    @warning Undefined behaviour will occur if flash_get() is used before
    the input function is set.
    @param in Pointer to the input stream function.
    @return none.
*/
void FLASHWRITE_setInput(uint8_t (*in)(uint16_t));

/*! 
    @brief Get the current target output stream
    @return Function pointer to output stream or Null if not defined.
*/
v_fp_u8_u16 FLASHWRITE_getOutput(void);

/*! 
    @brief Get the current target input stream
    @return Function pointer to input stream or Null if not defined.
*/
u8_fp_u16 FLASHWRITE_getInput(void);

/*! 
    @brief Write a unsigned 8 bit integer to address given
    @details This function is  invoked by macro definition flash_put()
    @param data The data to be written
    @param address The address to be written to
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_write_u8(uint8_t data, uint16_t address);

/*! 
    @brief Read a unsigned 8 bit integer from address given
    @details This function is invoked by macro definition flash_put
    @param data The container to store the read data
    @param address The address to read from 
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_read_u8(uint8_t *data, uint16_t address);
/*! 
    @brief Write a signed 8 bit integer to address given
    @details This function is invoked by macro definition flash_put
    @param data The data to be written
    @param address The address to be written to
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_write_8(int8_t data, uint16_t address);

/*! 
    @brief Read a signed 8 bit integer from address given
    @details This function is invoked by macro definition flash_put
    @param data The container to store the read data
    @param address The address to read from 
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_read_8(int8_t *data, uint16_t address);
/*! 
    @brief Write a char to address given
    @details This function is invoked by macro definition flash_put
    @param data The data to be written
    @param address The address to be written to
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_write_c(char data, uint16_t address);

/*! 
    @brief Read a char from address give
    @details This function is invoked by macro definition flash_put
    @param data The container to store the read data
    @param address The address to read from 
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_read_c(char *data, uint16_t address);
/*! 
    @brief Write a unsigned 16 bit integer to address given
    @details This function is invoked by macro definition flash_put
    @param data The data to be written
    @param address The address to be written to
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_write_u16(uint16_t data, uint16_t address);

/*! 
    @brief Read a unsigned 16 bit integer from address give
    @details This function is invoked by macro definition flash_put
    @param data The container to store the read data
    @param address The address to read from 
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_read_u16(uint16_t *data, uint16_t address);
/*! 
    @brief Write a signed 16 bit integer to address give
    @details This function is invoked by macro definition flash_put
    @param data The data to be written
    @param address The address to be written to
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_write_16(int16_t data, uint16_t address);

/*! 
    @brief Read a signed 16 bit integer from address give
    @details This function is invoked by macro definition flash_put
    @param data The container to store the read data
    @param address The address to read from 
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_read_16(int16_t *data, uint16_t address);
/*! 
    @brief Write a unsigned 32 bit integer to address given
    @details This function is invoked by macro definition flash_put
    @param data The data to be written
    @param address The address to be written to
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_write_u32(uint32_t data, uint16_t address);

/*! 
    @brief Read a unsigned 32 bit integer from address given
    @details This function is invoked by macro definition flash_put
    @param data The container to store the read data
    @param address The address to read from 
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_read_u32(uint32_t *data, uint16_t address);
/*! 
    @brief Write a signed 32 bit integer to address given
    @details This function is invoked by macro definition flash_put
    @param data The data to be written
    @param address The address to be written to
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_write_32(int32_t data, uint16_t address);

/*! 
    @brief Read a signed 32 bit integer from address given
    @details This function is invoked by macro definition flash_put
    @param data The container to store the read data
    @param address The address to read from 
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_read_32(int32_t *data, uint16_t address);
/*! 
    @brief Write a float to address given
    @details This function is invoked by macro definition flash_put
    @param data The data to be written
    @param address The address to be written to
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_write_float(float data, uint16_t address);

/*! 
    @brief Read a float from address given
    @details This function is invoked by macro definition flash_put
    @param data The container to store the read data
    @param address The address to read from 
    @return The memory address given plus one.
*/
uint16_t FLASHWRITE_read_float(float *data, uint16_t address);

/**@}*/
/**@}*/

#endif
