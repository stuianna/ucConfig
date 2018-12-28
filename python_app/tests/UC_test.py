import logging
import random
import time
import os

class CleanTest():

    def __init__(self,config,UC_module, header_module):

        if type(config) != dict:
            logging.warning('Config parameter should be of type dict, type = {}'.format(type(config)))

        self.UC = UC_module.UC_coms(config)
        self.head = header_module.Header(config)
        self.goodTrans = 0
        self.badTrans = 0
        self.passedTests = 0
        self.failedTests = 0

        self.testSize = config['test_UC_coms_testSize']
        self.testNumber = config['test_UC_coms_testNumber']
        self.retries = config['test_UC_coms_retries']
        self.serialPort = config['serialPort']
        self.baud = config['baud']

        return

    def runSingleTest(self,testNumber):

        byteSize = random.randint(1,self.testSize)
        dataList = self.head.generateRandomList(byteSize)
        successful = self.UC.sendList(dataList,retries=self.retries)

        if successful != len(dataList):
            self.badTrans = self.badTrans + 1
            self.goodTrans = self.goodTrans + successful
            return False
        else:
            self.goodTrans = self.goodTrans + successful


            os.system('cls' if os.name == 'nt' else 'clear')
            print('Total Tests: {}'.format(self.testNumber))
            print('Test Number: {}'.format(testNumber+1))
            print('Test Size: {}'.format(byteSize))
            print('Test Passed: {}'.format(self.passedTests))
            print('Test Failed: {}'.format(self.failedTests))
            print('Good Transactions: {}'.format(self.goodTrans))
            print('Bad Transactions: {}'.format(self.badTrans))

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
        print('Test Passed: {} ({}%)'.format(self.passedTests,round((100 * self.passedTests/self.testNumber),2)))
        print('Test Failed: {}'.format(self.failedTests))
        try:
            print('Good Transactions: {} ({}%)'.format(self.goodTrans, round(100* self.goodTrans/(self.goodTrans+self.badTrans),2)))
        except:
            print('Good Transactions: 0 (0%)')
        print('Bad Transactions: {}'.format(self.badTrans))
        print('----------------')

