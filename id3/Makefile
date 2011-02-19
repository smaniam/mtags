LIBB64_HOME = ../deps/libb64-1.2

INC = -I$(LIBB64_HOME)/include 
LIBPATH = -L$(LIBB64_HOME)/src -L$(TGL_HOME)/taglib/.libs
LIBS = -lmhash -lb64 -ltag -lz -ljson

CC = g++
CFLAGS = -Wno-long-long -Wundef -ansi -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -O2 -Wformat-security -Wmissing-format-attribute -Wno-non-virtual-dtor -fno-exceptions -fno-check-new -fno-common -fexceptions 

OBJ = id3_main.o id3tagjson.o

id3tags: $(OBJ)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)
	mv id3tags ../

id3_main.o: id3_main.cpp
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/id3_main.Tpo" -c -o $@ $<

id3tagjson.o: id3tagjson.cpp
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/id3tagjson.Tpo" -c -o $@ $<

clean:
	rm -f $(OBJ) .deps/* id3tags;
