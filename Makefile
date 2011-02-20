M4A_HOME   = ./m4a
ID3_HOME   = ./id3
IMG_HOME   = ./img
PDF_HOME   = ./pdf

MHASH_CFG_OPTS = --disable-shared --disable-md4 --disable-md2 --disable-tiger --disable-haval  --disable-crc32 --disable-adler32 --disable-ripemd --disable-gost --disable-snefru --disable-whirlpool

default:
	@echo;
	@echo "	1. Build Dependencies: 	make deps";
	@echo "	2. Build Executables: 	make mediatags";
	@echo;
	@echo "	   Clean Dependencies: 	make clean_deps";
	@echo "	   Clean Executables: 	make clean";
	@echo;

mediatags: m4atags id3tags imgtags pdftags

m4atags:
	make -C m4a;

id3tags:
	make -C id3;

imgtags:
	make -C img

pdftags:
	make -C pdf

deps:
	./build-deps.sh

clean_deps:
	rm deps -rf

clean:
	rm -rf imgtags pdftags m4atags id3tags
	make clean -C $(M4A_HOME)
	make clean -C $(ID3_HOME)
	make clean -C $(IMG_HOME)
	make clean -C $(PDF_HOME)

release: mediatags
	mv m4atags linux-x86-bins/
	mv id3tags linux-x86-bins/
	mv imgtags linux-x86-bins/
	mv pdftags linux-x86-bins/

install: release
	sudo cp -a linux-x86-bins/* /usr/bin/
