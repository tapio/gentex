#!/bin/sh
# Run from build dir
montage -tile 4x -geometry +0+0 clouds.tga terrain.tga sky.tga star.tga -quality 95 ../sample.png

