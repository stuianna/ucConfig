// Header file specific for the target microcontroller
#include <stm32f103cb_core.h> 

//Assume ucconfig.h is located in same directory as source code
#include "uconfig.h" 

//Automatically generated ucConfig header file
#include "variables.h"

//Updated to reflect different high and low times
uint16_t highDelay;
uint16_t lowDelay;

//Functions for interracting with ucConfig
uint8_t flashRead(uint16_t address);
void flashWrite(uint8_t data, uint16_t address);
void serialWrite(uint8_t data);
void serialRead(uint8_t data);

//Optional functions for additional functionallity
void onEnter(void);
void onFirstWrite(void);
void onExit(void);

int main(void){

    //Setup UCConfig
    UCCONFIG_setup(&flashRead,&flashWrite,&serialWrite);
    UCCONFIG_setOnEnter(&onEnter);
    UCCONFIG_setOnFirstWrite(&onFirstWrite);
    UCCONFIG_setOnExit(&onExit);

    //Fetch high and low times, just call the onExit function
    onExit();

    //Setup the pin for digital output.
    pinSetup(GPIO_DO,PORTC,PIN13);


    while(1){

        //Regular calls to the loop function to catch usage
        UCCONFIG_loop();

        //Toggle the pin with new specified high and low delays
        pinHigh(PORTC,PIN13);
        delayms(highDelay);
        pinLow(PORTC,PIN13);
        delayms(lowDelay);
    }

}

void onEnter(void){

    //Do something when ucConfig transitions from background to run mode

}

void onFirstWrite(void){

    //Do something before the first write operation takes place.
    //i.e Some flash technologies need to be erased before use.
}

void onExit(void){

    //Called when transitioning from run to background mode
    //Use this so variables are updated at 'real time' as soon as the write operation completes
    UCCONFIG_get(&highDelay,HIGH_DELAY);
    UCCONFIG_get(&lowDelay,LOW_DELAY);
}

uint8_t flashRead(uint16_t address){

    //Your instruction for readiing a byte
    //Remember to add apropriate read cycle delays!
}

void flashWrite(uint8_t data, uint16_t address){

    //Your instruction for writing a byte
    //Remember to add apropriate write cycle delays!

}

void serialWrite(uint8_t data){

    //Your instruction for sending a byte
}

void serialRead(uint8_t data){

    //Called when a byte is available to read
    
    //Monitor recieved serial data
    UCCONFIG_listen(data);
}
