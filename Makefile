MHASH_HOME = lib/mhash
APAR_HOME  = lib/AtomicParsley
M4A_HOME   = ./m4a
LIBB64_HOME = lib/libb64

MHASH_CFG_OPTS = --disable-shared --disable-md4 --disable-md2 --disable-tiger --disable-haval  --disable-crc32 --disable-adler32 --disable-ripemd --disable-gost --disable-snefru --disable-whirlpool

default:
	@echo;
	@echo "To Build m4atags"; echo "	1. Build Libraries: 	make libs";
	@echo "	2. Build m4atags: 	make m4atags";
	@echo;
m4atags:
	cd m4a; make;

libs: mhash AtomicParsley libb64

libb64:
	cd $(LIBB64_HOME); make;

mhash:
	cd $(MHASH_HOME); ./configure $(MHASH_CFG_OPTS); make;

AtomicParsley:
	cd $(APAR_HOME); ./build;

clean:
	cd $(MHASH_HOME); make clean;

cleanall:
	cd $(LIBB64_HOME); make clean;
	cd $(MHASH_HOME); make clean;
	cd $(APAR_HOME); rm -f AtomicParsley obj_files/*;
	cd $(M4A_HOME); make clean;
