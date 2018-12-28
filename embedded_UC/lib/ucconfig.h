#ifndef UCCONFIG_H
#define UCCONFIG_H_

/**
    @addtogroup COMMON
* @{
*/

/**
    @addtogroup UCCONFIG 
    @brief Header file for UCConfig module
    @details 

    This module depends on three other common libraries for its usage:
        - string11.h
        - flashWrite.h
        - fifo8.h

    This module is part of a larger project. Documentation is being developed.


    @author Stuart Ianna    
    @version 0.1
    @date December 2018
    @copyright GNU GPLv3
    @warning None
    @bug None
    @todo Documentation
    

    @par Compilers
    - arm-none-eabi-gcc (15:4.9.3+svn231177-1) 4.9.3 20150529 (prerelease)

    @par Example 
    A full example of UC_config using the STM32f1 internal flash memory as storage 
    @include example_ucConfig.c

    The example variables are populated from the python modules variables.yml 
    @include example_variables.yml

    The python module generates the header file which is included in the example program
    @include example_variables.h

 * @{
 */

/*! @file ucconfig.h
    @brief Header file for UCConfig module
*/

#include <string11.h>
#include <flashWrite.h>
#include <fifo8.h>
#include <stdio.h>

/*!
    @brief The length of the key which the PC needs to send to enter Config mode
*/
#define UCCONFIG_KEY_LENGTH 4
/*!
    @brief Key character 1
*/
#define UCCONFIG_KEY_1 2
/*!
    @brief Key character 2
*/
#define UCCONFIG_KEY_2 4
/*!
    @brief Key character 3
*/
#define UCCONFIG_KEY_3 6
/*!
    @brief Key character 4
*/
#define UCCONFIG_KEY_4  8
/*!
    @brief The size of the FIFO used by the module, don't change this
*/
#define UCCONFIG_FIFO_SIZE 32

/*!
    @brief The frame and character
*/
#define UCCONFIG_FRAME_END 22 
/*!
    @brief Command used to initiate setting the memory address pointer
*/
#define UCCONFIG_SET_MEMORY_ADDRESS 12
/*!
    @brief Command used to initial writing data 
*/
#define UCCONFIG_SET_WRITE_FRAME 13
/*!
    @brief Command used to initial reading data 
*/
#define UCCONFIG_READ_FRAME 14
/*!
    @brief Command used to exit from commmand mode
*/
#define UCCONFIG_TERMINATE 15
/*!
    @brief Command used get the current memory addresss pointer
*/
#define UCCONFIG_AT_ADDRESS 16
/*!
    @brief Command used to acknowledge a command
*/
#define UCCONFIG_ACK 17
/*!
    @brief Command used to not acknowledge a command
*/
#define UCCONFIG_NACK 18
/*!
    @brief ASCII character used for NULL
*/
#define UCCONFIG_NULL 19
/*!
    @brief ASCII character used for Not used
*/
#define UCCONFIG_NOT_USED 20
/*!
    @brief ASCII character used for new line, UC needs to send this so
    python module can easly pick end of command
*/
#define UCCONFIG_NEWLINE 10
/*!
    @brief ASCII character used for zero length data frames
*/
#define UCCONFIG_LENGTH_ZERO 21
/*!
    @brief ASCII character used for none types
*/
#define UCCONFIG_TYPE_NONE 11
/*!
    @brief ASCII character used for uint8_t types
*/
#define UCCONFIG_TYPE_UINT8_T 12
/*!
    @brief ASCII character used for int8_t types
*/
#define UCCONFIG_TYPE_INT8_T 13
/*!
    @brief ASCII character used for uint16_t types
*/
#define UCCONFIG_TYPE_UINT16_T 14
/*!
    @brief ASCII character used for int16_t types
*/
#define UCCONFIG_TYPE_INT16_T 15
/*!
    @brief ASCII character used for uint32_t types
*/
#define UCCONFIG_TYPE_UINT32_T 16
/*!
    @brief ASCII character used for int32_t types
*/
#define UCCONFIG_TYPE_INT32_T 17
/*!
    @brief ASCII character used for float types
*/
#define UCCONFIG_TYPE_FLOAT 18
/*!
    @brief ASCII character used for char types
*/
#define UCCONFIG_TYPE_CHAR 19
/*!
    @brief Number of loop iterratios before automattically exits from active mode
*/
#define UCCONFIG_ACTIVE_MODE_TIMEOUT 0xFFFF


/*!
    @brief UCCONFIG_GET() is the function macro used to get the value of any
    variable type in flash.
    @details This function works in conjuction with generated header file.
    See module example usage.
    @param X The pointer to the data containter for the variable, the value located in
    flash is stored in the containter.
    @param Y The address the data is located at.
    @warning There is no type checking of the variable fetched. Make sure that at the
    given memory location there is that type of variable.
*/
#define UCCONFIG_get(X,Y) _Generic((X),                                \
                                    char*:     ucconfig_get_c,      \
                                    uint8_t*:  ucconfig_get_u8,     \
                                    int8_t*:   ucconfig_get_8,      \
                                    uint16_t*: ucconfig_get_u16,    \
                                    int16_t*:  ucconfig_get_16,     \
                                    uint32_t*: ucconfig_get_u32,    \
                                    int32_t*:  ucconfig_get_32,     \
                                    float*:    ucconfig_get_float,  \
                                    default:   ucconfig_get_u8      \
                                                               )(X,Y)

/*!
    @brief Set up the module, this should be called before any other module
    function will work.
    @details See module example
    @param flash_read Function pointer to a function which reads one byte from a memory address. The address can be no longer than 16 bits wide
    @param flash_write Function pointer to a function which writes one byte to the memory address specified
    @param serial_write Funtions pointer to a function which writes one byte to a serial output stream such as USART or USB. This should be the same stream to which the PC uses to communicate with the UC
    @warning Flash read and write function usually take time to execture (~10ms). This module does not supply a delay for this execution. A delay should be probided in the main program. (see module example)

*/
void UCCONFIG_setup(
         uint8_t (*flash_read)(uint16_t address),
         void (*flash_write)(uint8_t byte,uint16_t adrress),
         void (*serial_write)(uint8_t byte));
/*!
    @brief Listens to serial communication and sets module in config mode if correct key is sent.
    @details This function should be placed in the output stream of the serial communication. See module example
    @param receivedByte A single byte recieved through serial communication.
 */
void UCCONFIG_listen(uint8_t receivedByte);
/*!
    @brief Runs config mode if the key has been received.
    @details This function should be placed in the programs main loop or on a timer callback (see example)
 */
void UCCONFIG_loop(void);
/*!
    @brief Set the memory address offset of the flash memory pointer (optional)
    @details This module initialises with memory pointer based at position 0. A offset can be supplied to charge the inital position of config parameters in flash.
    @param address The offset to be used.
 */
void UCCONFIG_setAddressOffset(uint16_t address);
/*!
    @brief Sets the function which is called when config mode is entered (optional)
    @details The function must be of type specified. Use a wrapper to call different function types (see example)
    @param on_enter The function to be called.
 */
void UCCONFIG_setOnEnter(void (*on_enter)(void));
/*!
    @brief Sets the function which is called when config mode is exited (optional)
    @details The function must be of type specified. Use a wrapper to call different function types (see example)
    @param on_exit The function to be called.
 */
void UCCONFIG_setOnExit(void (*on_exit)(void));
/*!
    @brief Sets the function which is called when the first byte is written to the UC from PC
    @details The function must be of type specified. Use a wrapper to call different function types (see example)
    @param on_first The function to be called.
 */
void UCCONFIG_setOnFirstWrite(void (*on_first)(void));
/*!
    @brief Get a character from the given memory address.
    @details This function is utilised in the marco expansion of UCCONFIG_get(). It
    should not be called manually
    @param data Pointer to a character to store the data in.
    @param address Address in flash to read the data from.
    @warning Don't call this function manually
*/
void ucconfig_get_c(char *data,uint16_t address);
/*!
    @brief Get a uint8 from the given memory address.
    @details This function is utilised in the marco expansion of UCCONFIG_get(). It
    should not be called manually
    @param data Pointer to a uint8 to store the data in.
    @param address Address in flash to read the data from.
    @warning Don't call this function manually
*/
void ucconfig_get_u8(uint8_t *data,uint16_t address);
/*!
    @brief Get a int8 from the given memory address.
    @details This function is utilised in the marco expansion of UCCONFIG_get(). It
    should not be called manually
    @param data Pointer to a int8 to store the data in.
    @param address Address in flash to read the data from.
    @warning Don't call this function manually
*/
void ucconfig_get_8(int8_t *data,uint16_t address);
/*!
    @brief Get a uint16 from the given memory address.
    @details This function is utilised in the marco expansion of UCCONFIG_get(). It
    should not be called manually
    @param data Pointer to a uint16 to store the data in.
    @param address Address in flash to read the data from.
    @warning Don't call this function manually
*/
void ucconfig_get_u16(uint16_t *data,uint16_t address);
/*!
    @brief Get a int16 from the given memory address.
    @details This function is utilised in the marco expansion of UCCONFIG_get(). It
    should not be called manually
    @param data Pointer to a int16 to store the data in.
    @param address Address in flash to read the data from.
    @warning Don't call this function manually
*/
void ucconfig_get_16(int16_t *data,uint16_t address);
/*!
    @brief Get a uint32 from the given memory address.
    @details This function is utilised in the marco expansion of UCCONFIG_get(). It
    should not be called manually
    @param data Pointer to a uint32 to store the data in.
    @param address Address in flash to read the data from.
    @warning Don't call this function manually
*/
void ucconfig_get_u32(uint32_t *data,uint16_t address);
/*!
    @brief Get a int32 from the given memory address.
    @details This function is utilised in the marco expansion of UCCONFIG_get(). It
    should not be called manually
    @param data Pointer to a int32 to store the data in.
    @param address Address in flash to read the data from.
    @warning Don't call this function manually
*/
void ucconfig_get_32(int32_t *data,uint16_t address);
/*!
    @brief Get a float from the given memory address.
    @details This function is utilised in the marco expansion of UCCONFIG_get(). It
    should not be called manually
    @param data Pointer to a float to store the data in.
    @param address Address in flash to read the data from.
    @warning Don't call this function manually
*/
void ucconfig_get_float(float *data,uint16_t address);

/**@}*/
/**@}*/
#endif
