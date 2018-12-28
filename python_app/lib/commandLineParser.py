import sys

version = 0.1
arguments = [
        {   'name':    'test',
            'argName': ['--test','-t'],
            'argDesc': 'Run as module test.',
            'argOpt' : ['UC_coms_simple','full_test'],
            'optDesc': ['Test UC communication with random variables',
                'Run a test with good random values which checks file parsing, generation and UC communication']
            },

        {   'name':    'generateConfig',
            'argName': ['--generateConfig','-gc'],
            'argDesc': 'Generate a config file with the default parameters.',
            'argOpt' : ['*filename*',],
            'optDesc': ['Name of file for new config file',]
            },
        {   'name':    'logLevel',
            'argName': ['--logLevel','-l'],
            'argDesc': 'Specify the log level, default level = warning.',
            'argOpt' : ['info','warning','none'],
            'optDesc': ['All log messages are shown.', 'Only warning and error messages are shown.', 'No messages are shown.',]
            },
        {   'name':    'config',
            'argName': ['--config','-c'],
            'argDesc': 'Use a custom configuration file.',
            'argOpt' : ['*filename*',],
            'optDesc': ['Name of the custom configuration file',]
            },
        {   'name':    'generateExample',
            'argName': ['--generateExample','-ge'],
            'argDesc': 'Generate an example variable definiton file "variables.yml" in current directory.',
            'argOpt' : [],
            'optDesc': []
            },

        {   'name':    'input',
            'argName': ['--input','-i'],
            'argDesc': 'Specify the input file containing the variable definitons.',
            'argOpt' : ['*filename*',],
            'optDesc': ['Name of file containing input variable definitons',]
            },
        {   'name':    'check',
            'argName': ['--query','-q'],
            'argDesc': 'Check the contents of the input file without sending them to the microcontroller',
            'argOpt' : [],
            'optDesc': []
            },
        {   'name':    'output',
            'argName': ['--output','-o'],
            'argDesc': 'Specify the name of the output generated C header file.',
            'argOpt' : ['*filename*',],
            'optDesc': ['Name of file for the generated C header file',]
            },
        {   'name':    'read',
            'argName': ['--read','-r'],
            'argDesc': 'Read variables from the microcontroller and verify them against an input variable file.',
            'argOpt' : ['*filename*',],
            'optDesc': ['Name of the file containing variable definitions',]
            },

        {   'name':    'version',
            'argName': ['--version','-v'],
            'argDesc': 'Print current program version.',
            'argOpt' : [],
            'optDesc': []
            },
        {   'name':    'port',
            'argName': ['--port','-p'],
            'argDesc': 'The serial port the microcontroller is connected to. The value entered is saved as default in config.yml',
            'argOpt' : ['*Port name*'],
            'optDesc': ['The name of the serial port']
            },
        {   'name':    'baud',
            'argName': ['--baud','-b'],
            'argDesc': 'The baud rate for the serial port, The value enetered is saved as default in config.yml',
            'argOpt' : ['9600','19200','38400','57600','115200'],
            'optDesc': ['9600 bps','19200 bps','38400 bps','57600 bps','115200 bps']
            },

        {   'name':    'help',
            'argName': ['--help','-h'],
            'argDesc': 'Show list of arguments and options.',
            'argOpt' : [],
            'optDesc': []
            }
    ]

def printHelp():

    print('UC_config version {} command line options:'.format(version))
    print()

    for i,desc in enumerate(arguments):
        print('{:<20} {:<5} -{}'.format(desc['argName'][0],desc['argName'][1],desc['argDesc']))
        if len(desc['argOpt']) != 0:
            print()
            print(' Options:')
            for j in range(len(desc['argOpt'])):
                print('  {:<18} -{}'.format(desc['argOpt'][j],desc['optDesc'][j]))
            print()

    sys.exit()

def printVersion():

    print('UC_config command line interface version {}'.format(version))
    sys.exit()

def printValidArguments(arg):

    if arg == None:
        print('No input arguments, valid arguments:')
    else:
        print('Unknown argument {}, valid arguments:'.format(arg))
    print()

    for i,desc in enumerate(arguments):
        print('{:<20} {:<5} -{}'.format(desc['argName'][0],desc['argName'][1],desc['argDesc']))

    sys.exit()

def printValidOptions(argument,entered):

    if entered == None:
        print('No option entered, valid options:')
    else:
        print('Invalid option {}, valid options:'.format(entered))

    print()

    for i in range(len(argument['argOpt'])):
        print('{:<15} -{}'.format(argument['argOpt'][i],argument['optDesc'][i]))

    sys.exit()

def parseInput(argv):

    inputDict = []
    #if len(argv) == 1:
        #printValidArguments(None)

    par = 0

    #Interpret input arguments
    for i,arg in enumerate(argv):

        #check every second argument
        if i%2 == par:
            continue

        for args in arguments:
            if arg in args['argName']:

                #First check if help or version
                if args['name'] == 'help':
                    printHelp()
                elif args['name'] == 'version':
                    printVersion()

                #Check an option exists
                if i != (len(argv)-1) and len(args['argOpt']) > 0:
                    #long enough, check if in option list
                    if argv[i+1] in args['argOpt'] or args['argOpt'][0][0] == '*':
                        #valid option found add it to inputDict
                        inputDict.append({'name':args['name'],'option':argv[i+1]})
                    else:
                        #No valid option found 
                        printValidOptions(args,argv[i+1])
                elif len(args['argOpt']) > 0:
                    #No option
                    printValidOptions(args,None)
                    sys.exit()
                else:
                    inputDict.append({'name':args['name'],'option':None})
                    if par == 0:
                        par = 1
                    else:
                        par = 0
        if i == (len(argv)-1) and len(inputDict) == 0:
            printValidArguments(arg)
            sys.exit()
            
    if len(inputDict) == 0:
        printValidArguments(None)
        sys.exit()

    return inputDict
    
