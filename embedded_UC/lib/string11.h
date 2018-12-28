/**
    @addtogroup COMMON
* @{
*/

/**
    @addtogroup String11
    @brief Character string library for C11 compilers 
    @details This module is compatable with version C11 of C standard. It impiments the _Generic keyword
    so a common print() function can be used to create a character stream from multiple data types.

    Five main functions exist for usage:
    - **print()** -> Print the given value.
    - **prints()** -> Print the given value plus a space.
    - **printt()** -> Print the given value plus a tab.
    - **printc()** -> Print the given value plus a comma.
    - **printl()** -> Print the given value plus a new line.

    Current limitations
    - Maximum float size is limiting to uint32 max / 10^MAX_DEC.
    - Float decimal places are limited by Macro #MAX_DEC
    - Optimizations could be done on some functions.

    @author Stuart Ianna
    @version 0.2
    @date December 2018
    @copyright GNU GPLv3
    @warning None
    @bug None
    @todo 
        - Optimisations on functions
        
    @par Verified Compilers
    - arm-none-eabi-gcc (15:4.9.3+svn231177-1) 4.9.3 20150529 (prerelease)

    @par Example 

    @include example_string11.c

 * @{
 */

/*! @file string11.h
    @brief Header file for C11 string library.
*/

#ifndef STRING11_H
#define STRING11_H

#include <stdio.h>

/*! 
    @brief Function pointer typedef for void function with uint8_t parameter
*/
typedef void(*v_fp_u8)(uint8_t);

/*! 
    @brief Print a given datatype to the output stream.
*/
#define print(X) _Generic((X),              \
                    char:       print_c,    \
                    uint8_t:    print_u8,   \
                    int8_t:     print_8,    \
                    uint16_t:   print_u16,  \
                    int16_t:    print_16,   \
                    uint32_t:   print_u32,  \
                    int32_t:    print_32,   \
                    float:      print_f,    \
                    int:        print_32,   \
                    char*:      print_s,    \
                    default:    print_c     \
                                            )(X)

/*! 
    @brief Print a given datatype + 'space' to the output stream.
*/
#define prints(X) _Generic((X),             \
                    char:       prints_c,   \
                    uint8_t:    prints_u8,  \
                    int8_t:     prints_8,   \
                    uint16_t:   prints_u16, \
                    int16_t:    prints_16,  \
                    uint32_t:   prints_u32, \
                    int32_t:    prints_32,  \
                    float:      prints_f,   \
                    int:        prints_32,  \
                    char*:      prints_s,   \
                    default:    prints_c    \
                                            )(X)
/*! 
    @brief Print a given datatype + 'comma' to the output stream.
*/
#define printc(X) _Generic((X),             \
                    char:       printc_c,   \
                    uint8_t:    printc_u8,  \
                    int8_t:     printc_8,   \
                    uint16_t:   printc_u16, \
                    int16_t:    printc_16,  \
                    uint32_t:   printc_u32, \
                    int32_t:    printc_32,  \
                    float:      printc_f,   \
                    int:        printc_32,  \
                    char*:      printc_s,   \
                    default:    printc_c    \
                                            )(X)

/*! 
    @brief Print a given datatype + 'newline' to the output stream.
*/
#define printl(X) _Generic((X),             \
                    char:       printl_c,   \
                    uint8_t:    printl_u8,  \
                    int8_t:     printl_8,   \
                    uint16_t:   printl_u16, \
                    int16_t:    printl_16,  \
                    uint32_t:   printl_u32, \
                    int32_t:    printl_32,  \
                    float:      printl_f,   \
                    int:        printl_32,  \
                    char*:      printl_s,   \
                    default:    printl_c    \
                                            )(X)

/*! 
    @brief Print a given datatype + 'tab' to the output stream.
*/
#define printt(X) _Generic((X),             \
                    char:       printt_c,   \
                    uint8_t:    printt_u8,  \
                    int8_t:     printt_8,   \
                    uint16_t:   printt_u16, \
                    int16_t:    printt_16,  \
                    uint32_t:   printt_u32, \
                    int32_t:    printt_32,  \
                    float:      printt_f,   \
                    int:        printt_32,  \
                    char*:      printt_s,   \
                    default:    printt_c    \
                                            )(X)
/*! 
    @brief Maximum number of decimal places a float will contain.
*/
#define MAX_DEC 4

/*! 
    @brief Convert a null terminated string to an integer.
    @param buffer Character array containing the number
    @return The parsed integer, returns 0 if contained invalid characters.
*/
int32_t str2int(char *buffer);

/*! 
    @brief Convert a null terminated string to an unsigned integer.
    @param buffer Character array containing the number
    @return The parsed integer, returns 0 if contained invalid characters.
*/
uint32_t str2uint(char *buffer);
/*! 
    @brief Convert a null terminated string to a float.
    @param buffer Character array containing the number
    @return The parsed integer, returns 0 if contained invalid characters.
*/
float str2float(char *buffer);

/*! 
    @brief Set the target output stream for print functions
    @details This function must be called before any print function will work.
    @param out Pointer to the output stream function.
    @return none.
*/
void STRING11_setOutput(void (*out)(uint8_t));

/*! 
    @brief Get the current target output stream
    @return Function pointer to output stream or Null if not defined.
*/
v_fp_u8 STRING11_getOutput(void);

/*! 
    @brief Send a character to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void print_c(char x);
/*! 
    @brief Send a unsigned byte to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void print_u8(uint8_t x);
/*! 
    @brief Send a signed byte to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void print_8(int8_t x);
/*! 
    @brief Send a unsigned short to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void print_u16(uint16_t x);
/*! 
    @brief Send a signed short to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void print_16(int16_t x);
/*! 
    @brief Send a unsigned integer to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void print_u32(uint32_t x);
/*! 
    @brief Send a signed integer to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void print_32(int32_t x);
/*! 
    @brief Send a float to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void print_f(float x);
/*! 
    @brief Send a string literal to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void print_s(char *x);
/*! 
    @brief Send a character plus a space to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void prints_c(char x);
/*! 
    @brief Send a unsigned byte plus a space to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void prints_u8(uint8_t x);
/*! 
    @brief Send a signed byte plus a space to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void prints_8(int8_t x);
/*! 
    @brief Send a unsigned short plus a space to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void prints_u16(uint16_t x);
/*! 
    @brief Send a signed short plus a space to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void prints_16(int16_t x);
/*! 
    @brief Send a unsigned integer plus a space to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void prints_u32(uint32_t x);
/*! 
    @brief Send a signed integer plus a space to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void prints_32(int32_t x);
/*! 
    @brief Send a float plus a space to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void prints_f(float x);
/*! 
    @brief Send a string literal plus a space to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void prints_s(char *x);
/*! 
    @brief Send a character plus a tab to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printt_c(char x);
/*! 
    @brief Send a unsigned byte plus a tab to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printt_u8(uint8_t x);
/*! 
    @brief Send a signed byte plus a tab to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printt_8(int8_t x);
/*! 
    @brief Send a unsigned short plus a tab to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printt_u16(uint16_t x);
/*! 
    @brief Send a signed short plus a tab to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printt_16(int16_t x);
/*! 
    @brief Send a unsigned integer plus a tab to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printt_u32(uint32_t x);
/*! 
    @brief Send a signed integer plus a tab to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printt_32(int32_t x);
/*! 
    @brief Send a float plus a tab to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printt_f(float x);
/*! 
    @brief Send a string literal plus a tab to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printt_s(char *x);
/*! 
    @brief Send a character plus a comma to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printc_c(char x);
/*! 
    @brief Send a unsigned byte plus a comma to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printc_u8(uint8_t x);
/*! 
    @brief Send a signed byte plus a comma to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printc_8(int8_t x);
/*! 
    @brief Send a unsigned short plus a comma to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printc_u16(uint16_t x);
/*! 
    @brief Send a signed short plus a comma to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printc_16(int16_t x);
/*! 
    @brief Send a unsigned integer plus a comma to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printc_u32(uint32_t x);
/*! 
    @brief Send a signed integer plus a comma to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printc_32(int32_t x);
/*! 
    @brief Send a float plus a comma to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printc_f(float x);
/*! 
    @brief Send a string literal plus a comma to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printc_s(char *x);
/*! 
    @brief Send a character plus a new line to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printl_c(char x);
/*! 
    @brief Send a unsigned byte plus a new line to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printl_u8(uint8_t x);
/*! 
    @brief Send a signed byte plus a new line to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printl_8(int8_t x);
/*! 
    @brief Send a unsigned short plus a new line to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printl_u16(uint16_t x);
/*! 
    @brief Send a signed short plus a new line to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printl_16(int16_t x);
/*! 
    @brief Send a unsigned integer plus a new line to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printl_u32(uint32_t x);
/*! 
    @brief Send a signed integer plus a new line to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printl_32(int32_t x);
/*! 
    @brief Send a float plus a new line to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printl_f(float x);
/*! 
    @brief Send a string literal plus a new line to the output stream. Implemented internally.
    @details This function is called by macro defined _Generic.
    @param x The data to be printed.
    @return none.
*/
void printl_s(char *x);

/**@}*/
/**@}*/

#endif
