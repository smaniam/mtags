POPP_HOME = /usr/include
MHASH_HOME = ../lib/mhash
LIBB64_HOME = ../lib/libb64
LIBJSON_HOME = ../lib/libjson

POPP_INC = -I$(POPP_HOME)/poppler
INC = $(POPP_INC) -I$(MHASH_HOME)/include -I$(LIBB64_HOME)/include -I$(LIBJSON_HOME) -I$(JHEAD_HOME)
LIBPATH = -L$(MHASH_HOME)/lib/.libs -L$(LIBB64_HOME)/src -L$(EXIV2_HOME)/src/.libs -L$(LIBJSON_HOME)
##LIBS = -lmhash -lb64 -ltag -lz -ljson
LIBS = -lpoppler -ljson -lmhash

CC = g++
CFLAGS = -Wno-long-long -Wundef -ansi -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -O2 -Wformat-security -Wmissing-format-attribute -Wno-non-virtual-dtor -fno-exceptions -fno-check-new -fno-common -fexceptions 

OBJ = pdf_main.o pdftagjson.o

pdftags: $(OBJ)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

pdf_main.o: pdf_main.cpp pdftagjson.h
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/pdf_main.Tpo" -c -o $@ $<

pdftagjson.o: pdftagjson.cpp pdftagjson.h
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/imgtagjson.Tpo" -c -o $@ $<

clean:
	rm -f $(OBJ) .deps/* pdftags;
