#! /bin/sh

if [ ! -f ../../Version.def ] ; then
	echo "please, run from dists/android."
	exit 1
fi

# version dependent file names
. ../../Version.def
ID=jbit-$JBIT_VERSION
IN=../..
OUT=tmp/bin

# make empty tmp directory
rm -rf tmp
mkdir -p $OUT

# make binaries (all combinations)
( cd $IN/native ; for arch in arm x86 mips ; do
for elf in pie nopie static ; do
./MkAndroid.sh $arch $elf
done
done )

# copy android binaries
cp $IN/native/jbit-android-*.bin $OUT

# make archives
( cd tmp ; rm -f ../$ID-android-all.zip ; zip ../$ID-android-all.zip -r bin )
( cd tmp ; tar czf ../$ID-android-all.tar.gz bin )

# cleanup
rm -rf tmp
