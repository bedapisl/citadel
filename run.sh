#!/bin/sh
export LD_LIBRARY_PATH="lib32:$LD_LIBRARY_PATH"

exec "./citadel32.out" "$@"








