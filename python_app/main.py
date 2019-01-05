import logging
import argparse
import lib.sendUC as coms
import tests.UC_test as UC_test
import tests.FULL_test as FULL_test
import lib.header as Header_C
import lib.configParser as configParser
import os
import sys

__version__ = '0.1.0-alpha'

#Get the working directory for either a frozen app or dev app
if getattr(sys, 'frozen', False):
    #Set the path to the directory where the executable is located
    dir_path = os.path.dirname(sys.executable) + os.sep
else:
    dir_path = os.path.dirname(os.path.abspath(__file__)) + os.sep

configFile = dir_path + 'config.yml'
config = None

def handleArguments(arguments):

    #allArgNames = [a['name'] for a in arguments]

    #Check in this order
    if arguments['config'] != None:
        customConfig(arguments['config'])

    if arguments['baud'] != None:
        setBaud(arguments['baud'][0])

    if arguments['port'] != None:
        setPort(arguments['port'][0])

    if arguments['logLevel'] != None:
        changeLogLevel(arguments['logLevel'][0])

    if arguments['input'] != None or arguments['output'] != None or arguments['query'] != None:
        flash(arguments)
        return

    if arguments['test'] != None:
        runTest(arguments['test'][0])

    if arguments['genConfig'] != None:
        generateConfig(arguments['genConfig'][0])

    if arguments['genExample'] != None:
        generateExample()

    if arguments['read'] != None:
        readValues(arguments['read'][0])

    return

#Set the working serial port and update the configuration file with this value
def setPort(port):

    config['serialPort'] = port
    configParser.ConfigParser().saveConf(configFile,config)
    return

#Set the working baud rate and update the configuration file with this value
def setBaud(baud):

    config['baud'] = baud
    configParser.ConfigParser().saveConf(configFile,config)
    return

#Reads flash values from microcontroller based on passed variable definition file
#Checks read value for accuracy 
#Prints information
def readValues(fileName):

    #Load the input file
    head = Header_C.Header(None)
    dataList = head.getDefinitions(fileName)

    if dataList == None:
        print('Error loading input file {}'.format(fileName))
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
def flash(arguments):

    if (arguments['output'] != None  and arguments['input'] == None):
        if arguments['query'] == None:
            print('An input of query file is needed in order to generate and output header file')
            return

    #Load the variable file either for query or input
    if arguments['query'] != None:
        if arguments['input'] != None:
            print('Can either query or write and input file, not both')
            return
        else:
            head = Header_C.Header(None)
            dataList = head.getDefinitions(arguments['query'][0])

            #check the variabels are valid.
            if dataList == None:
                print('Error loading query file {}'.format(arguments['query'][0]))
                return

            #Print out the variables
            for data in dataList:
                print('{}: {}'.format(data['name'],round(data['value'],4)))
                
            print('Variables require a total of {} bytes'.format(sum([d['size'] for d in dataList])))

    if arguments['input'] != None:
        if arguments['query'] != None:
            print('Can either query or write and input file, not both')
            return
        else:
            head = Header_C.Header(None)
            dataList = head.getDefinitions(arguments['input'][0])

            #check the variabels are valid.
            if dataList == None:
                print('Error loading input file {}'.format(arguments['input'][0]))
                return

    if arguments['output'] != None:
        #generate the header file
        if not head.generateHeader(arguments['output'][0],dataList):
            print('Error generating output header file {}'.format(arguments['output'][0]))

    ## Exit here if just a query
    if arguments['query'] != None:
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
def changeLogLevel(level):

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
def customConfig(fileName):

    config = configParser.ConfigParser().loadFileConfig(fileName[0])

    if config == None:
        exit()

    if not configParser.ConfigParser().checkConfigLimits(config):
        print('Errors in config file, try generating new file with command line options.')
        exit()

#Runs a given test
def runTest(test):

    if test == 'UC_coms_simple':

        UC_test_simple = UC_test.CleanTest(config,coms,Header_C)
        UC_test_simple.runTest()

    elif test == 'full_test':

        testFull = FULL_test.CleanTest(config,coms,Header_C,configParser)
        testFull.runTest()
    else:
        #This shouldn't happen
        print('Unknown option "{}" received for argument'.format(arg['option'],arg['name']))
        logging.warning('Unknown option "{}" received for argument'.format(arg['option'],arg['name']))
    return

#Generate the default config file in the passed location
def generateConfig(fileName):

    logging.info('Generating default config file {}'.format(fileName))

    if configParser.ConfigParser().generateConfigFile(fileName):
        print('Generating default config file {}'.format(fileName))
    else:
        print('Error generating config file {}.'.format(fileName))
    return

#Generates an example varialbe file
def generateExample():

    head = Header_C.Header(None)
    #dataList = head.generateRandomList(10)
    head.generateExample('variables.yml')

def makeArgs():

    parser = argparse.ArgumentParser(description='ucConfig CLI V{}'.format(__version__),
            formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('-i','--input',
            metavar='',type=str,nargs=1,
            help='The *.yml containting variable definitions.')
    parser.add_argument('-o','--output',
            metavar='',type=str,nargs=1,
            help='The generated C header file, requires input variable file.')
    parser.add_argument('-q','--query',
            metavar='',type=str,nargs=1,
            help='Query a variable file, requries input *.yml variable file.')
    parser.add_argument('-v','--version',
            action='version',version='ucConfig CLI V{}'.format(__version__),
            help='Display program version.')
    parser.add_argument('-ge','--genExample',
            help='Generate an example variable *.yml file.',action='store_true')
    parser.add_argument('-l','--logLevel',
            metavar='',
            help='Specify log level. \nAllowed levels:' + '\n' + 
            '\tinfo - print all messages '+ '\n' + 
            '\twarning - print only warning and error messages '+ '\n' + 
            '\tnone - print no messages '+ '\n' + 
            '\tdefault: warning.',nargs=1,type=str,
            choices=['info','warning','none'])
    parser.add_argument('-t','--test',
            metavar='',
            help='Run a module test.\n Options:' + '\n' + 
            '\tUC_coms_simple - Test UC communication with random variables'+ '\n' + 
            '\tfull_test - Run a test with good random variables, testing file parsing, generation and UC communication '+ '\n' + 
            '',nargs=1,type=str,choices=['UC_coms_simple','full_test'])
    parser.add_argument('-gc','--genConfig',
            metavar='',type=str,nargs=1,
            help='Generate a configuration file of given name in current directory.')
    parser.add_argument('-c','--config',
            metavar='',type=str,nargs=1,
            help='Use a custom configuration file.')
    parser.add_argument('-r','--read',
            metavar='',type=str,nargs=1,
            help='Read a configuration file and test its values against those in the connected UC.')
    parser.add_argument('-p','--port',
            metavar='',type=str,nargs=1,
            help='Specify the port the UC is connected to. Change is permantent.')
    parser.add_argument('-b','--baud',
            metavar='',type=int,nargs=1,
            help='Specify the baud rate for communication. Change is permantent. \nAllowable baud rates:' + '\n' +
            '\t9600 - 9600 baud rate' + '\n' +
            '\t19200 - 19200 baud rate' + '\n' +
            '\t38400 - 38400 baud rate' + '\n' +
            '\t57600 - 57600 baud rate' + '\n' +
            '\t115200 - 115200bps baud rate.' + '\n',
            choices=[9600,19200,38400,57600,115200])
    return parser


#Program entry point
if __name__ == '__main__':

    ##Make the list of arguments
    parser = makeArgs()
    
    ##Check some arguments existed
    if len(sys.argv) < 2:
        parser.print_help()
        sys.exit()

    ##Load the arguments
    arguments = vars(parser.parse_args())

    #Load the configuration file
    config = configParser.ConfigParser().loadConfig(configFile)

    if config == None:
        logging.critical('Cannot read any configuration files, exiting')
        sys.exit()

    #Set the log level
    logging.getLogger().setLevel(config['logLevel'])

    #Get input arguments, program exits if nothing valid found
    #arguments = CP.parseInput(sys.argv)

    #Act on given arguments
    handleArguments(arguments)
    
    #Finished
    sys.exit()

