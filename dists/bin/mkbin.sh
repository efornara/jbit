#! /bin/sh

if [ ! -f ../../Version.defs ] ; then
	echo "please, run from dists/bin."
	exit 1
fi

# version dependent file names
. ../../Version.defs
ID=jbit-$JBIT_VERSION
IN=../..
OUT=tmp/$ID

# make empty tmp directory
rm -rf tmp
mkdir -p $OUT

# copy template and license file
cp -r template/* $OUT
cp -r $IN/LICENSE.md $OUT/LICENSE.txt

# copy jbit.exe and jbitw.exe
mkdir -p $OUT/win32
( cd $IN/native ; make clean ; make -f MinGW.mk )
cp $IN/native/jbit.exe $OUT/win32
( cd $IN/native ; make clean )

# copy linux/android binaries
mkdir -p $OUT/linux
( cd $IN/native ; for arch in arm x86 mips ; do
./MkAndroid.sh $arch static
done )
for arch in arm x86 mips ; do
cp $IN/native/jbit-android-$arch-static.bin $OUT/linux/jbit$arch.bin
done
( cd $IN/native ; make clean )

# copy jbit.1 and convert and copy jbdoc
mkdir -p $OUT/doc
for i in $IN/doc/jbdoc/*.md ; do
	f=`basename $i .md`
	$IN/doc/proc/jbdoc.py -f dat \
		$IN/doc/jbdoc/$f.md \
		$IN/midp/help/res/$f.dat
	$IN/doc/proc/jbdoc.py -f xhtml1 \
		$IN/doc/jbdoc/$f.md \
		$OUT/doc/$f.htm
done
( cd $IN/tools ; rm -f *.class )
ronn <$IN/native/jbit.1.ronn --html >$OUT/doc/jbit_1.htm

# copy selection of midlets
mkdir -p $OUT/midp
( cd $IN/midp/jbit ; ant all )
( cd $IN/midp/help ; ant )
cp $IN/midp/jbit/bin/JBit1M.* $OUT/midp
cp $IN/midp/jbit/bin/JBit.* $OUT/midp
cp $IN/midp/help/bin/JBDoc.* $OUT/midp
cp $IN/midp/COPYING.LIB $OUT/midp/LGPL21.txt
( cd $IN/midp/help/res ; rm -f *.dat )

# copy selection of samples
mkdir -p $OUT/samples/6502
cp ../../samples/*.asm $OUT/samples
cp ../../samples/js/bytes.asm $OUT/samples/6502
cp ../../samples/6502/*.asm $OUT/samples/6502

# make archives
( cd tmp ; rm -f ../$ID-bin.zip ; zip ../$ID-bin.zip -l -r $ID )
( cd tmp ; tar czf ../$ID-bin.tar.gz $ID )

# cleanup
rm -rf tmp
