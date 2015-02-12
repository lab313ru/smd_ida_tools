# smd_ida_tools
Special IDA Pro tools for the Sega Genesis/Megadrive romhackers.

## Supported versions of IDA Pro
Tested work on v5.2, v6.6. Should work on other versions.

## Supported versions of IDA Pro SDK
You have to use v6.6 version to compile this source.

## How to use
Put the "smd_loader.ldw" file into your "<IDA>\loaders" folder. Put the "smd_consts.plw" file into your "<IDA>\plugins" folder. 

## How to build
1) Open solution (*.sln) in your Visual Studio 2013 (you may use even Express Edition, I hope).

2) Change pathes in that places:

2.1) "Project properties -> Configuration Properties -> General -> Output Directory" accordingly your IDA installation ("loaders" or "plugins");

2.2) "Project properties -> Configuration Properties -> General -> Debugging -> Command" accordingly your IDA installation (path to idaq/idag.exe);

2.3) "Project properties -> Configuration Properties -> C/C++ -> General -> Additional Include Directories" accordingly your IDA SDK installation;

2.4) "Project properties -> Configuration Properties -> Linker -> General -> Additional Library Directories" accordingly your IDA SDK installation;

3) Select your Configuration and press Build.
