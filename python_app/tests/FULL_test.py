import logging
import random
import time
import os
import sys

if getattr(sys, 'frozen', False):
    # If the application is run as a bundle, the pyInstaller bootloader
    # extends the sys module by a flag frozen=True and sets the app 
    # path into variable _MEIPASS'.
    dir_path = sys._MEIPASS + os.sep

else:
    dir_path = os.path.dirname(os.path.abspath(__file__)) + os.sep

class CleanTest():

    #Generate random variable file
    #Read the file
    #Send to UC with verify
    #Read value back and check with random variable file

    def __init__(self,config,UC_module, header_module,config_module):

        if type(config) != dict:
            logging.warning('Config parameter should be of type dict, type = {}'.format(type(config)))

        self.UC = UC_module.UC_coms(config)
        self.head = header_module.Header(config)
        self.config = config_module.ConfigParser();
        self.goodTrans = 0
        self.badTrans = 0
        self.passedTests = 0
        self.failedTests = 0
        self.bytes = 0

        self.testSize = config['test_full_testSize']
        self.testNumber = config['test_full_testNumber']
        self.retries = config['test_full_retries']
        self.serialPort = config['serialPort']
        self.baud = config['baud']
        return

    def runSingleTest(self,testNumber):

        byteSize = random.randint(1,self.testSize)
        dataList = self.head.generateRandomList(byteSize)
        self.head.generateDefinition(dir_path + 'tempVariables.yml',dataList)
        readDataList = self.head.getDefinitions(dir_path + 'tempVariables.yml')
        self.bytes += byteSize

        if dataList != readDataList:
            logging.warning('Random dataList does not match read generated list')
            self.bytes -= byteSize
            return False

        for attempt in range(self.retries):

            successful = self.UC.sendList(readDataList,retries=self.retries)

            if successful != len(readDataList):
                self.badTrans = self.badTrans + 1
                self.goodTrans = self.goodTrans + successful
                if attempt == (self.retries - 1) :
                    self.bytes -= byteSize
                    return False
                else:
                    logging.warning('Failed sending data set, attempt number {}'.format(attempt+1))
                    continue
            else:
                self.goodTrans = self.goodTrans + successful
                break

            readList = self.UC.readList(readDataList)

            if False in [read['correct'] for read in readList]:
                return False

        os.system('cls' if os.name == 'nt' else 'clear')
        print('Total Tests: {}'.format(self.testNumber))
        print('Test Number: {}'.format(testNumber+1))
        print('Test Size: {}'.format(byteSize))
        print('Tests Passed: {}'.format(self.passedTests))
        print('Tests Failed: {}'.format(self.failedTests))
        print('Good Transactions: {}'.format(self.goodTrans))
        print('Bad Transactions: {}'.format(self.badTrans))
        print('Total Bytes: {}'.format(self.bytes))

        return True

    def runTest(self):

        self.goodTrans = 0
        self.badTrans = 0
        self.passedTests = 0
        self.failedTests = 0

        if not self.UC.connectSerial(self.serialPort,self.baud,self.retries):
            logging.error('Cannot connect to serial port, exiting')
            return

        for test in range(self.testNumber):

            if self.runSingleTest(test) == True:
                self.passedTests = self.passedTests + 1
            else:
                self.failedTests = self.failedTests + 1

        self.UC.closeSerial()

        os.system('cls' if os.name == 'nt' else 'clear')
        print('----------------')
        print('Finished tests')
        print('Tests Passed: {} ({}%)'.format(self.passedTests,round((100 * self.passedTests/self.testNumber),2)))
        print('Tests Failed: {}'.format(self.failedTests))
        try:
            print('Good Transactions: {} ({}%)'.format(self.goodTrans, round(100* self.goodTrans/(self.goodTrans+self.badTrans),2)))
        except:
            print('Good Transactions: 0 (0%)')
        print('Bad Transactions: {}'.format(self.badTrans))
        print('Total Bytes: {}'.format(self.bytes))
        print('----------------')

