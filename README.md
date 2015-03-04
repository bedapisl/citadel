#Citadel
RTS game written in C++

#Getting the game
##Windows
Not supported

##Download binaries on Linux
This is the easiest option of trying this game. It should work on recent Linux distributions (with glibc version at least 2.15, tested on Debian Jessie and Fedora 21).

Download [Linux binaries](https://sourceforge.net/projects/citadel-game/files/), unzip downloaded file and run "./run.sh" script.

##Compilation on Linux
This guide was tested with Debian Jessie and Fedora 21. It should also work with minor changes on other recent distros. 

###Dependencies
To download and compile this game you will need:

1. g++ 4.8 or higher

2. git, make

3. boost serialization, boost filesystem, boost iostream, boost system

4. Allegro library version 5

To install all dependecies on Debian run command:

	su apt-get update && apt-get install g++ git make libboost-serialization-dev libboost-system-dev libboost-filesystem-dev libboost-iostreams-dev liballegro-acodec5-dev liballegro-audio5-dev liballegro-dialog5-dev liballegro-image5-dev liballegro-physfs5-dev liballegro-ttf5-dev liballegro5-dev

or 

	su
	./install_lib_debian_ubuntu.sh

On Fedora:

	su yum install gcc-c++ git make boost-devel allegro5-devel allegro5-addon-acodec-devel allegro5-addon-audio-devel allegro5-addon-dialog-devel allegro5-addon-image-devel allegro5-addon-physfs-devel allegro5-addon-ttf-devel

or
	
	su
	./install_lib_fedora.sh

###Download source
	git clone git://github.com/spekoun/citadel

###Compile
	cd citadel; make

###Run
	./a.out

##Troubleshooting
In case of any problems you can contact me on bedapisl@gmail.com.

