import numpy as np
import random
import logging
import datetime
import yaml

__version__ = '0.1.0-alpha'

##########################
    #Variable types
##########################

#Float is limited by signed 32 bit integer size divied by 100
#as this is how it is currently stored in EEPROM memory
types = [
        {'name':'uint8_t',
            'size': 1,
            'min': 0,
            'max': 255},
        {'name':'int8_t',
            'size': 1,
            'min': -127,
            'max': 127},
        {'name':'uint16_t',
            'size': 2,
            'min': 0,
            'max': 2**16 -1},
        {'name':'int16_t',
            'size': 2,
            'min': -2**15 + 1,
            'max': 2**15 -1},
        {'name':'uint32_t',
            'size': 4,
            'min': 0,
            'max': 2**32 -1},
        {'name':'int32_t',
            'size': 4,
            'min': -2**31 + 1,
            'max': 2**31 -1},
        {'name':'float',
            'size': 4,
            'min': (-2**31 + 1) / 10000,
            'max': (2**31 - 1) / 10000},
        {'name':'char',
            'size': 1,
            'min': 32,
            'max': 127}
        ]

class Header():


    def __init__(self,config):

        return


    def checkLimits(self,data,dataType):

        listIndex = self.getTypeIndex(dataType)

        if listIndex == None:
            return False

        if listIndex == None:
            return False

        if data > types[listIndex]['max']:
            logging.warning('Value of {} too large for type {}'.format(data,dataType))
            return False

        if data < types[listIndex]['min']:
            logging.warning('Value of {} too small for type {}'.format(data,dataType))
            return False
        return True

    def generateRandomList(self,byteLength=1):

        type_name_list = [v['name'] for v in types]
        type_name_length = len(type_name_list)
        current_length = 0;
        dataList = []
        dataValue = None
        dataNumber = 0

        while current_length != byteLength:

            ranTypeIndex = random.randint(0,type_name_length-1)

            current_length = current_length + types[ranTypeIndex]['size']

            if current_length > byteLength:
                current_length = current_length - types[ranTypeIndex]['size']
                continue

            dataValue = self.generateRandomValue(types[ranTypeIndex]['name'])
            dataList.append({
                'name':'random_variable_{}'.format(dataNumber),
                'desc':'A randomly generated variable',
                'dataType':types[ranTypeIndex]['name'],
                'value':dataValue,
                'min':types[ranTypeIndex]['min'],
                'max':types[ranTypeIndex]['max'],
                'size':types[ranTypeIndex]['size']})
            dataNumber = dataNumber + 1

        return dataList

    def getTypeIndex(self,dataType):

        listIndex = None
        #Get list index / check if valid datatype
        type_name_list = [v['name'] for v in types]

        if dataType not in type_name_list:
            logging.warning('Unknown type {}, use valid type names {}'.format(dataType,type_name_list))
            return None
        else:
            listIndex = type_name_list.index(dataType)

        return listIndex

    def generateRandomValue(self,dataType):

        data = None

        listIndex = self.getTypeIndex(dataType)
        if listIndex == None:
            return None

        if dataType != 'float':

            data = random.randint(types[listIndex]['min'],types[listIndex]['max'])
        else:

            data = random.uniform(types[listIndex]['min'],types[listIndex]['max'])

        logging.info('Random value {} of type {} generated'.format(data,dataType))
        return data

    def getDefinitions(self,filename):

        dataList = None

        try:
            with open(filename,'r') as ymlfile:
                dataList = yaml.load(ymlfile)
        except:
            logging.warning('Cannot open definition file "{}" containing variable values'.format(filename))
            return None

        for data in dataList:
            if not self.checkKeys(data):
                return None
            if not self.checkLimits(data['value'],data['dataType']):
                index = self.getTypeIndex(data['dataType'])
                if index == None:
                    return None
                logging.warning('Variable {} with value {} out of range for type {},minimum: {}, maximum: {}'.format(
                            data['name'],
                            data['value'],
                            data['dataType'],
                            types[index]['min'],
                            types[index]['max']))
                return None
            if not self.checkLimits(data['min'],data['dataType']):
                index = self.getTypeIndex(data['dataType'])
                if index == None:
                    return None
                logging.warning('Variable {} minimum value {} too low for type {},minimum: {}, maximum: {}'.format(
                            data['name'],
                            data['min'],
                            data['dataType'],
                            types[index]['min'],
                            types[index]['max']))
                return None
            if not self.checkLimits(data['max'],data['dataType']):
                index = self.getTypeIndex(data['dataType'])
                if index == None:
                    return None
                logging.warning('Variable {} maximum value {} too high for type {},minimum: {}, maximum: {}'.format(
                            data['name'],
                            data['max'],
                            data['dataType'],
                            types[index]['min'],
                            types[index]['max']))
                return None
            if data['value'] > data['max']:
                logging.warning('Variable {} value {} greater than maximum: {}'.format(
                            data['name'],
                            data['value'],
                            data['max']))
                return None
            if data['value'] < data['min']:
                logging.warning('Variable {} value {} less than minimum: {}'.format(
                            data['name'],
                            data['value'],
                            data['min']))
                return None

            index = self.getTypeIndex(data['dataType'])
            data['size'] = types[index]['size']

        ##Check for duplicate varaible names
        varNames = np.array([d['name'] for d in dataList])

        for var in varNames:
            if len(np.where(varNames == var)[0]) > 1:
                logging.warning('Cannot have duplicate variable names, duplicate: {}'.format(var))
                return None

        return dataList

    def generateExample(self,filename):

        outStream = '# Example UC config file demonstrating correct variable layout.\n'
        outStream += '# There are six required parameters for each variable \n'
        outStream += '#\tname - The name of the variable, must conform to C naming convention. \n'
        outStream += '#\tdesc - A description of the variable, used to populate C header file comments and GUI. \n'
        outStream += '#\tvalue - The variable value flashed to the microcontroller. \n'
        outStream += '#\tdataType - A valid C type for the variable, options are:\n'
        outStream += '#\t\tuint8_t - Unsigned 8-bit integer. \n'
        outStream += '#\t\tint8_t - Signed 8-bit integer. \n'
        outStream += '#\t\tuint16_t - Unsigned 16-bit integer. \n'
        outStream += '#\t\tint16_t - Signed 16-bit integer. \n'
        outStream += '#\t\tuint32_t - Unsigned 32-bit integer. \n'
        outStream += '#\t\tint32_t - Signed 32-bit integer. \n'
        outStream += '#\t\tfloat -  Floating point, up to four decimal point percision. \n'
        outStream += '#\t\tchar - An ASCII character - valid from ASCII 32 to ASCII 127. \n'
        outStream += "#\tmax - The maximum allowed value, should be less the variable type's maximum. \n"
        outStream += "#\tmin - The minimum allowed value, should be less the variable type's minimum. \n"
        outStream += "\n# A single variable is demonstrated as: \n"

        dataList = [{
                
            'name': 'DELAY',
            'desc': 'Millisecond delay between LED toggles',
            'value': 500,
            'dataType': 'uint16_t',
            'max': 2000,
            'min': 1,
            }]


        self.generateDefinition(filename,dataList,outStream)


    def checkKeys(self,data):

        try:
            temp = data['name']
        except:
            logging.warning('No "name" parameter found in passed variable file, try generating example variable file with command line options.')
            return False

        try:
            temp = data['dataType']
        except:
            logging.warning('Key value "{}" not found for variable {}, try generating example variable file with command line options.'.format('dataType',data['name']))
            return False

        try:
            temp = data['min']
        except:
            logging.warning('Key value "{}" not found for variable {}, try generating example variable file with command line options.'.format('min',data['name']))
            return False

        try:
            temp = data['max']
        except:
            logging.warning('Key value "{}" not found for variable {}, try generating example variable file with command line options.'.format('max',data['name']))
            return False

        try:
            temp = data['value']
        except:
            logging.warning('Key value "{}" not found for variable {}, try generating example variable file with command line options.'.format('value',data['name']))
            return False

        try:
            temp = data['desc']
        except:
            logging.warning('Key value "{}" not found for variable {}, try generating example variable file with command line options.'.format('desc',data['name']))
            return False

        return True

    def generateDefinition(self,filename,dataList,prepend = ''):

        logging.info('Generating definition file')

        savedDataFormat = {
                'name':None,
                'value':None,
                'desc':None,
                'min':None,
                'max':None,
                }
        savedData = []

        for data in dataList:
            savedDataFormat['name'] = data['name']
            savedDataFormat['value'] = data['value']
            savedDataFormat['dataType'] = data['dataType']
            savedDataFormat['desc'] = data['desc']
            savedDataFormat['min'] = data['min']
            savedDataFormat['max'] = data['max']
            savedData.append(savedDataFormat.copy())


        if type(filename) != str:
            logging.warning('Filename for definition file must be of type "string", got type {}'.format(type(filename)))
            return False

        try:
            outFile = open(filename,'w')
            outFile.write(prepend)
            outFile.close()
            outFile = open(filename,'a+')
        except OSError:
            logging.warning('Cannot write definition file "{}"'.format(filename))
            return False


        for data in savedData:
            yaml.dump([data],outFile,default_flow_style=False)
            outFile.write('\n')

        return True

    def generateHeader(self,filename,dataList):
        
        if type(filename) != str:
            logging.warning('Filename for C header file must be of type "string", got type {}'.format(type(filename)))
            return False

        outStream = ''
        currentMemoryPosition = 0

        outStream = outStream + '#ifndef UCCONFIG_GEN_H\n'
        outStream = outStream + '#define UCCONFIG_GEN_H\n'
        outStream = outStream + '/*!\n'
        outStream = outStream + '\t@file ' + filename +  '\n'
        outStream = outStream + '\t@brief ' + 'UcConfig V{} automatically generated C header file.'.format(__version__) +  '\n'
        outStream = outStream + '\t@details Generated: ' + datetime.datetime.strftime(datetime.datetime.now(),'%c') + '\n'
        outStream = outStream + '\tInclude this file in embedded programs code' + '\n'
        outStream = outStream + '*/\n'

        if len(dataList) < 1:
            logging.warning('Generating an C header file with no variables.')

        for data in dataList:

            if not self.checkVariableName(data['name']):
                logging.warning('Invalid variable names, header file not created')
                return False
            outStream = outStream + '/*!\n'
            outStream = outStream + '\t@brief ' + data['desc'] +  '\n'
            outStream = outStream + '\t@details The variable has the following parameters:\n'
            outStream = outStream + '\t - Minimum Value: ' + str(data['min']) +  '\n'
            outStream = outStream + '\t - Maximum Value: ' + str(data['max']) +  '\n'
            outStream = outStream + '\t - Flashed Value: ' + str(round(data['value'],4)) +  '\n'
            outStream = outStream + '\t - Variable Type: ' + data['dataType'] +  '\n'
            outStream = outStream + '\tThe hexidecimal number is the variables location in non-volatile memory.\n'
            outStream = outStream + '*/\n'
            outStream = outStream + '#define ' + data['name'] + ' ' + ' ' + hex(currentMemoryPosition) + '\n'
            currentMemoryPosition = currentMemoryPosition + data['size']

        outStream = outStream + '\n#endif'

        logging.info('Saving C header file {}'.format(filename))

        try:
            outFile = open(filename,'w')
            outFile.write(outStream)
        except OSError:
            logging.warning('Could not write output C header file {}'.format(filename))
            logging.warning('C header file not created')
            return False

        return True

    def checkVariableName(self,name):
        
        if ' ' in name:
            logging.warning('Variables name "{}" cannot conatin spaces.'.format(name))
            return False

        return True
