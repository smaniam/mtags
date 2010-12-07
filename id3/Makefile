TGL_HOME = ../lib/taglib
MHASH_HOME = ../lib/mhash
LIBB64_HOME = ../lib/libb64
LIBJSON_HOME = ../lib/libjson

TGL_INC = -I$(TGL_HOME)/taglib/  -I$(TGL_HOME)/taglib/toolkit -I$(TGL_HOME)/taglib/mpeg -I$(TGL_HOME)/taglib/mpeg/id3v2 -I$(TGL_HOME)/taglib/mpeg/id3v1 -I$(TGL_HOME)/taglib/ape
INC = $(TGL_INC) -I$(MHASH_HOME)/include -I$(LIBB64_HOME)/include -I$(LIBJSON_HOME)
LIBPATH = -L$(MHASH_HOME)/lib/.libs -L$(LIBB64_HOME)/src -L$(TGL_HOME)/taglib/.libs -L$(LIBJSON_HOME)
LIBS = -lmhash -lb64 -ltag -lz -ljson

CC = g++
CFLAGS = -Wno-long-long -Wundef -ansi -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -O2 -Wformat-security -Wmissing-format-attribute -Wno-non-virtual-dtor -fno-exceptions -fno-check-new -fno-common -fexceptions 

OBJ = id3_main.o id3tagjson.o

id3tags: $(OBJ)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

id3_main.o: id3_main.cpp
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/id3_main.Tpo" -c -o $@ $<

id3tagjson.o: id3tagjson.cpp
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/id3tagjson.Tpo" -c -o $@ $<

clean:
	rm -f $(OBJ) .deps/* id3tags;
