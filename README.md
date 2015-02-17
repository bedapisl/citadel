#Citadel
RTS game written in C++

#Download binary
This is the easiest option of trying this game. It should work on recent Linux distributions (with glibc version at least 2.15, tested on Debian Jessie and Fedora 21).

1) Install git ("apt-get install git", "yum install git", ...).

2) Execute on console: 

2a) git clone git://github.com/spekoun/citadel

2b) cd citadel

2c) ./run.sh

Last step should launch the game. If this is not working for you, please, send me email on bedapisl@gmail.com with description of what went wrong.


#Compilation on Linux
This guide was tested with Debian Jessie and Fedora 21. It should also work with minor changes on other distros. 
##Dependencies
###g++ 4.8 or higher
Installation - Debian: apt-get install g++

Installation - Fedora: yum install gcc-c++

To check version: g++ --version

###Boost library
Recommended version is 1.55 but probably any version higher than 1.32 will be ok.

Installation - Debian: apt-get install libboost-all-dev

Installation - Fedora: yum install boost-devel

###Allegro library
Game can be compiled with Allegro version 5.0, but works better with Allegro 5.1. So try to get the newer version if you can.

[Guide to compile Allegro on Debian.](https://wiki.allegro.cc/index.php?title=Install_Allegro5_From_Git/Linux/Debian) 
In this guide you should change command "git checkout 5.0" to "git checkout 5.1".

Fedora: yum install allegro5 allegro5-devel allegro5-debuginfo allegro5-addon-acodec allegro5-addon-acodec-devel allegro5-addon-audio allegro5-addon-audio-devel allegro5-addon-dialog allegro5-addon-dialog-devel allegro5-addon-image allegro5-addon-image-devel allegro5-addon-physfs allegro5-addon-physfs-devel allegro5-addon-ttf allegro5-addon-ttf-devel

This will install Allegro 5.0. To get Allegro 5.1, you probably have to download sources and compile them manually.

###Download source
git clone git://github.com/spekoun/citadel

Above command should create directory named "citadel" with source codes.
###Compile
cd citadel; make

This should create executable file a.out, which can be run with "./a.out".



