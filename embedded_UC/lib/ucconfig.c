#include "ucconfig.h"

//Flash read function pointer
static uint8_t (*ucconfig_fp_flashRead)(uint16_t address);

//Flash write function pointer
static void (*ucconfig_fp_flashWrite)(uint8_t byte,uint16_t address);

//Serial write function pointer
static void (*ucconfig_fp_serialWrite)(uint8_t byte);

//Config mode exit function pointer
static void (*ucconfig_fp_onExit)(void);

//Config mode enter function pointer
static void (*ucconfig_fp_onEnter)(void);

//Config mode first write function pointer
static void (*ucconfig_fp_onFirstWrite)(void);

//Keep track of number of varaibles written, used to determine when to call firstWrite FP
static uint16_t ucconfig_written;

//True if in active config mode
static uint16_t ucconfig_activeMode;

//Flash memory pointers
static uint16_t ucconfig_memPointer;
static uint16_t ucconfig_memPointerOffset;

//FIFO used for serial communication
static FIFO8 ucconfig_fifo;
static uint8_t ucconfig_fifo_buffer[UCCONFIG_FIFO_SIZE];

static void ucconfig_checkKey(uint8_t byte);

//Main config loop, gets triggered by ucconfig_checkKey when a valid key is found
//The UC is 'Trapped' in this while(1) loop until a terminate command is sent or timeout occurs
static void ucconfig_active(void);

//Determine if it is the first byte write operation
static void ucconfig_call_if_first(void);

//Determine which command (if any) was sent
static void ucconfig_parseCommand();

//A successful write data command was sent
static void ucconfig_write_data();
//A successful read data command was sent
static void ucconfig_read_data();
//A successful set address command was sent
static void ucconfig_set_address();
//A successful get address command was sent
static void ucconfig_get_address();
//Exits from config mode
static void ucconfig_terminate();
//Sent not acknowledge
static void ucconfig_sendNack();
//Sent acknowledge
static void ucconfig_sendAck();
//Sent not acknowledge
static void ucconfig_sendNack();

//Send given data types back to PC when requested by read_data
static void ucconfig_send_u8(void);
static void ucconfig_send_8(void);
static void ucconfig_send_u16(void);
static void ucconfig_send_16(void);
static void ucconfig_send_u32(void);
static void ucconfig_send_32(void);
static void ucconfig_send_float(void);
static void ucconfig_send_char(void);

//Write given data types to flash memory when requested by set_data
static void ucconfig_write_u8(char *data);
static void ucconfig_write_8(char *data);
static void ucconfig_write_u16(char *data);
static void ucconfig_write_16(char *data);
static void ucconfig_write_u32(char *data);
static void ucconfig_write_32(char *data);
static void ucconfig_write_float(char *data);
static void ucconfig_write_char(char *data);

void UCCONFIG_loop(void){

    while(ucconfig_activeMode){
        
        for(volatile uint16_t i = 0; i < 0xFFF;i++);
        ucconfig_activeMode--;
    }
}

/***********************************************************************/
    //Function relating to the macro expansion of UCCONFIG_get()
/***********************************************************************/

void UCCONFIG_setOnEnter(void (*on_enter)(void)){ 

    ucconfig_fp_onEnter = on_enter;
}
void UCCONFIG_setOnExit(void (*on_exit)(void)){ 

    ucconfig_fp_onExit = on_exit;
}
void UCCONFIG_setOnFirstWrite(void (*on_first)(void)){ 

    ucconfig_fp_onFirstWrite = on_first;
}

void ucconfig_get_c(char *data,uint16_t address){

    ucconfig_memPointer = flash_get(data,address);
    return;
}

void ucconfig_get_u8(uint8_t *data,uint16_t address){

    ucconfig_memPointer = flash_get(data,address);
    return;
}

void ucconfig_get_8(int8_t *data,uint16_t address){

    ucconfig_memPointer = flash_get(data,address);
    return;
}

void ucconfig_get_u16(uint16_t *data,uint16_t address){


    ucconfig_memPointer = flash_get(data,address);
    return;
}

void ucconfig_get_16(int16_t *data,uint16_t address){

    ucconfig_memPointer = flash_get(data,address);
    return;
}

void ucconfig_get_u32(uint32_t *data,uint16_t address){

    ucconfig_memPointer = flash_get(data,address);
    return;
}

void ucconfig_get_32(int32_t *data,uint16_t address){

    ucconfig_memPointer = flash_get(data,address);
    return;
}

void ucconfig_get_float(float *data,uint16_t address){

    ucconfig_memPointer = flash_get(data,address);
    return;
}

void UCCONFIG_setAddressOffset(uint16_t address){

    ucconfig_memPointerOffset = address;
}

//Main config loop, gets triggered by ucconfig_checkKey when a valid key is found
void ucconfig_active(void){

    //Store the current function pointer for STRING11 output
    void (*current_fp)(uint8_t) = STRING11_getOutput();

    //Store current flashwrite fp
    void (*current_fw_fp)(uint8_t,uint16_t) = FLASHWRITE_getOutput();
    uint8_t (*current_fr_fp)(uint16_t) = FLASHWRITE_getInput();

    //Set the output function for STRING_11 to serial fp, this allows use of all print functions
    STRING11_setOutput(ucconfig_fp_serialWrite);

    //Setup flash output
    FLASHWRITE_setOutput(ucconfig_fp_flashWrite);
    FLASHWRITE_setInput(ucconfig_fp_flashRead);

    //Store the old FP in config fp for later resetting
    ucconfig_fp_serialWrite = current_fp;
    ucconfig_fp_flashWrite = current_fw_fp;
    ucconfig_fp_flashRead = current_fr_fp;

    ucconfig_written = 0;

    ucconfig_sendAck();

    if(ucconfig_fp_onEnter != NULL){

        ucconfig_fp_onEnter();
    }

    ucconfig_activeMode = UCCONFIG_ACTIVE_MODE_TIMEOUT;
    return;
}

//Called when a successful write command was initiated.
//If frame is valid writes the given data to flash
void ucconfig_write_data(){

    uint8_t dataLength;
    uint8_t dataType;
    char data[24]; //maximum size in characters of data i 24 bytes
    uint8_t i;

    //Data type is checked later when branching to flash write functions
    dataType = FIFO8_pop(&ucconfig_fifo);

    //Length is the capital letter of the aplphabet. eg. A = 1, C = 3
    dataLength = FIFO8_pop(&ucconfig_fifo) - 64 ;

    //Check length, max length is 24
    if((dataLength < 1) | (dataLength > 24)){

        ucconfig_sendNack();
        return;
    }

    //Next two should be not used charaters
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NOT_USED){
        ucconfig_sendNack();
        return;
    }

    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NOT_USED){
        ucconfig_sendNack();
        return;
    }

    for(i = 0; i < dataLength; i++){

        data[i] = FIFO8_pop(&ucconfig_fifo);

        //Check the data is only numbers
        if((data[i] > 47) && (data[i] < 58)){
            
        }
        //If there is a period, make sure the type is float
        else if((data[i] == 46) && (dataType == UCCONFIG_TYPE_FLOAT)){

        }
        //Check for minus for 8 bit
        else if(data[i] == 45 && (dataType == UCCONFIG_TYPE_INT8_T)){

        }
        //Check for minus for 16 bit
        else if(data[i] == 45 && (dataType == UCCONFIG_TYPE_INT16_T)){

        }
        //Check for minus for 32 bit
        else if(data[i] == 45 && (dataType == UCCONFIG_TYPE_INT32_T)){

        }
        //Check for minus for float
        else if(data[i] == 45 && (dataType == UCCONFIG_TYPE_FLOAT)){

        }
        //Char can be anything
        else if(dataType == UCCONFIG_TYPE_CHAR){

        }
        else{
            //Not a valid data value
            ucconfig_sendNack();
            return;
        }
    }

    //Null should follow data
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NULL){
        ucconfig_sendNack();
        return;
    }

    //Make sure data is correct length
    if(i != dataLength){

        ucconfig_sendNack();
        return;
    }

    //Add a null character
    data[i] = 0;

    //Branch and write data if the type was valid
    switch(dataType){

        case UCCONFIG_TYPE_UINT8_T: 
            ucconfig_call_if_first();
            ucconfig_write_u8(data);
            break;
        case UCCONFIG_TYPE_INT8_T: 
            ucconfig_call_if_first();
            ucconfig_write_8(data);
            break;
        case UCCONFIG_TYPE_UINT16_T: 
            ucconfig_call_if_first();
            ucconfig_write_u16(data);
            break;
        case UCCONFIG_TYPE_INT16_T: 
            ucconfig_call_if_first();
            ucconfig_write_16(data);
            break;
        case UCCONFIG_TYPE_UINT32_T: 
            ucconfig_call_if_first();
            ucconfig_write_u32(data);
            break;
        case UCCONFIG_TYPE_INT32_T: 
            ucconfig_call_if_first();
            ucconfig_write_32(data);
            break;
        case UCCONFIG_TYPE_FLOAT: 
            ucconfig_call_if_first();
            ucconfig_write_float(data);
            break;
        case UCCONFIG_TYPE_CHAR: 
            ucconfig_call_if_first();
            ucconfig_write_char(data);
            break;
        default:
            ucconfig_sendNack();
            return;

    }

    ucconfig_written++;

    //All good, let PC know write is finished.
    ucconfig_sendAck();

}
static void ucconfig_call_if_first(void){

    if(ucconfig_written == 0){
        ucconfig_fp_onFirstWrite();
    }

    return;

}

/**************************************************************************/
    //Individual data type writes to flash, called by ucconfig_write_data()
/**************************************************************************/
static void ucconfig_write_u8(char *data){

    uint8_t toWrite;
    toWrite = str2uint(data);
    ucconfig_memPointer = flash_put(toWrite,ucconfig_memPointer);
}

static void ucconfig_write_8(char *data){

    int8_t toWrite;
    toWrite = str2int(data);
    ucconfig_memPointer = flash_put(toWrite,ucconfig_memPointer);
}

static void ucconfig_write_u16(char *data){

    uint16_t toWrite;
    toWrite = str2uint(data);
    ucconfig_memPointer = flash_put(toWrite,ucconfig_memPointer);
}

static void ucconfig_write_16(char *data){

    int16_t toWrite;
    toWrite = str2int(data);
    ucconfig_memPointer = flash_put(toWrite,ucconfig_memPointer);
}

static void ucconfig_write_u32(char *data){

    uint32_t toWrite;
    toWrite = str2uint(data);
    ucconfig_memPointer = flash_put(toWrite,ucconfig_memPointer);
}

static void ucconfig_write_32(char *data){

    int32_t toWrite;
    toWrite = str2int(data);
    ucconfig_memPointer = flash_put(toWrite,ucconfig_memPointer);
}

static void ucconfig_write_float(char *data){

    float toWrite;
    toWrite = str2float(data);
    ucconfig_memPointer = flash_put(toWrite,ucconfig_memPointer);
}

static void ucconfig_write_char(char *data){

    ucconfig_memPointer = flash_put((char)data[0],ucconfig_memPointer);
}

//Called when a successful read data command was sent
//If frame is valid, sends read data to PC
//Responds with Nack frame is invalid
void ucconfig_read_data(){

    uint8_t dataType = FIFO8_pop(&ucconfig_fifo);

    //Length should be zero for get data
    if((FIFO8_pop(&ucconfig_fifo) != UCCONFIG_LENGTH_ZERO)){

        ucconfig_sendNack();
        return;
    }

    //Next two should be not used charaters
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NOT_USED){
        ucconfig_sendNack();
        return;
    }

    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NOT_USED){
        ucconfig_sendNack();
        return;
    }

    //Last is Null character
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NULL){
        ucconfig_sendNack();
        return;
    }

    switch(dataType){

        case UCCONFIG_TYPE_UINT8_T:
            ucconfig_send_u8();
            break;
        case UCCONFIG_TYPE_INT8_T:
            ucconfig_send_8();
            break;
        case UCCONFIG_TYPE_UINT16_T:
            ucconfig_send_u16();
            break;
        case UCCONFIG_TYPE_INT16_T:
            ucconfig_send_16();
            break;
        case UCCONFIG_TYPE_UINT32_T:
            ucconfig_send_u32();
            break;
        case UCCONFIG_TYPE_INT32_T:
            ucconfig_send_32();
            break;
        case UCCONFIG_TYPE_FLOAT:
            ucconfig_send_float();
            break;
        case UCCONFIG_TYPE_CHAR:
            ucconfig_send_char();
            break;
        default:
            ucconfig_sendNack();
            break;
    }

}
/*****************************************************************************/
    //These fectch a given data type from flash and send it to PC
    //Called by ucconfig_read_data()
/*****************************************************************************/
void ucconfig_send_u8(void){

    //get data
    uint8_t data;
    ucconfig_memPointer = flash_get(&data,ucconfig_memPointer);

    print((char)UCCONFIG_READ_FRAME);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_TYPE_UINT8_T);
    print((char)UCCONFIG_LENGTH_ZERO);
    print((char)UCCONFIG_NOT_USED);
    print((char)UCCONFIG_NOT_USED);
    print(data);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
    return;
}

void ucconfig_send_8(void){

    //get data
    int8_t data;
    ucconfig_memPointer = flash_get(&data,ucconfig_memPointer);

    print((char)UCCONFIG_READ_FRAME);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_TYPE_INT8_T);
    print((char)UCCONFIG_LENGTH_ZERO);
    print((char)UCCONFIG_NOT_USED);
    print((char)UCCONFIG_NOT_USED);
    print((int8_t)data);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
    return;
}

void ucconfig_send_u16(void){

    //get data
    uint16_t data;
    ucconfig_memPointer = flash_get(&data,ucconfig_memPointer);

    print((char)UCCONFIG_READ_FRAME);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_TYPE_UINT16_T);
    print((char)UCCONFIG_LENGTH_ZERO);
    print((char)UCCONFIG_NOT_USED);
    print((char)UCCONFIG_NOT_USED);
    print(data);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
    return;
}

void ucconfig_send_16(void){

    //get data
    int16_t data;
    ucconfig_memPointer = flash_get(&data,ucconfig_memPointer);

    print((char)UCCONFIG_READ_FRAME);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_TYPE_INT16_T);
    print((char)UCCONFIG_LENGTH_ZERO);
    print((char)UCCONFIG_NOT_USED);
    print((char)UCCONFIG_NOT_USED);
    print(data);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
    return;
}

void ucconfig_send_u32(void){

    //get data
    uint32_t data;
    ucconfig_memPointer = flash_get(&data,ucconfig_memPointer);

    print((char)UCCONFIG_READ_FRAME);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_TYPE_UINT32_T);
    print((char)UCCONFIG_LENGTH_ZERO);
    print((char)UCCONFIG_NOT_USED);
    print((char)UCCONFIG_NOT_USED);
    print(data);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
    return;
}

void ucconfig_send_32(void){

    //get data
    int32_t data;
    ucconfig_memPointer = flash_get(&data,ucconfig_memPointer);

    print((char)UCCONFIG_READ_FRAME);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_TYPE_INT32_T);
    print((char)UCCONFIG_LENGTH_ZERO);
    print((char)UCCONFIG_NOT_USED);
    print((char)UCCONFIG_NOT_USED);
    print(data);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
    return;
}

void ucconfig_send_float(void){

    //get data
    float data;
    ucconfig_memPointer = flash_get(&data,ucconfig_memPointer);

    print((char)UCCONFIG_READ_FRAME);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_TYPE_FLOAT);
    print((char)UCCONFIG_LENGTH_ZERO);
    print((char)UCCONFIG_NOT_USED);
    print((char)UCCONFIG_NOT_USED);
    print(data);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
    return;
}

void ucconfig_send_char(void){

    //get data
    char data;
    ucconfig_memPointer = flash_get(&data,ucconfig_memPointer);

    print((char)UCCONFIG_READ_FRAME);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_TYPE_CHAR);
    print((char)UCCONFIG_LENGTH_ZERO);
    print((char)UCCONFIG_NOT_USED);
    print((char)UCCONFIG_NOT_USED);
    print(data);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
    return;
}

//Sets the current memeory address of the flash pointer + offset
//Responds with ack if ok or nack if error in received frame.
void ucconfig_set_address(){

    uint8_t addressLength;
    char address[6]; //maximum size in characters of 16 bit number is 5. +1 for NULL terminator
    uint8_t i;

    //Frame pos 3 is type, should be none for address
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_TYPE_NONE){
        ucconfig_sendNack();
        return;
    }

    //Length is the capital letter of the aplphabet. eg. A = 1, C = 3
    addressLength = FIFO8_pop(&ucconfig_fifo) - 64 ;

    //Check length, a 16 bit number won't be longer than 5 digits
    if((addressLength < 1) | (addressLength > 5)){

        ucconfig_sendNack();
        return;
    }

    //Next two should be not used charaters
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NOT_USED){
        ucconfig_sendNack();
        return;
    }

    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NOT_USED){
        ucconfig_sendNack();
        return;
    }


    //Address should only contain numbers
    for(i = 0; i < addressLength; i++){

        address[i] = FIFO8_pop(&ucconfig_fifo);

        if((address[i] < 48) | (address[i] > 57)){
            
            ucconfig_sendNack();
            return;
        }
    }

    //Make sure the lenght of the address maches the acual data
    if(i != addressLength){

        ucconfig_sendNack();
        return;

    }

    //Null character last
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NULL){
        ucconfig_sendNack();
        return;
    }

    address[i] = 0;

    //Set the memeory pointer address plus the offset.
    ucconfig_memPointer = (uint16_t)str2int(address) + ucconfig_memPointerOffset;

    //All good
    ucconfig_sendAck();
}

//Sends the current memeory address of the flash to the PC
//Responds with Nack if received request frame is invalid.
void ucconfig_get_address(){
    
    //Frame pos 3 is type, should be none for getAddress
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_TYPE_NONE){
        ucconfig_sendNack();
        return;
    }

    //Length should be zero for get address
    if((FIFO8_pop(&ucconfig_fifo) != UCCONFIG_LENGTH_ZERO)){

        ucconfig_sendNack();
        return;
    }

    //Next two should be not used charaters
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NOT_USED){
        ucconfig_sendNack();
        return;
    }

    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NOT_USED){
        ucconfig_sendNack();
        return;
    }

    //Last is Null character
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NULL){
        ucconfig_sendNack();
        return;
    }

    //Send that puppy
    print((char)UCCONFIG_AT_ADDRESS);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_TYPE_NONE);
    print((char)UCCONFIG_LENGTH_ZERO);
    print((char)UCCONFIG_NOT_USED);
    print((char)UCCONFIG_NOT_USED);
    print(ucconfig_memPointer);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
}

//Exit from config mode if the terminate frame was valid.
void ucconfig_terminate(){

    //Frame pos 3 is type, should be none for terminate
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_TYPE_NONE){
        ucconfig_sendNack();
        return;
    }

    //Length should be zero for terminate
    if((FIFO8_pop(&ucconfig_fifo) != UCCONFIG_LENGTH_ZERO)){

        ucconfig_sendNack();
        return;
    }

    //Next two should be not used charaters
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NOT_USED){
        ucconfig_sendNack();
        return;
    }

    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NOT_USED){
        ucconfig_sendNack();
        return;
    }

    //Last is Null character
    if(FIFO8_pop(&ucconfig_fifo) != UCCONFIG_NULL){
        ucconfig_sendNack();
        return;
    }

    //Everythin good send the acknowledge
    ucconfig_sendAck();

    //Store the current function pointer for STRING11 output
    void (*current_fp)(uint8_t) = STRING11_getOutput();

    //Store current flashwrite fp
    void (*current_fw_fp)(uint8_t,uint16_t) = FLASHWRITE_getOutput();
    uint8_t (*current_fr_fp)(uint16_t) = FLASHWRITE_getInput();

    //Set the output function for STRING_11 to serial fp, this allows use of all print functions
    STRING11_setOutput(ucconfig_fp_serialWrite);

    //Setup flash output
    FLASHWRITE_setOutput(ucconfig_fp_flashWrite);
    FLASHWRITE_setInput(ucconfig_fp_flashRead);

    //Store the old FP in config fp for later resetting
    ucconfig_fp_serialWrite = current_fp;
    ucconfig_fp_flashWrite = current_fw_fp;
    ucconfig_fp_flashRead = current_fr_fp;

    if(ucconfig_fp_onExit != NULL){

        ucconfig_fp_onExit();
    }

    //Set the flag so active mode loop terminates
    ucconfig_activeMode = 0;
}

//Send not acknowledge
void ucconfig_sendNack(){

    print((char)UCCONFIG_NACK);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
}

//Send acknowledge
void ucconfig_sendAck(){

    print((char)UCCONFIG_ACK);
    print((char)UCCONFIG_NULL);
    print((char)UCCONFIG_FRAME_END);
    print((char)UCCONFIG_NEWLINE);
}

//A frame end character was received
//Check the fifo buffer the see if a valid command exists
void ucconfig_parseCommand(){

    while(FIFO8_size(&ucconfig_fifo) > 0){

        switch(FIFO8_pop(&ucconfig_fifo)){

            case UCCONFIG_FRAME_END:

                //Nothing valid in the frame
                return;
                break;

            case UCCONFIG_SET_MEMORY_ADDRESS:

                if(FIFO8_pop(&ucconfig_fifo) == UCCONFIG_NULL){

                    ucconfig_set_address();

                    //Flush the rest of the FIFO
                    while(FIFO8_size(&ucconfig_fifo) > 0){
                        
                        FIFO8_pop(&ucconfig_fifo);
                    }
                    return;
                }
                break;

            case UCCONFIG_SET_WRITE_FRAME:

                if(FIFO8_pop(&ucconfig_fifo) == UCCONFIG_NULL){

                    ucconfig_write_data();

                    //Flush the rest of the FIFO
                    while(FIFO8_size(&ucconfig_fifo) > 0){
                        
                        FIFO8_pop(&ucconfig_fifo);
                    }
                    return;
                }
                break;

            case UCCONFIG_READ_FRAME:

                if(FIFO8_pop(&ucconfig_fifo) == UCCONFIG_NULL){

                    ucconfig_read_data();

                    //Flush the rest of the FIFO
                    while(FIFO8_size(&ucconfig_fifo) > 0){
                        
                        FIFO8_pop(&ucconfig_fifo);
                    }
                    return;
                }
                break;

            case UCCONFIG_AT_ADDRESS:

                if(FIFO8_pop(&ucconfig_fifo) == UCCONFIG_NULL){

                    ucconfig_get_address();

                    //Flush the rest of the FIFO
                    while(FIFO8_size(&ucconfig_fifo) > 0){
                        
                        FIFO8_pop(&ucconfig_fifo);
                    }
                    return;
                }
                break;

            case UCCONFIG_TERMINATE:

                if(FIFO8_pop(&ucconfig_fifo) == UCCONFIG_NULL){
                    ucconfig_terminate();
                    return;
                }
                break;

            default:
                break;
        }
    }
}

//Setup the module
void UCCONFIG_setup(uint8_t (*flash_read)(uint16_t address),void (*flash_write)(uint8_t byte, uint16_t address),void (*serial_write)(uint8_t byte)){

    //Assign Function pointers for flash operations
    ucconfig_fp_flashRead = flash_read;
    ucconfig_fp_flashWrite = flash_write;

    //Assign function pointer for serial write
    ucconfig_fp_serialWrite = serial_write;

    //Setup the received FIFO Object
    FIFO8_init(&ucconfig_fifo,FIFO8_TRIGGER,ucconfig_fifo_buffer,UCCONFIG_FIFO_SIZE,&ucconfig_checkKey);

    FLASHWRITE_setOutput(flash_write);
    FLASHWRITE_setInput(flash_read);

    //Initially inactive (running in background)
    ucconfig_activeMode = 0;
}

//This is called each time a byte is flushed from the FIFO
//Triggered by the UCCONFIG_listen when the FIFO has at least UCCONFIG_KEY_LENGTH bytes of key bytes
//If the key combination is found calls ucconfig_active()
void ucconfig_checkKey(uint8_t received){

    static uint8_t dump_pos = UCCONFIG_KEY_LENGTH;

    if(FIFO8_size(&ucconfig_fifo) > UCCONFIG_KEY_LENGTH){

        dump_pos = UCCONFIG_KEY_LENGTH;
        return;
    }
    
    switch(dump_pos){

        case 4:
            if(received == UCCONFIG_KEY_1){

                if(FIFO8_size(&ucconfig_fifo) > UCCONFIG_KEY_LENGTH -1){
                    dump_pos--;
                }
                else{

                    dump_pos = UCCONFIG_KEY_LENGTH;
                }
            }
            else{
                dump_pos = UCCONFIG_KEY_LENGTH;
            }
            break;
        case 3:
            if(received == UCCONFIG_KEY_2){

                if(FIFO8_size(&ucconfig_fifo) > UCCONFIG_KEY_LENGTH -2){
                    dump_pos--;
                }
                else{

                    dump_pos = UCCONFIG_KEY_LENGTH;
                }
            }
            else{
                dump_pos = UCCONFIG_KEY_LENGTH;
            }
            break;
        case 2:
            if(received == UCCONFIG_KEY_3){

                if(FIFO8_size(&ucconfig_fifo) > UCCONFIG_KEY_LENGTH -3){
                    dump_pos--;
                }
                else{

                    dump_pos = UCCONFIG_KEY_LENGTH;
                }
            }
            else{
                dump_pos = UCCONFIG_KEY_LENGTH;
            }
            break;
        case 1:
            if(received == UCCONFIG_KEY_4){

                //Sucsess
                ucconfig_activeMode = UCCONFIG_ACTIVE_MODE_TIMEOUT;
                dump_pos = UCCONFIG_KEY_LENGTH;
                ucconfig_active();
            }
            else{
                dump_pos = UCCONFIG_KEY_LENGTH;
            }
            break;

        default:
            dump_pos = UCCONFIG_KEY_LENGTH;
            break;
    }
}

void UCCONFIG_listen(uint8_t received){

    //If in active mode, check if a frame end character was received.
    if(ucconfig_activeMode){

        //Reload the timeout
        ucconfig_activeMode = UCCONFIG_ACTIVE_MODE_TIMEOUT;

        //Frame end receive, check if a valid command existed
        if(received == UCCONFIG_FRAME_END){

            FIFO8_put(&ucconfig_fifo,received);
            ucconfig_parseCommand();
        }
        else{

            //No frame end just fill the FIFO
            FIFO8_put(&ucconfig_fifo,received);
        }
        return;
    }

    //Check if the character received is at the end of the key. If it is make sure
    //There is enough characters in the fifo
    if((received == UCCONFIG_KEY_4) && (FIFO8_size(&ucconfig_fifo) >= (UCCONFIG_KEY_LENGTH-1))){

        FIFO8_put(&ucconfig_fifo,received);
        FIFO8_flush(&ucconfig_fifo);
    }
    //Not enough characters, only append to the FIFO characters which are part of the key
    else{

        switch(received){

            case UCCONFIG_KEY_1:
                FIFO8_put(&ucconfig_fifo,received);
                break;
            case UCCONFIG_KEY_2:
                FIFO8_put(&ucconfig_fifo,received);
                break;
            case UCCONFIG_KEY_3:
                FIFO8_put(&ucconfig_fifo,received);
                break;
            case UCCONFIG_KEY_4:
                FIFO8_put(&ucconfig_fifo,received);
                break;
            default:
                break;
        }
    }
    return;
}
