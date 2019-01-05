import yaml
import sys
import logging
import os
import pkgutil

defaultConfigFilename = 'defaultConfig.yml'

if getattr(sys, 'frozen', False):
    # If the application is run as a bundle, the pyInstaller bootloader
    # extends the sys module by a flag frozen=True and sets the app 
    # path into variable _MEIPASS'.
    dir_path = sys._MEIPASS + os.sep

else:
    dir_path = os.path.dirname(os.path.abspath(__file__)) + os.sep


class ConfigParser():

    def __init__(self):

        self.confDict = None
        self.confLoaded = False

        try:
            with open(dir_path + defaultConfigFilename,'r') as ymlfile:
                self.defaultConfig = yaml.load(ymlfile)
        except:
            logging.warning('Cannot open default config file {}'.format(dir_path + defaultConfigFilename))
            return None

        self.default_params = {}

        for conf in self.defaultConfig:
            self.default_params[conf['name']] = conf['default']

        return 

    def loadConfig(self,configFile):
 
        #Try and open the config file
        config = self.loadFileConfig(configFile)

        #Return None if failed
        if config == None:

            #Failed to load, generate one
            if self.generateConfigFile(configFile) == True:
                #Load the generated file
                config = self.loadConfig(configFile)
            else:
                #Nothing worked, load the default values
                config = self.getDefaultConfig()

        if self.checkConfigLimits(config) == False:
            config = self.getDefaultConfig()
            logging.warning('Problem with config file, using default values')

        return config

    #Return a dictionary containing all config parameters
    def loadFileConfig(self,confFile):

        logging.info('Loading configuration file {}.'.format(confFile))

        try:
            with open(confFile,'r') as ymlfile:
                self.confDict = yaml.load(ymlfile)
                self.confLoaded = True
        except:
            logging.warning('Cannot open config file {}'.format(confFile))
            return None

        self.confLoaded = True
        return self.confDict

    def checkConfigLimits(self,configFile):

        try:
            test = self.defaultConfig[0]['name']
        except:
            logging.warning('Cannot obtain any default config values')
            return False

        #Check all keys match up
        for i,key in enumerate(configFile.keys()):
            if key != self.defaultConfig[i]['name']:
                logging.warning('Key values do not match or key missing, try generating new config file with command line options')
            if configFile[key] > self.defaultConfig[i]['max'] and self.defaultConfig[i]['max'] != 'None':
                logging.warning('Config parameter {} too large, maximum is {}'.format(key,self.defaultConfig[i]['max']))
                return False

            if configFile[key] < self.defaultConfig[i]['min'] and self.defaultConfig[i]['min'] != 'None':
                logging.warning('Config parameter {} too small, minimum is {}'.format(key,self.defaultConfig[i]['max']))
                return False

        return True

    #Generate a config file using default parametrs
    def generateConfigFile(self,filePath):

        logging.info('Generating new config file {} with default paramters'.format(filePath))

        if type(filePath) != str:
            logging.warning('Given file path is not of type string')
            logging.warning('New config file not generated')
            return False

        config = self.getDefaultConfig()

        if self.saveConf(filePath,config) == True:
            return True
        return False

    #Return the devault parameters
    def getDefaultConfig(self):
        logging.info('Fetching default config parameters')
        try:
            return self.default_params
        except:
            return None


    #Save a configuration dictionary to a given file
    def saveConf(self,filePath,configDict):

        logging.info('Saving config file {}'.format(filePath))

        if type(configDict) != dict:
            logging.warning('Config dict is not of type dictionary, type = {}'.format(type(configDict)))
            logging.warning('Updated config file not written')
            return False

        if type(filePath) != str:
            logging.warning('Given file path is not of type string')
            logging.warning('Updated config file not written')
            return False

        #Check all keys match up
        for i,key in enumerate(configDict.keys()):
            if key != self.defaultConfig[i]['name']:
                logging.warning('Key values do not match, try generating new config file with command line options')
                return False

        try:
            open(filePath,'w')
        except OSError:
            logging.warning('Could not write config file with path {}'.format(filePath))
            logging.warning('Updated config file not written')
            return False


        for i in range(len(configDict)):

            try:
                outFile = open(filePath,'a+')
                if i != 0:
                    outFile.write('\n')
                else:
                    outFile.write('# Auto generated config file \n\n')

                outFile.write('# ' + self.defaultConfig[i]['desc'] + '\n')
                outFile.write('# Minimum Value: ' + str(self.defaultConfig[i]['min']) + '\n')
                outFile.write('# Maximum Value: ' + str(self.defaultConfig[i]['max']) + '\n')
                outFile.write('# Default Value: ' + str(self.defaultConfig[i]['default']) + '\n')
                outFile.close()
            except OSError:
                logging.warning('Could not write config file with path {}'.format(filePath))
                logging.warning('Updated config file not written')
                return False
            except:
                logging.warning('Error writing keys')
                return False

            try:
                yaml.dump({list(configDict.keys())[i]:list(configDict.values())[i]},open(filePath,'a+'),default_flow_style = False)
            except OSError:
                logging.warning('Could not write config file with path {}'.format(filePath))
                logging.warning('Updated config file not written')
                return False

        return True



