#!/bin/sh
export LD_LIBRARY_PATH="lib64:$LD_LIBRARY_PATH"

exec "./citadel64.out" "$@"








