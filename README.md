#citadel
RTS game written in C++


#Compilation on Linux
These guide was tested with Debian Jessie and Fedora 21. It should also work with minor changes on other distros. 
##Dependencies
###g++ 4.9 or higher
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
You should change command "git checkout 5.0" to "git checkout 5.1". Also if you want build Allegro statically, you have to use "cmake -DSHARED=off -DCMAKE_INSTALL_PREFIX=/usr .." instead of "cmake _DCMAKE_INSTALL_PREFIX=/usr ..".

Fedora: yum install allegro5 allegro5-devel allegro5-debuginfo allegro5-addon-acodec allegro5-addon-acodec-devel allegro5-addon-audio allegro5-addon-audio-devel allegro5-addon-dialog allegro5-addon-dialog-devel allegro5-addon-image allegro5-addon-image-devel allegro5-addon-physfs allegro5-addon-physfs-devel allegro5-addon-ttf allegro5-addon-ttf-devel

This will install Allegro 5.0. To get Allegro 5.1, you probably have to download sources and compile them manually.

###Download source
git clone git://github.com/spekoun/citadel

Above command should create directory named "citadel" with source codes.
###Compile
cd citadel; make

This should create executable file a.out, which can be run with "./a.out".

If you want executable with Allegro library linked statically, use "make static" instead of make. But this works only if you have static version of Allegro installed.



