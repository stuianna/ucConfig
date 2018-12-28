# Build Instruction

pyinstaller is used to build generate a single file executable. This needs to be run on each platform which the program requires to run.

Pyinstaller doesn't work with anaconda. To use pyinstaller the addition to the PATH variable in .bashrc can be commented out while the program is built

To use pyinstaller:

1. From the main directory run:
```
pyi-makespec main.py -n ucConfig --onefile
```
This will generate a file ucConfig.spec.

2. Inside ucConfig.spec change:
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
pyinstall ucconfig.spec
```
to build the package

The binary is now located at dist/ucConfig, a hard link / shortcut can be made to the appropriate location to run from path.
