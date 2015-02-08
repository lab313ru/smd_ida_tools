# ida_smd_loader
Sega Genesis/Megadrive ROMs loader plugin for IDA Pro.

## Supported versions of IDA Pro
Tested work on v5.2, v6.6. Should work on other versions.

## Supported versions of IDA Pro SDK
You have to use v6.6 version to compile this source.

## How to use
Put the "smd_loader.ldw" file into your <IDA>\loaders folder.

## How to build
1) Open solution (*.sln) in your Visual Studio 2013 (you may use even Express Edition, I hope).

2) Change pathes in that places:

2.1) "Project properties -> Configuration Properties -> General -> Output Directory" accordingly your IDA installation;

2.2) "Project properties -> Configuration Properties -> General -> Debugging -> Command" accordingly your IDA installation;

2.3) "Project properties -> Configuration Properties -> C/C++ -> General -> Additional Include Directories" accordingly your IDA installation;

2.4) "Project properties -> Configuration Properties -> Linker -> General -> Additional Library Directories" accordingly your IDA installation;

3) Select your Configuration and press Build.
