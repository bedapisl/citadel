#citadel
RTS game written in C++


#Compilation:
##Debian
Best version of Debian for this compilation is Jessie or Sid, because you will need g++ 4.9 or newer, which is not easy to get on Debian Wheezy.
###Dependencies
####g++
Recommended version: 4.9 or higher

Version 4.7 or lower can't compile this game.

To get it run command: apt-get install g++

To check version: g++ --version
####Boost library
Recommended version: 1.55 or higher

To get it run command: apt-get install libboost-all-dev

To check version: dpkg -s libboost-dev | grep 'Version'
####Library Allegro 5.1
[Guidline to install Allegro](https://wiki.allegro.cc/index.php?title=Install_Allegro5_From_Git/Linux/Debian)

Dont forget that you need version 5.1 not 5.0, so instead of "git checkout 5.0" use "git checkout 5.1".

Version 5.0 should compile too, but the game doesn't work well with 5.0.
###Download source
git clone git://github.com/spekoun/citadel

Above command should create directory named citadel with source codes.
###Compile
cd citadel

make












