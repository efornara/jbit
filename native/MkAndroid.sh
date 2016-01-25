#! /bin/bash

if [ -z "$JBITANDROID_WORKDIR" ] ; then
	echo "JBITANDROID_WORKDIR not set"
	exit 1
fi

if [ $# -ne 2 ] ; then
	echo "usage: MkAndroid.sh arch elf"
	exit 1
fi

if [ "$1" == "arm" ] ; then
	ID=3
	TOOL=arm-linux-androideabi
elif [ "$1" == "x86" ] ; then
	ID=9
	TOOL=i686-linux-android
elif [ "$1" == "mips" ] ; then
	ID=9
	TOOL=mipsel-linux-android
else
	echo "invalid arch"
	exit 1
fi
ARCH=$1

TOOLCHAIN=$JBITANDROID_WORKDIR/toolchain-$ARCH-$ID
if [ ! -d $TOOLCHAIN ] ; then
	if [ -z "$JBITANDROID_NDK" ] ; then
		echo "JBITANDROID_NDK not set"
		exit 1
	fi
	$JBITANDROID_NDK/build/tools/make-standalone-toolchain.sh \
	--arch=$1 --platform=android-$ID \
	--install-dir="$TOOLCHAIN"
fi

export PATH="$PATH:$TOOLCHAIN/bin"
if [ "$2" == "nopie" ] ; then
	:
elif [ "$2" == "pie" ] ; then
	export CFLAGS=-fPIE
	export LDFLAGS="-fPIE -pie"
elif [ "$2" == "static" ] ; then
	export LDFLAGS="-static"
else
	echo "invalid elf"
	exit 1
fi
ELF=$2

source ../Version.defs

EXE=jbit-android-$ARCH-$ELF.bin
echo "building $EXE..."

$TOOL-g++ \
 -fno-exceptions -fno-rtti -Wall \
 -DJBIT_VERSION=\"${JBIT_VERSION}\" \
 -O2 -fomit-frame-pointer -s \
 $CFLAGS $LDFLAGS \
 main.cpp devimpl.cpp cpu.cpp asm.cpp symdefs.cpp \
 stdout.cpp xv65.cpp \
 -o $EXE
