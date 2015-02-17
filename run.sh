#!/bin/sh

if test `uname -m` = "x86_64"
then
	BIT="64"
else
	BIT="32"
fi

export LD_LIBRARY_PATH="lib$BIT:$LD_LIBRARY_PATH"

exec "./citadel$BIT.out" "$@"








