#!/bin/bash
################################################
#
# A quick and dirty script to convert a source
# image into the format needed by the launcher
# tool:
#
# - No larger than 320x200
# - 8bpp
# - No more than 208 palette entries
# - BMP version 3
#
# Some files that are already < 16 colours or
# less can sometimes get converted into a 4bpp
# BMP which we don't support - the middle step
# of converting to a jpeg first gets around this.
#
# https://github.com/megatron-uk/x86Launcher
#
###############################################

f=`echo "$1" | awk -F. '{print $1}'`
f_tmp=tmp_$f

if [ -s "$1" ]
then
	echo "Converting $1"
	echo "- Converting to $f_tmp.jpg"
	convert "$1" -resize 320x200 -alpha OFF -compress none -type Palette -depth 8 -colors 208 "$f_tmp.jpg"
	echo "- Converting $f_tmp.jpg -> $f.BMP"
	convert "$f_tmp.jpg" -resize 320x200 -alpha OFF -compress none -type Palette -depth 8 -colors 208 BMP3:"$f.BMP"
	rm -v "$f_tmp.jpg"
else
	echo $1 not found
fi
