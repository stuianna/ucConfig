# Introduction

The python API is separated into three different modules, all located in the lib directory, documentation is being developed for their usage.

The command line application is contained in main.py with a shell script wrapper ucConfig-dev pointing to it.

Python and all required packages need to be installed to use the dev version. Using the binaries under 'releases' is the easiest way to use ucConfig.

# Examples

Example usage of the API is to make at a later date.

# Build Instruction

Pyinstaller is used to build generate a single file executable. This needs to be run on each platform which the program requires to run.

Pyinstaller doesn't work with anaconda. To use pyinstaller the addition to the PATH variable in .bashrc can be commented out while the program is built

To use pyinstaller:

1. From the main python directory run:
```
pyi-makespec main.py -n ucConfig_linux --onefile
```
This will generate a file ucConfig.spec.

2. Inside ucConfig_linux.spec change:
```
datas=[],
```
to
```
datas=[('lib/defaultConfig.yml','.'),('tests/tempVariables.yml','tests')],
```
to include the default configuration file and testing temp variables file in the binary.

3. From the main directory run
```
pyinstaller ucConfig_linux.spec
```
to build the package

The binary is now located at dist/ucConfig, a hard link / shortcut can be made to the appropriate location to run from path.
