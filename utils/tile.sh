#!/bin/sh
FILE=$1
montage -geometry +0+0 "$FILE" "$FILE" "$FILE" "$FILE" tiled.png

