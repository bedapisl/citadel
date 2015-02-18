#Citadel
RTS game written in C++

#Getting the game
##Windows
Not supported

##Download binaries on Linux
This is the easiest option of trying this game. It should work on recent Linux distributions (with glibc version at least 2.15, tested on Debian Jessie and Fedora 21).

[Linux binaries](https://sourceforge.net/projects/citadel-game/files/)

After download, unzip downloaded file and run "./run.sh" script.

##Compilation on Linux
This guide was tested with Debian Jessie and Fedora 21. It should also work with minor changes on other recent distros. 
###Dependencies
To download and compile this game you will need:

g++ 4.8 or higher

git, make

boost serialization, boost filesystem, boost iostream, boost system

Allegro library version 5

To install all dependecies on Debian run command:
su apt-get update && apt-get install g++ git make boost-serialization-dev boost-system-dev boost-filesystem-dev boost-iostreams-dev liballegro-acodec5-dev liballegro-audio5-dev liballegro-dialog5-dev liballegro-image5-dev liballegro-physfs5-dev liballegro-ttf5-dev liballegro5-dev


On Fedora:
yum install gcc-c++ git make boost-devel allegro5-devel allegro5-addon-acodec-devel allegro5-addon-audio-devel allegro5-addon-dialog-devel allegro5-addon-image-devel allegro5-addon-physfs-devel allegro5-addon-ttf-devel


###Download source
git clone git://github.com/spekoun/citadel

###Compile
cd citadel; make

###Run
./a.out

##Troubleshooting
Incase of any problems you can contact me on bedapisl@gmail.com.


