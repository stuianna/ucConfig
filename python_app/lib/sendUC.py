import serial
import logging
import re
import time
import numpy as np

UCCONFIG_KEY = [2,4,6,8]

UCCONFIG_FRAME_END = 22
UCCONFIG_SET_MEMORY_ADDRESS = 12
UCCONFIG_WRITE_FRAME = 13
UCCONFIG_READ_FRAME = 14
UCCONFIG_TERMINATE = 15
UCCONFIG_AT_ADDRESS = 16
UCCONFIG_ACK = 17
UCCONFIG_NACK = 18

UCCONFIG_NULL = 19
UCCONFIG_NOT_USED = 20
UCCONFIG_NEWLINE = 10


UCCONFIG_TYPE_NONE = 11 
UCCONFIG_TYPE_UINT8_T = 12
UCCONFIG_TYPE_INT8_T = 13
UCCONFIG_TYPE_UINT16_T = 14
UCCONFIG_TYPE_INT16_T = 15
UCCONFIG_TYPE_UINT32_T = 16
UCCONFIG_TYPE_INT32_T = 17
UCCONFIG_TYPE_FLOAT = 18
UCCONFIG_TYPE_CHAR = 19

UCCONFIG_LENGTH_ZERO = 21

ack_length = 4
nack_length = 4

ucconfig_ack = bytearray([
        
        UCCONFIG_ACK,
        UCCONFIG_NULL,
        UCCONFIG_FRAME_END,
        UCCONFIG_NEWLINE
        ])

ucconfig_nack = bytearray([
        UCCONFIG_NACK,
        UCCONFIG_NULL,
        UCCONFIG_FRAME_END,
        UCCONFIG_NEWLINE,
        ])

ucconfig_terminate = bytearray([
        UCCONFIG_TERMINATE,
        UCCONFIG_NULL,
        UCCONFIG_TYPE_NONE,
        UCCONFIG_LENGTH_ZERO,
        UCCONFIG_NOT_USED,
        UCCONFIG_NOT_USED,
        UCCONFIG_NULL,
        UCCONFIG_FRAME_END,
        ])

ucconfig_setMemoryHeader = bytearray([
        UCCONFIG_SET_MEMORY_ADDRESS,
        UCCONFIG_NULL,
        UCCONFIG_TYPE_NONE,
        ])

ucconfig_writeFrameHeader = bytearray([
        UCCONFIG_WRITE_FRAME,
        UCCONFIG_NULL,
        ])

ucconfig_atAddressHeader = bytearray([
        UCCONFIG_AT_ADDRESS,
        UCCONFIG_NULL,
        UCCONFIG_TYPE_NONE,
        UCCONFIG_LENGTH_ZERO,
        UCCONFIG_NOT_USED,
        UCCONFIG_NOT_USED,
        ])

ucconfig_getDataHeader = bytearray([
        UCCONFIG_READ_FRAME,
        UCCONFIG_NULL,
        ])

ucconfig_getMemory = bytearray([
        UCCONFIG_AT_ADDRESS,
        UCCONFIG_NULL,
        UCCONFIG_TYPE_NONE,
        UCCONFIG_LENGTH_ZERO,
        UCCONFIG_NOT_USED,
        UCCONFIG_NOT_USED,
        UCCONFIG_NULL,
        UCCONFIG_FRAME_END,
        ])

ucconfig_getData = bytearray([
        UCCONFIG_LENGTH_ZERO,
        UCCONFIG_NOT_USED,
        UCCONFIG_NOT_USED,
        UCCONFIG_NULL,
        UCCONFIG_FRAME_END,
        ])

class UC_coms:

    def __init__(self,conf):

        self.conf = conf
        self.ser = None
        self.inConfig = False
        self.readTimeout = conf['readTimeout']
        self.portName = conf['serialPort']
        self.baud = conf['baud']
        return

    def connectSerial(self,portName=None,baud=None,retries=1):

        if portName == None:
            portName = self.portName

        if baud == None:
            baud = self.baud

        self.ser = serial.Serial(timeout=self.readTimeout)
        self.ser.baudrate = baud
        self.ser.port = portName

        for r in range(retries):
            try:
                self.ser.open()
                logging.info('Opened serial port: {} successfully'.format(portName))
                return True
                break
            except:
                logging.warning('Cannot open serial port: {}, attempt number {}'.format(portName,r+1))
                time.sleep(0.1)

        return False

    def closeSerial(self):

        if  self.ser == None:
            logging.warning('Tyring to close a serial port which is not open.')
            return False

        if not self.ser.is_open:
            logging.warning('Tyring to close a serial port which is not open.')
            return False


        self.ser.close()
        logging.info('Closed serial port: {}'.format(self.portName))
        return True

    def setData(self,data,dataType):

        if  self.ser == None:
            logging.warning('Tyring to write data on serial port which is not open.')
            return False

        if not self.ser.is_open:
            logging.warning('Tyring to write data on serial port which is not open.')
            return False

        if not self.inConfig:
            logging.warning('Trying to write data when not in config mode')
            return False

        if type(data) != str:
            logging.warning('Argument "Data" must be of type string, type: {}'.format(type(data)))
            return False

        if dataType == 'char' and len(data) != 1:
            logging.warning('Trying to write a character longer that length 1, data: {}'.format(data))
            return False

        logging.info('Writing data {} of type {} to current flash address'.format(data,dataType))

        if self.writeSerial(ucconfig_writeFrameHeader) == False:
            return False

        #Write the data type
        if dataType == 'uint8_t':
            self.writeSerial(UCCONFIG_TYPE_UINT8_T.to_bytes(1,'little'))
        elif dataType == 'int8_t':
            self.writeSerial(UCCONFIG_TYPE_INT8_T.to_bytes(1,'little'))
        elif dataType == 'uint16_t':
            self.writeSerial(UCCONFIG_TYPE_UINT16_T.to_bytes(1,'little'))
        elif dataType == 'int16_t':
            self.writeSerial(UCCONFIG_TYPE_INT16_T.to_bytes(1,'little'))
        elif dataType == 'uint32_t':
            self.writeSerial(UCCONFIG_TYPE_UINT32_T.to_bytes(1,'little'))
        elif dataType == 'int32_t':
            self.writeSerial(UCCONFIG_TYPE_INT32_T.to_bytes(1,'little'))
        elif dataType == 'float':
            self.writeSerial(UCCONFIG_TYPE_FLOAT.to_bytes(1,'little'))
        elif dataType == 'char':
            self.writeSerial(UCCONFIG_TYPE_CHAR.to_bytes(1,'little'))
        else:
            logging.warning('Trying to write invalid data type: {}'.format(dataType))
            return False

        #Write the length of the data
        self.writeSerial((len(data) + 64).to_bytes(1,'little'))

        #Now two not used characters
        self.writeSerial(UCCONFIG_NOT_USED.to_bytes(1,'little'))
        self.writeSerial(UCCONFIG_NOT_USED.to_bytes(1,'little'))

        #Now the data
        self.writeSerial(data.encode('UTF-8'))

        #End of frame stuff
        self.writeSerial(UCCONFIG_NULL.to_bytes(1,'little'))
        self.writeSerial(UCCONFIG_FRAME_END.to_bytes(1,'little'))

        return self.getAck()

    def getData(self,dataType):

        if  self.ser == None:
            logging.warning('Tyring to get data on serial port which is not open.')
            return None

        if not self.ser.is_open:
            logging.warning('Tyring to get data on serial port which is not open.')
            return None

        if not self.inConfig:
            logging.warning('Trying to get data when not in config mode')
            return None

        logging.info('Requesting data at current flash address')

        if self.writeSerial(ucconfig_getDataHeader) == None:
            return None

        #Write the data type
        if dataType == 'uint8_t':
            self.writeSerial(UCCONFIG_TYPE_UINT8_T.to_bytes(1,'little'))
        elif dataType == 'int8_t':
            self.writeSerial(UCCONFIG_TYPE_INT8_T.to_bytes(1,'little'))
        elif dataType == 'uint16_t':
            self.writeSerial(UCCONFIG_TYPE_UINT16_T.to_bytes(1,'little'))
        elif dataType == 'int16_t':
            self.writeSerial(UCCONFIG_TYPE_INT16_T.to_bytes(1,'little'))
        elif dataType == 'uint32_t':
            self.writeSerial(UCCONFIG_TYPE_UINT32_T.to_bytes(1,'little'))
        elif dataType == 'int32_t':
            self.writeSerial(UCCONFIG_TYPE_INT32_T.to_bytes(1,'little'))
        elif dataType == 'float':
            self.writeSerial(UCCONFIG_TYPE_FLOAT.to_bytes(1,'little'))
        elif dataType == 'char':
            self.writeSerial(UCCONFIG_TYPE_CHAR.to_bytes(1,'little'))
        else:
            logging.warning('Trying to write invalid data type: {}'.format(dataType))
            return False

        self.writeSerial(ucconfig_getData)

        response = self.readLine()

        if response == None:
            return None

        #Basic Check for Nack
        if response[0] == UCCONFIG_NACK:
            logging.warning('Not acknowleged')
            return None


        if len(response) < 9:
            logging.warning('Length of received data frame is too short, received {}'.format(response))
            return None

        #Check correct header
        if response[:2] != ucconfig_getDataHeader:
            logging.warning('Incorrect get data header, received {}'.format(response))
            return None

        #Read the type
        dataType = None


        if chr(response[2]) == chr(UCCONFIG_TYPE_UINT8_T):
            dataType = 'uint8_t'
        elif chr(response[2]) == chr(UCCONFIG_TYPE_INT8_T):
            dataType = 'int8_t'
        elif chr(response[2]) == chr(UCCONFIG_TYPE_UINT16_T):
            dataType = 'uint16_t'
        elif chr(response[2]) == chr(UCCONFIG_TYPE_INT16_T):
            dataType = 'int16_t'
        elif chr(response[2]) == chr(UCCONFIG_TYPE_UINT32_T):
            dataType = 'uint32_t'
        elif chr(response[2]) == chr(UCCONFIG_TYPE_INT32_T):
            dataType = 'int32_t'
        elif chr(response[2]) == chr(UCCONFIG_TYPE_FLOAT):
            dataType = 'float'
        elif chr(response[2]) == chr(UCCONFIG_TYPE_CHAR):
            dataType = 'char'
        else:
            logging.warning('Received invalid data type, received {}'.format(response))
            return None



        #Check the length of the data matches the actual data
        if len((response[6:])[:-3]) > 24:
            logging.warning('Length of data received is too long, received {}'.format(response))
            return None

        #Now should be two not used
        if chr(response[4]) != chr(UCCONFIG_NOT_USED) and chr(response[5]) != chr(UCCONFIG_NOT_USED):
            logging.warning('Not used bytes contain invalid characters, received {}'.format(response))
            return None

        #Check if float
        if '.' in str(response[6:]) and dataType != 'char':
            if dataType != 'float':
                logging.warning('Decimal point in data but float type not specified, received {}'.format(response))
                return None
            else:
                try:
                    data = float((response[6:])[:-3])
                except:
                    logging.warning('Cannot parse float from {}'.format(response))
                    return None

        elif dataType == 'char':
            data = chr(response[6])

        else:
            try: 
                data = int((response[6:])[:-3])
            except:
                logging.warning('Cannot parse int from {}'.format(response))
                return None

        #Double check the Null character exists
        if chr(response[-3]) != chr(UCCONFIG_NULL):
            logging.warning('Null character not found, received: {}'.format(response))
            return None


        #All good
        logging.info('Data {} of type {} at current address'.format(data,dataType))
        return data

    def getMemoryAddress(self):

        if  self.ser == None:
            logging.warning('Tyring to get memory address on serial port which is not open.')
            return None

        if not self.ser.is_open:
            logging.warning('Tyring to get memory address on serial port which is not open.')
            return None

        if not self.inConfig:
            logging.warning('Trying to get memory address when not in config mode')
            return None

        logging.info('Requesting current flash address')

        if self.writeSerial(ucconfig_getMemory) == False:
            return None

        response = self.readLine()

        if response == None:
            return None

        #Basic Check for Nack
        if response[0] == UCCONFIG_NACK:
            logging.warning('Not acknowleged')
            return None


        if len(response) < 9:
            logging.warning('Length of received address frame is too short, received {}'.format(response))
            return None


        #Check correct header
        if response[:6] != ucconfig_atAddressHeader:
            logging.warning('Incorrect get address header, received {}'.format(response))
            return None

        #Now read the data
        address = re.sub('[^0-9]','', (response[6:]).decode('UTF-8'))
        address = int(address)

        #Check range
        if (address < 0 or address > 65536):
            logging.warning('Received address out of range, received {}'.format(response))
            return None

        #All good
        logging.info('Current flash address: {}'.format(address))
        return address


    def setMemoryAddress(self,address):

        if  self.ser == None:
            logging.warning('Tyring to set memory address on serial port which is not open.')
            return False

        if not self.ser.is_open:
            logging.warning('Tyring to set memory address on serial port which is not open.')
            return False

        if not self.inConfig:
            logging.warning('Trying to set memory address when not in config mode')
            return False

        if type(address) != str:
            logging.warning('Address argument must be a string')
            return False

        logging.info('Setting memory address to {}'.format(address))

        if self.writeSerial(ucconfig_setMemoryHeader) == None:
            return False

        addressLength = (len(address) + 64).to_bytes(1, 'little')
        self.writeSerial(addressLength)
        self.writeSerial(UCCONFIG_NOT_USED.to_bytes(1,'little'))
        self.writeSerial(UCCONFIG_NOT_USED.to_bytes(1,'little'))
        self.writeSerial(address.encode('UTF-8'))
        self.writeSerial(UCCONFIG_NULL.to_bytes(1,'little'))
        self.writeSerial(UCCONFIG_FRAME_END.to_bytes(1,'little'))
        return self.getAck()

    def getAck(self):

        response = self.readLine()

        if response == None:
            return False

        return self.parseAck(response)

    def parseAck(self,response):

        if len(response) != ack_length:
            logging.warning('Wrong acknowledge length, received {}'.format(response))
            return False

        if response == ucconfig_ack:
            logging.info('Acknowledged')
            return True
        elif response == ucconfig_nack:
            logging.info('Not Acknowledged.')
            return False
        else:
            logging.warning('Unknown acknowledgement {}.'.format(response))
            return False
        return False

    def sendList(self,dataList,verify=True,retries=1):

        if not self.enterConfigMode():
            logging.warning('Failed to enter config mode')
            self.exitConfigMode()
            return 0

        numberSent = 0
        if not self.setMemoryAddress(str(0)):
            logging.warning('Failed to set memory address')
            return 0

        for data in dataList:

            if not self.send(data['value'],data['dataType'],verify,retries):
                logging.warning('Failed sending data "{}" of value {} and type {}'.format(data['name'],data['value'],data['dataType']))
                self.exitConfigMode()
                return numberSent
            else:
                numberSent = numberSent + 1

        self.exitConfigMode()
        return numberSent

    def readList(self,dataList,retries=1):

        if not self.enterConfigMode():
            logging.warning('Failed to enter config mode')
            self.exitConfigMode()
            return 0

        if not self.setMemoryAddress(str(0)):
            logging.warning('Failed to set memory address')
            self.exitConfigMode()
            return 0

        readList = []
        readDict = {
                'name':None,
                'value':None,
                'read':None,
                'correct':None
                }

        for data in dataList:

           succeed,value,correct = self.read(data['value'],data['dataType'],retries)

           if succeed == False:
                self.exitConfigMode()
                return None

           readDict['name'] = data['name']
           readDict['value'] = data['value']
           readDict['read'] = value
           readDict['correct'] = correct
           readList.append(readDict.copy())

        self.exitConfigMode()

        return readList

    def read(self,data,dataType,retries=1):

        originalAddress = None

        #Get the orignal address
        for r in range(retries):
            originalAddress = self.getMemoryAddress()
            if originalAddress != None:
                break
            else:
                logging.warning('Attempting to get memory address again, attempt: {}'.format(r+1))

            if originalAddress == None:

                logging.warning('Could not get memory address after {} attempts'.format(r))
                return False,None,False

        #Got the memory address now read data

        for r in range(retries):

            readValue = self.getData(dataType)

            if readValue == None:
                logging.warning('Failed reading data back for verification on attempt number: {}'.format(r+1))
                continue

            #Got some data back, check if it matches
            if dataType == 'char':
                if readValue == chr(data):
                    return True,readValue,True
                else:
                    logging.warning('Received incorrect data for verification on attempt number {}'.format(r+1))

            elif dataType == 'float':
                if np.isclose(readValue,float(data),atol=0.5):
                    return True,readValue,True
                else:
                    logging.warning('Received incorrect data for verification on attempt number {}'.format(r+1))

            else:
                if readValue == int(data):
                    return True,readValue,True
                else:
                    logging.warning('Received incorrect data for verification on attempt number {}'.format(r+1))

            #Incorrect data was received so the memory address needs to be reset
            if not self.setMemoryAddress(str(originalAddress)):
                logging.warning('Failed setting the orignal address for repeat verification on attempt number: {}'.format(r+1))

        return True,readValue,False



    def send(self,data,dataType,verify=True,retries=1):

        originalAddress = None

        #Get the orignal address (only if need to verify)
        if verify == True:
            for r in range(retries):
                originalAddress = self.getMemoryAddress()
                if originalAddress != None:
                    break
                else:
                    logging.warning('Attempting to get memory address again, attempt: {}'.format(r+1))

            if originalAddress == None:

                logging.warning('Could not get memory address after {} attempts'.format(r))
                return False

        #Got the memory address now write data
        for r in range(retries):

            if dataType == 'char':
                if not self.setData(chr(data),dataType):
                    logging.warning('Failed setting data on attempt number {}'.format(r+1))
                    continue
                else:
                    break
            else:
                if not self.setData(str(data),dataType):
                    logging.warning('Failed setting data on attempt number {}'.format(r+1))
                    continue
                else:
                    break


        #Finished if don't need to verify
        if verify == False:
            return True


        #Set back to orignal address so if can be verified
        for r in range(retries):

            if not self.setMemoryAddress(str(originalAddress)):
                logging.warning('Failed setting the orignal address for verification on attempt number: {}'.format(r+1))
                continue
            else:
                break

        #Verify the data
        for r in range(retries):

            readValue = self.getData(dataType)

            if readValue == None:
                logging.warning('Failed reading data back for verification on attempt number: {}'.format(r+1))
                continue

            #Got some data back, check if it matches
            if dataType == 'char':
                if readValue == chr(data):
                    return True
                else:
                    logging.warning('Received incorrect data for verification on attempt number {}'.format(r+1))

            elif dataType == 'float':
                if np.isclose(readValue,float(data),atol=0.5):
                    return True
                else:
                    logging.warning('Received incorrect data for verification on attempt number {}'.format(r+1))

            else:
                if readValue == int(data):
                    return True
                else:
                    logging.warning('Received incorrect data for verification on attempt number {}'.format(r+1))

            #Incorrect was received so the memory address needs to be reset
            if not self.setMemoryAddress(str(originalAddress)):
                logging.warning('Failed setting the orignal address for repeat verification on attempt number: {}'.format(r+1))

    def enterConfigMode(self,retries=1):

        if self.ser is None:
            logging.warning('Tyring to enter config mode on a serial port which is not open.')
            return False

        if not self.ser.is_open:
            logging.warning('Tyring to enter config mode on a serial port which is not open.')
            return False

        for r in range(retries):

            if not self.flushInput():
                return False

            if not self.flushOutput():
                return False

            if self.writeSerial(UCCONFIG_KEY) == False:
                return False


            if self.getAck() == True:
                self.inConfig = True
                logging.info('Entering config mode.')
                return True
            else:
                logging.warning('Failed to enter config mode on attempt number {}'.format(r+1))

        self.inConfig = False
        return False

        
    def exitConfigMode(self,retries=1):

        if self.ser is None:
            logging.warning('Tyring to exit config mode on a serial port which is not open.')
            return False

        if not self.ser.is_open:
            logging.warning('Tyring to exit config mode on a serial port which is not open.')
            return False

        for r in range(retries):

            if not self.flushInput():
                return False
            if not self.flushOutput():
                return False
            if self.writeSerial(ucconfig_terminate) == None:
                return False


            if self.getAck() == True:
                self.inConfig = False
                logging.info('Exiting config mode.')
                return True
            else:
                logging.warning('Failed to exit config mode on attempt number {}'.format(r+1))

        self.inConfig = True
        return False

    def writeSerial(self,stream):

        try:
            while self.ser.out_waiting > 0:
                pass
            self.ser.write(stream)
            return True
        except:
            logging.warning('Port busy')
            return False

    def readLine(self):

        try:
            response = self.ser.readline()
            if len(response) == 0:
                logging.warning('Port timeout. Current timeout = {}'.format(readTimeout))
                return None
            return response
        except:
            logging.warning('Error reading from port.')
            return None

    
    def flushInput(self):
        try:
            self.ser.flushInput()
        except:
            logging.warning('Error flushing input from port.')
            return False
        return True

    def flushOutput(self):
        try:
            self.ser.flushOutput()
        except:
            logging.warning('Error flushing output from port.')
            return False
        return True
