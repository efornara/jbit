#! /bin/sh

if [ ! -f ../../Version.def ] ; then
	echo "please, run from dists/dos."
	exit 1
fi

# version dependent file names
. ../../Version.def
VER=`echo $JBIT_VERSION | sed 's/\.//g'`
ID=jb${VER}dos
IN=../..
DIR=jbit$VER
OUT=tmp/$DIR

# make empty tmp directory
rm -rf tmp
mkdir -p $OUT

# copy jbit.exe and io2sim.exe
( cd $IN/native ; make clean ; make PLATFORM=dos16 TARGET=jbit )
cp $IN/native/jbit.exe $OUT
( cd $IN/native ; make clean ; make PLATFORM=dos16 TARGET=io2sim )
cp $IN/native/io2sim.exe $OUT

# copy support files
cp $IN/tools/vga14.rom $OUT

# make archive
( cd tmp ; rm -f ../$ID.zip ; zip ../$ID.zip -r $DIR )

# cleanup
rm -rf tmp
