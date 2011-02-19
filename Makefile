MHASH_HOME   = lib/mhash
APAR_HOME    = lib/AtomicParsley
LIBB64_HOME  = lib/libb64
TAGLIB_HOME  = lib/taglib
LIBJSON_HOME = lib/libjson
EXIV2_HOME   = lib/exiv2

M4A_HOME   = ./m4a
ID3_HOME   = ./id3
IMG_HOME   = ./img
PDF_HOME   = ./pdf

MHASH_CFG_OPTS = --disable-shared --disable-md4 --disable-md2 --disable-tiger --disable-haval  --disable-crc32 --disable-adler32 --disable-ripemd --disable-gost --disable-snefru --disable-whirlpool

default:
	@echo;
	@echo "To Build m4atags, id4tags and imgtags:";
	@echo "	1. Build Libraries: 	make libs";
	@echo "	2. Build Executables: 	make mediatags";
	@echo;

mediatags: m4atags id3tags imgtags pdftags

m4atags:
	make -C m4a;

id3tags:
	make -C id3;

imgtags:
	cd img; make;

pdftags:
	cd pdf; make;

libs: mhash AtomicParsley libb64 libjson taglib exiv2

libb64:
	cd $(LIBB64_HOME); make;

mhash:
	cd $(MHASH_HOME); ./configure $(MHASH_CFG_OPTS); make;

AtomicParsley:
	cd $(APAR_HOME); ./build;

taglib:
	echo "sudo apt-get install libtag1-vanilla libtag1-dev"

libjson:
	cd $(LIBJSON_HOME); make;

exiv2:
	cd $(EXIV2_HOME); ./configure --disable-shared; make;

clean:
	cd $(MHASH_HOME); make clean;

cleanall:
	cd $(LIBB64_HOME); make clean;
	cd $(MHASH_HOME); make clean;
	cd $(APAR_HOME); rm -f AtomicParsley obj_files/*;
	cd $(TAGLIB_HOME); make clean;
	cd $(LIBJSON_HOME); rm -f Objects/* libjson.a;
	cd $(M4A_HOME); make clean;
	cd $(ID3_HOME); make clean;
	cd $(EXIV2_HOME); make clean;
	cd $(IMG_HOME); make clean;
	cd $(PDF_HOME); make clean;

release: mediatags
	mv m4atags linux-x86-bins/
	mv id3tags linux-x86-bins/
	mv imgtags linux-x86-bins/
	mv pdftags linux-x86-bins/

install: release
	sudo cp -a linux-x86-bins/* /usr/bin/
