EXIV2_HOME = ../lib/exiv2
MHASH_HOME = ../lib/mhash
LIBB64_HOME = ../lib/libb64
LIBJSON_HOME = ../lib/libjson

EXIV2_INC = -I$(EXIV2_HOME)/src
INC = $(EXIV2_INC) -I$(MHASH_HOME)/include -I$(LIBB64_HOME)/include -I$(LIBJSON_HOME)
LIBPATH = -L$(MHASH_HOME)/lib/.libs -L$(LIBB64_HOME)/src -L$(EXIV2_HOME)/src/.libs -L$(LIBJSON_HOME)
##LIBS = -lmhash -lb64 -ltag -lz -ljson
LIBS = -lexiv2 -lz -lexpat -ljson

CC = g++
CFLAGS = -Wno-long-long -Wundef -ansi -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -O2 -Wformat-security -Wmissing-format-attribute -Wno-non-virtual-dtor -fno-exceptions -fno-check-new -fno-common -fexceptions 

OBJ = img_main.o imgtagjson.o

imgtags: $(OBJ)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

img_main.o: img_main.cpp
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/img_main.Tpo" -c -o $@ $<

imgtagjson.o: imgtagjson.cpp
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/imgtagjson.Tpo" -c -o $@ $<

clean:
	rm -f $(OBJ) .deps/* imgtags;
