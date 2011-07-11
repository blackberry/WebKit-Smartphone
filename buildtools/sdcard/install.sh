#!/bin/sh


if [ "x$1" = "x" ]; then
   echo "usage: install.sh destination_path"
   exit 1;
fi

if [ ! -d "$1" ]; then
   echo "Path $1 not found.  Please create it."
   exit 2;
fi

echo "Installing SD Image to $1"

cp t.html "$1"
cp -aR features "$1"
cp -aR acid2 "$1"
cp -aR acid3 "$1"
cp -aR SunSpider "$1"
cp -aR dromaeo "$1"
cp -aR svg "$1"
cp -aR media "$1"

