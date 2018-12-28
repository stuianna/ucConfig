import logging
import lib.sendUC as coms
import tests.UC_test as UC_test
import tests.FULL_test as FULL_test
import lib.commandLineParser as CP
import lib.header as Header_C
import lib.configParser as configParser
import os
import sys

#Get the working directory for either a frozen app or dev app
if getattr(sys, 'frozen', False):
    #Set the path to the directory where the executable is located
    dir_path = os.path.dirname(sys.executable) + os.sep
else:
    dir_path = os.path.dirname(os.path.abspath(__file__)) + os.sep

configFile = dir_path + 'config.yml'
config = None

def handleArguments(arguments):

    allArgNames = [a['name'] for a in arguments]

    #Check in this order
    if 'config' in allArgNames:
        customConfig(arguments[allArgNames.index('config')])

    if 'baud' in allArgNames:
        setBaud(arguments[allArgNames.index('baud')])

    if 'port' in allArgNames:
        setPort(arguments[allArgNames.index('port')])

    if 'logLevel' in allArgNames:
        changeLogLevel(arguments[allArgNames.index('logLevel')])

    if 'input' in allArgNames or 'output' in allArgNames or 'check' in allArgNames:
        flash(arguments,allArgNames)
        return

    #Act on given arguments
    for arg in arguments:

        if arg['name'] == 'test':
            runTest(arg)
        elif arg['name'] == 'generateConfig':
            generateConfig(arg)
        elif arg['name'] == 'generateExample':
            generateExample()
        elif arg['name'] == 'port':
            pass
        elif arg['name'] == 'baud':
            pass
        elif arg['name'] == 'logLevel':
            pass
        elif arg['name'] == 'config':
            pass
        elif arg['name'] == 'read':
            readValues(arg)

        else:
            #This shouldn't happen
            print('Unknown argument "{}" received'.format(arg['name']))
            logging.warning('Unknown argument "{}" received'.format(arg['name']))

    return

#Set the working serial port and update the configuration file with this value
def setPort(arg):

    config['serialPort'] = arg['option']
    configParser.ConfigParser().saveConf(configFile,config)
    return

#Set the working baud rate and update the configuration file with this value
def setBaud(arg):

    config['baud'] = int(arg['option'])
    configParser.ConfigParser().saveConf(configFile,config)
    return

#Reads flash values from microcontroller based on passed variable definition file
#Checks read value for accuracy 
#Prints information
def readValues(arg):

    #Load the input file
    head = Header_C.Header(None)
    dataList = head.getDefinitions(arg['option'])

    if dataList == None:
        print('Error loading input file {}'.format(arg['option']))
        return

    UC = coms.UC_coms(config)

    if not UC.connectSerial(retries=config['retries']):
        print('Error connecting to device on port {}'.format(config['serialPort']))
        return

    readList =  UC.readList(dataList)
    UC.closeSerial()

    if readList == None:
        print('Error reading data from microcontroller.')

    print('{:<32}{:<20}{:<20}'.format('Variable','Value','Read'))
    for read in readList:
        print('{:<32}{:<20}{:<20}'.format(read['name'],round(read['value'],4),read['read']))

    return

#Sends variables in given variable definition file to microcontroller
#Verifies sent variables for accuracy
def flash(args,names):

    if 'output' in names and 'input' not in names:
        print('An input file is needed in order to flash or generate and output header file')
        return

    if 'check' in names and 'input' not in names:
        print('An input file is need in order to check the variable parameters')
        return

    #Get the index of the input argument
    inIndex = names.index('input')

    #Load the input file
    head = Header_C.Header(None)
    dataList = head.getDefinitions(args[inIndex]['option'])


    if dataList == None:
        print('Error loading input file {}'.format(args[inIndex]['option']))
        return

    if 'output' in names:

        #get the index of the output file argument
        outIndex = names.index('output')

        #generate the header file
        if not head.generateHeader(args[outIndex]['option'],dataList):
            print('Error generating output header file {}'.format(args[outIndex]['option']))

    if 'check' in names:
        for data in dataList:
            print('{}: {}'.format(data['name'],round(data['value'],4)))
            
        print('Variables require a total of {} bytes'.format(sum([d['size'] for d in dataList])))
        return

    UC = coms.UC_coms(config)

    if not UC.connectSerial(retries=config['retries']):
        print('Error connecting to device on port {}'.format(config['serialPort']))
        return

    for i in range(config['retries']):
        if UC.sendList(dataList) == len(dataList):
            #Same size means everythin was sent
            UC.closeSerial()
            break

        if i == (config['retries'] -1):
            #Error sending
            print('Unable to send data to microcontroller, check logs')
            UC.closeSerial()
            return

    #Everything good, print information.
    print('Flashed:')
    print('-----------------')
    for data in dataList:
        print('{}: {}'.format(data['name'],round(data['value'],4)))
    print('-----------------')
    print('Successfully verified {} bytes.'.format(sum([d['size'] for d in dataList])))

#Sets the log level for all modules
def changeLogLevel(arg):

    level = arg['option']

    if level == 'info':
        logging.getLogger().setLevel(logging.INFO)
    elif level == 'warning':
        logging.getLogger().setLevel(logging.WARNING)
    elif level == 'none':
        logging.getLogger().setLevel(logging.OFF)
    else:
        #Shouldn't get here
        logging.warning('Invalid log level passed')
        print('Invalid log level passed')

#Runs the application with a custom passed config file
def customConfig(arg):

    config = configParser.ConfigParser().loadFileConfig(arg['option'])

    if config == None:
        exit()

    if not configParser.ConfigParser().checkConfigLimits(config):
        print('Errors in config file, try generating new file with command line options.')
        exit()

#Runs a given test
def runTest(arg):

    if arg['option'] == 'UC_coms_simple':

        UC_test_simple = UC_test.CleanTest(config,coms,Header_C)
        UC_test_simple.runTest()
    elif arg['option'] == 'full_test':

        testFull = FULL_test.CleanTest(config,coms,Header_C,configParser)
        testFull.runTest()
    else:
        #This shouldn't happen
        print('Unknown option "{}" received for argument'.format(arg['option'],arg['name']))
        logging.warning('Unknown option "{}" received for argument'.format(arg['option'],arg['name']))
    return

#Generate the default config file in the passed location
def generateConfig(arg):

    logging.info('Generating default config file {}'.format(arg['option']))

    if configParser.ConfigParser().generateConfigFile(arg['option']):
        print('Generating default config file {}'.format(arg['option']))
    else:
        print('Error generating config file {}.'.format(arg['option']))

    return

#Generates an example varialbe file
def generateExample():

    head = Header_C.Header(None)
    #dataList = head.generateRandomList(10)
    head.generateExample('variables.yml')

#Program entry point
if __name__ == '__main__':

    #Load the configuration file
    config = configParser.ConfigParser().loadConfig(configFile)

    if config == None:
        logging.critical('Cannot read any configuration files, exiting')
        sys.exit()

    #Set the log level
    logging.getLogger().setLevel(config['logLevel'])

    #Get input arguments, program exits if nothing valid found
    arguments = CP.parseInput(sys.argv)

    #Act on given arguments
    handleArguments(arguments)
    
    #Finished
    sys.exit()






