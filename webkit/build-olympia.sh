#!/bin/sh

DEBUG="--release"

if [ "$1" = "debug" ]
then
   DEBUG="--debug"
fi

perl ./WebKitTools/Scripts/build-webkit --qt $DEBUG --minimal --qmakearg="-nodepend" \
	--makeargs="$MAKE_FLAGS"
