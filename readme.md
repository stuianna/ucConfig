Table of Contents
=================

   * [Table of Contents](#table-of-contents)
   * [Introduction](#introduction)
      * [Platforms and Testing](#platforms-and-testing)
      * [How It Works](#how-it-works)
   * [Installation](#installation)
   * [Usage](#usage)
      * [Embedded Application](#embedded-application)
      * [PC Interface](#pc-interface)
         * [Generating the Variables File](#generating-the-variables-file)
         * [Generating the Header File](#generating-the-header-file)
         * [End Usage](#end-usage)
         * [Changing Variable Values](#changing-variable-values)
         * [Adding Additional Variables](#adding-additional-variables)
         * [Additional Options](#additional-options)
         * [Testing](#testing)
   * [Future Work (TODO)](#future-work-todo)
   * [Development](#development)

# Introduction

UcConfig is a combined command line interface and embedded firmware library to support the management of configuration variables stored in an embedded device's flash memory or external EEPROM device.

The combination of these two programs provides an interface in which the value of a variable stored in an embedded devices flash memory can be updated at runtime, without compilation, by changing a single line in a configuration file and running a terminal command.

The current development of ucConfig consists of a alpha version of the python API, command line interface and embedded firmware library. 'One-file' releases of the python module are built for windows and unix-based distributions.

## Platforms and Testing

UcConfig's PC interface has binaries released for both Windows and Linux 64 bit. Testing has been conducted on:

- Windows 7
- Windows 10
- Ubuntu 16.04
- Ubuntu 18.04

The embedded libraries have been tested with the following devices:

- STM32F103 with GCC version 5.4.0

The C API needs to be compiled with a compiler that supports the **C11 standard**. For GCC this can be done by passing the flag -std=c11. Potentially, this requirement will be removed in the future.

## How It Works

UcConfig is divided into two separate applications; The python module reads a list of variables defined in a .yml file and generates a C header file containing their designated embedded memory location. The C header file is then included in the embedded application's code on compilation.

The embedded module is included in the application firmware and provided with a set of function pointers for interacting with the target hardware memory and USART / Bluetooth / USB communication. The module 'listens' to external communication and becomes active when a certain key combination is received from the PC python application.

Once the module is active, in '***run***' mode, it takes runtime control of the hardware device and responds to all commands from the python program. The python program sends all variables as defined in the variable .yml file to the hardware device and verifies the received data was correct. Once all variables have been sent, there values are updated on the embedded device and the module return to ***background*** operation.

Assuming the type and amount of variables does not change, the python program can be used indefinitely to change the values of the embedded flash variables at runtime without the need for recompilation. This allows an 'end user' without specific knowledge of the developed firmware to change a set of parameters.

# Installation

The current release of the command line interface and embedded C module can be found under releases. The zip file can be downloaded and extracted into a local directory. 

The PC interface 'ucConfig' for Linux and 'ucConfig.exe' for Windows can be run from terminal or Powershell. To test it is functioning correctly, the command ```./ucConfig -v```  can be run. The output should be:

```
ucConfig CLI V0.1.0-alpha
```

The embedded sorce files located in the directory 'src' should be copied into your embedded applications directory, like in the usage example, or they can be added to the linkers search and compilers include path.


# Usage

Usage of ucConfig is demonstrated using an example application. The following assumptions are made:

1. You have a basic knowledge of how to use Linux terminal or Windows command prompt / Power shell.
2. You have reasonable knowledge of the Embedded C.
3. You have a emebedded device, microcontroller, connected to a PC with a running serial interface.
4. You are using and have working either the internall flash storage of the microcontroller or are using an external EEPROM, such as the AT24C256.
5. ucConfig, or a link (shortcut) to ucConfig is located in a directory forming part of your path environmental variable. Possibly ~/bin or /usr/bin on Linux or C:\Windows\System32 on Windows.

The example embedded application has the following directory structure:

src  
├── fifo8.c  
├── fifo8.h  
├── flashWrite.c  
├── flashWrite.h  
├── main.c  
├── string11.c  
├── string11.h  
├── ucconfig.c  
└── ucconfig.h  

Where main.c is the application being developed and all other files come from 'src' directory in releases.


## Embedded Application 

The complete source code for this example is located in embedded_UC/examples/main.c

Consider a simple blink program which toggles the output of a single pin with a 500 millisecond delay in between states.

```c
// File: src/main.c

// Header file specific for the target microcontroller
#include <stm32f103cb_core.h> 

uint16_t delayTime;

int main(void){

    delayTime = 500;

    //Setup the pin for digital output.
    pinSetup(GPIO_DO,PORTC,PIN13);

    while(1){

        //Toggle the pin output status and wait specified time.
        pinToggle(PORTC,PIN13);
        delayms(delayTime);
    }

}
```

Lets say we want to be able to change the variable *delayTime* permanently, without having the recompile the program, and at runtime.

There are four functions which ucConfig needs to operate, note the names of these functions do not needs to match exactly, only their functionality and signature.

1. ``` uint8_t flashRead(uint16_t address) ``` - Reads a single byte from flash memory at a given address.
1. ``` void flashWrite(uint8_t data, uint16_t address) ``` - Writes a single byte to flash memory at a given memory address.
1. ``` void serialWrite(uint8_t data) ``` - Writes a single byte to serial output stream
1. ``` void serialRead(uint8_t data) ``` - This function should either be called or be the interrupt service routine for USART or other serial recieve byte.

The flash read and write operations assume a 8-bit addressable memory structure, such as the AT24C256, however the library has been successfully tested with the 32-bit addressable internal memory of the STM32F1.

The previous program with the additional function is shown as:

```c
// File: src/main.c

// Header file specific for the target microcontroller
#include <stm32f103cb_core.h> 

uint16_t delayTime;

//Functions for interracting with ucConfig
uint8_t flashRead(uint16_t address);
void flashWrite(uint8_t data, uint16_t address);
void serialWrite(uint8_t data);
void serialRead(uint8_t data);


int main(void){

    delayTime = 500;

    //Setup the pin for digital output.
    pinSetup(GPIO_DO,PORTC,PIN13);

    while(1){

        //Toggle the pin output status and wait specified time.
        pinToggle(PORTC,PIN13);
        delayms(delayTime);
    }

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
}

```

Now that the required functions have been defined the ucConfig header and four required functions can be used:

1. ```void UCCONFIG_setup(uint8_t (*flashRead)(uint16_t),void (*flashWrite)(uint8_t,uint16_t),void (*serialWrite)(uint8_t))``` - Setup UCConfig and assign function pointers.
2. ``` void UCCONFIG_listen(uint8_t data)``` - Monitors received serial data and stores it in module local circular buffer.
3. ```void UCCONFIG_listen(void) ``` - Checks if the correct key has been received via serial communication. Locks into run mode is key received.
4. ```void UCCONFIG_get(_generic container,address)``` - Retreive the variable at the address location and store it in the container. Note the usage of the C11 generic keyword to ensure that ***all*** variable types can be retrieved using the same function call.

Note: The addition of the macro DELAY is covered shortly.

```c
// File: src/main.c

// Header file specific for the target microcontroller
#include <stm32f103cb_core.h> 

//Assume ucconfig.h is located in same directory as source code
#include "uconfig.h" 

uint16_t delayTime;

//Functions for interracting with ucConfig
uint8_t flashRead(uint16_t address);
void flashWrite(uint8_t data, uint16_t address);
void serialWrite(uint8_t data);
void serialRead(uint8_t data);

int main(void){

    //Setup UCConfig
    UCCONFIG_setup(&flashRead,&flashWrite,&serialWrite);

    //Fetch the value of the delay variable
    UCCONFIG_get(&delayTime,DELAY);

    //Setup the pin for digital output.
    pinSetup(GPIO_DO,PORTC,PIN13);


    while(1){

        //Regular calls to the loop function to catch usage
        UCCONFIG_loop();

        //Toggle the pin output status and wait specified time.
        pinToggle(PORTC,PIN13);
        delayms(delayTime);
    }

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

```

There are three additional optional function pointers which can be passed to ucConfig to provide adittional functionallity.

1. ```UCCONFIG_setOnEnter(void *(onEnter)(void))``` - A function which is called when ucConfig is transitioning from background mode to run mode.
2. ```UCCONFIG_setOnFirstWrite(void *(onEnter)(void))``` - A function which is called before the first write operation to flash memory is performed.
3. ```UCCONFIG_setOnExit(void *(onExit)(void))``` - A function which is called when ucConfig is transitioning from run mode to background mode.

```UCCONFIG_setOnExit()``` is usefull for reassigning new data values to variables after new values has been sent.

```c
// File: src/main.c

// Header file specific for the target microcontroller
#include <stm32f103cb_core.h> 

//Assume ucconfig.h is located in same directory as source code
#include "uconfig.h" 

uint16_t delayTime;

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


    //Fetch the value of the delay variable
    UCCONFIG_get(&delayTime,DELAY);

    //Setup the pin for digital output.
    pinSetup(GPIO_DO,PORTC,PIN13);


    while(1){

        //Regular calls to the loop function to catch usage
        UCCONFIG_loop();

        //Toggle the pin output status and wait specified time.
        pinToggle(PORTC,PIN13);
        delayms(delayTime);
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
    UCCONFIG_get(&delayTime,DELAY);
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
```

The current memory address in flash is increased and decreased based on the size of the data written and read from it. By default the inital address position is 0. An offset can be provided by calling ```UCCONFIG_setAddressOffset(uint16_t offset)``` after the module is setup with ```UCCONFIG_setup()```.

The additional macro ***DELAY*** is defined in a header file, automatically generated when the PC command line tool is used. For now it can be included to the top of the program like so:

```c
// File: src/main.c

// Header file specific for the target microcontroller
#include <stm32f103cb_core.h> 

//Assume ucconfig.h is located in same directory as source code
#include "uconfig.h" 

//Automatically generated ucConfig header file
#include "variables.h"
```

The embedded side of ucConfig is complete, although compilation will fail until the required header file is generated using the PC interface.

## PC Interface

The PC interface is a command line tool which is used to generate C header files, variable files and perform all communication withe embedded module.

The instructions provided are specific to unix based distribution, however the functionality has been tested on windows 7 and windows 10.

All commands are entered directly into linux terminal or windown command promt.

1. Check the installation is working correctly by typing ```ucConfig -v```. Output should be
```
ucConfig CLI V0.1.0-alpha
```
### Generating the Variables File

Navigate to the src directory used in the example above and generate a variable file with ```ucConfig -ge```. The resulting directory should now look like:

src  
├── fifo8.c  
├── fifo8.h  
├── flashWrite.c  
├── flashWrite.h  
├── main.c  
├── string11.c  
├── string11.h  
├── ucconfig.c  
├── ucconfig.h  
└── variables.yml  

Variables.yml is a file in [PyYAML](https://pyyaml.org/wiki/PyYAMLDocumentation) format. It is used to define all variables which are to be located in flash memory. The values in this file are used to generate the C header file and define which values are sent to the embedded device.

Open variables.yml with your favorite text editor (vim) to inspect and modify its contents.

variables.yml
```
# Example UC config file demonstrating correct variable layout.
# There are six required parameters for each variable
#   name - The name of the variable, must conform to C naming convention.
#   desc - A description of the variable, used to populate C header file comments and GUI.
#   value - The variable value flashed to the microcontroller.
#   dataType - A valid C type for the variable, options are:
#       uint8_t - Unsigned 8-bit integer.
#       int8_t - Signed 8-bit integer.
#       uint16_t - Unsigned 16-bit integer.
#       int16_t - Signed 16-bit integer.
#       uint32_t - Unsigned 32-bit integer.
#       int32_t - Signed 32-bit integer.
#       float -  Floating point, up to two decimal point percision.
#       char - An ASCII character - valid from ASCII 32 to ASCII 127.
#   max - The maximum allowed value, should be less the variable type's maximum.
#   min - The minimum allowed value, should be less the variable type's minimum.

# A single variable is demonstrated as:
- dataType: uint16_t
desc: Millisecond delay between LED toggles
max: 2000
min: 1
name: DELAY
value: 500
```
Notice the single variable DELAY matches the name given in the embedded code above. This has to be the case so the generated header file variable name matches.

Also, the type specified must be from the list in variables.yml and match the type of variable used in the embedded application. For example, in this case, the embedded aplication variable is of type uint16_t and variables.yml specifies the same type.

The only value stored in the embedded flash memory is the actual variable value. This means that our single variable for this example will only take up two bytes of storage.

Additional variables can be added by duplicating the single variable example as many times as necessary.

### Generating the Header File

Now that the variables file is complete the header file can be generated. From the same directory as the variables.yml file is locatin (src) run the following command: ``` ucConfig -q variables.yml -o variables.h```

The options specified perfom the following:

- -o Specifies the name of the output *.h C header file.
- -q Query the variable file. This has two functions, firstly the parsed variable are printed to stdout, allowing them to be checked. Secondly, it prevents the variables being sent to the embedded device, which at this point has not had its program compiled or deployed.

The working directory should now contain the following:

src  
├── fifo8.c  
├── fifo8.h  
├── flashWrite.c  
├── flashWrite.h  
├── variables.h  
├── main.c  
├── string11.c  
├── string11.h  
├── ucconfig.c  
├── ucconfig.h  
└── variables.yml  

The generated C header file should contain:

```c
#ifndef UCCONFIG_GEN_H
#define UCCONFIG_GEN_H
/*!
@file variables.h
@brief UcConfig V0.1.0-alpha automatically generated C header file.
@details Generated: Thu Jan  3 19:39:26 2019
Include this file in embedded programs code
*/
/*!
@brief Millisecond delay between LED toggles
@details The variable has the following parameters:
- Minimum Value: 1
- Maximum Value: 2000
- Flashed Value: 500
- Variable Type: uint16_t
The hexidecimal number is the variables location in non-volatile memory.
*/
#define DELAY  0x0

#endif
```

The header file is generated with [doxygen](http://www.doxygen.nl/) style comments detailing the same parameters as defined in the \*.yml file. Don't be tempted to change the Macro defined address location, this is used by the PC interface when sending variable values to the embedded device as well.

The embedded program now has the header file defined and included and can be compiled and deployed using whichever method necessary for your device.

### End Usage

Now that the embedded program is running, ucConfig can be used to permanently change / update the value of DELAY at runtime.

To perform the initial transfer or any subsequent transfers run
```
ucConfig -i variables.yml
```
This command transfers all variable values located in variables.yml to the embedded device, the output of this command should look something like:
```
Flashed:
-----------------
DELAY: 500
-----------------
Successfully verified 2 bytes.
```
The program transfers the variable values and confirms the transmission by reading them from the embedded device.

For the paranoid, the values can be read back by using:
```
ucConfig -r variables.yml
```
The variable \*.yml needs to be passed to ucConfig so it knows what memory structure to expect when reading the variables. The output of this command for the given example should be:
```
Variable                        Value               Read
DELAY                           500                 500
```
where 'Value' is the actual value expected from the \*.yml file and 'Read' is the value read from the device.

***Important*** Floating point values sent and received might not exactly match due to both floating point error and the current method of transmission.

### Changing Variable Values

To change the value of the **DELAY** variable all that needs to be done is open variables.yml, change the value like so:
```
- dataType: uint16_t
desc: Millisecond delay between LED toggles
max: 2000
min: 1
name: DELAY
value: 200 #Changed to 200 ms delay
```
Now run ```ucConfig -i variables.yml``` To update the value on the embedded devies flash memory and reload its runtime value.

Done.

### Adding Additional Variables

Let's say we wanted to change the embedded program to have different high and low delay times. The modified embedded program might look like this:

```c
// File: src/main.c

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
```

Now the additional variables are being used ***the header file must be regenerated*** and program recompiled and deployed, this makes sense, as those macros didn't exist before hand. It is good to note that any time additional variable are created or their types changed, this process will need to be done.

The variables.yml file needs to updated as well to reflect the changes. It might look like something like this:

```
- dataType: uint16_t
desc: Millisecond low time delay.
max: 2000
min: 1
name: LOW_DELAY
value: 800 

- dataType: uint16_t
desc: Millisecond high time delay.
max: 2000
min: 1
name: HIGH_DELAY
value: 200 
```

The new header file can be generated with ```ucConfig -q variables.yml -o variables.h```. The embedded program then can be recompiled and deployed.

The device can now be sent the new or any continually updated values using ```ucConfig -i variables.yml``` as many times as necessary.

### Additional Options

The PC interface ships with the default serial options of port /dev/ttyACM0 and a baud rate of 115200. These options can be changed using the command line interface, with the options specified being saved as the new default value. For example:

- ```ucConfig -b 9600``` - Set the baud to 9600
- ```ucConfig -p COM3``` - Set the port to COM3

To specify your own custom configuration file, the template can first be generated with:

- ```ucConfig -gc 'filename.yml'```

The file can then be modified and reloaded for use with

- ```ucConfig -c 'filename.yml'```

Python logging is used to track warnings, info and errors in the program. The logs are printed to stdout and their level can be changed wih:

- ```ucConfig -l 'logLevel'```

Where logLevel is either info, warning or none.

### Testing

There are two tests currently configured for ucConfig:

- ```ucConfig -t UC_coms_simple```

Generates a number of random variables of random types and sends them to the embedded device. Values are verified and accuracy is reported. The number of tests and variables to send can be changed by generating a custom configuration file and changing the respective parameters.

- ```ucConfig -t full_test```

The full test tests mosts aspects of the program:

1. A variable file is generated of random length, with random variables of random types.
2. The variable file is read by the PC parser.
3. The read variables are sent to the embedded device.
4. The original variable file is reloaded and used to read value back from the device.
5. The accuracy of the values are checked.
6. Repeat

The number of tests and variables to send can be changed by generating a custom configuration file and changing the respective parameters.

Currently tests are being develop to test the serial communication with added 'Noise'.

# Future Work (TODO)

The current command line interface provides friendly access to the underlying python API. There are two major features which are to be added in future releases.

1. The four separate embedded modules are to be combined into one. They are currently separate as they are pulled directly from [globLib](https://github.com/stuianna/globLib). Their combination will save space on both 'text' and 'bss' memory sections.

2. API documentation for the python interface, Doxygen documentation for the C embedded API.

3. A graphical user interface for easier 'point and click' access to the embedded flash memory. The user interface is to utilise the variable type, description and specified minimum and maximum values of the .yml file to limit the values to which a variable can be set. This ideally will provide an interface in which a user with 'limited' programming knowledge can change embedded parameters.

4. Possibly, all variable information could be stored in the first portion of the non-volatile memory. This would be space hungry, using around 64 bytes per variable, although external EEPROM device can have around 128kb of storage, so it would still enable around 2000 variables to be used. Having this option would hide the variables file from a graphical interface and allow parameters to be changed by simply using a point and click interface.

# Development

Development of the PC interface must be conducted with python version > 3.6 as it relies on ordered dictionaries. The following non-standard modules are used:

- [pyYAML](https://pyyaml.org/wiki/PyYAMLDocumentation)
- [PySERIAL](https://pyserial.readthedocs.io/en/latest/pyserial_api.html)
- [NumPy](http://www.numpy.org/)
- [PyInstaller](http://www.pyinstaller.org/?)

These packages are required should you want to develop / use ucConfig with it's development version ucConfig-dev located in the python_app directory.

The C API needs to be compiled with a compiler that supports the c11 standard. For GCC this can be done by passing the flag -std=c11.

