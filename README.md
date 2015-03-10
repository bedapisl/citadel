#Citadel
RTS game written in C++

#Getting the game
##Windows
Not supported

##Download binaries on Linux
Download [Linux binaries](https://sourceforge.net/projects/citadel-game/files/), unzip downloaded file and run executable file with name similar to "citadel_0.1_32bit.out".

##Compilation on Linux
This guide was tested with Debian Jessie and Fedora 21. It should also work with minor changes on other recent distros. 

###Dependencies
To download and compile this game you will need:

1. g++ 4.8 or higher

2. git, make

3. boost serialization, boost filesystem, boost iostream, boost system

4. Allegro library version 5

To install all dependecies on Debian or Ubuntu run command:

	sudo apt-get update && apt-get install g++ git make libboost-serialization-dev libboost-system-dev libboost-filesystem-dev libboost-iostreams-dev liballegro-acodec5-dev liballegro-audio5-dev liballegro-dialog5-dev liballegro-image5-dev liballegro-physfs5-dev liballegro-ttf5-dev liballegro5-dev

or if you have already downloaded source:

	sudo
	./install_lib_debian_ubuntu.sh

On Fedora:

	su yum install gcc-c++ git make boost-devel allegro5-devel allegro5-addon-acodec-devel allegro5-addon-audio-devel allegro5-addon-dialog-devel allegro5-addon-image-devel allegro5-addon-physfs-devel allegro5-addon-ttf-devel

or if you have already downloaded source:
	
	su
	./install_lib_fedora.sh

###Download source
	git clone git://github.com/spekoun/citadel

###Compile
	cd citadel; make

###Run
	./a.out

##Known problems
On newly installed Ubuntu installing dependencies may give you error like:

	The following packages have unmet dependencies:

	libcheese-gtk23 : Depends: ...

	...

	Error, pkgProblemResolver::Resolve generated breaks, ...

This can be solved by:

	sudo apt-get remove libcheese-gtk23

And then run again command to install dependencies.

##Troubleshooting
In case of any problems you can contact me on bedapisl@gmail.com.


