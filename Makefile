MHASH_HOME = lib/mhash
APAR_HOME  = lib/AtomicParsley
LIBB64_HOME = lib/libb64
TAGLIB_HOME = lib/taglib
LIBJSON_HOME = lib/libjson

M4A_HOME   = ./m4a
ID3_HOME   = ./id3

MHASH_CFG_OPTS = --disable-shared --disable-md4 --disable-md2 --disable-tiger --disable-haval  --disable-crc32 --disable-adler32 --disable-ripemd --disable-gost --disable-snefru --disable-whirlpool

default:
	@echo;
	@echo "To Build m4atags and id4tags:";
	@echo "	1. Build Libraries: 	make libs";
	@echo "	2. Build Executables: 	make mediatags";
	@echo;

mediatags: m4atags id3tags

m4atags:
	cd m4a; make;

id3tags:
	cd id3; make;

libs: mhash AtomicParsley libb64 libjson taglib

libb64:
	cd $(LIBB64_HOME); make;

mhash:
	cd $(MHASH_HOME); ./configure $(MHASH_CFG_OPTS); make;

AtomicParsley:
	cd $(APAR_HOME); ./build;

taglib:
	cd $(TAGLIB_HOME); ./configure --disable-debug --disable-shared --enable-static; make

libjson:
	cd $(LIBJSON_HOME); make;

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
