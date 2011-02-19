LIBB64_HOME = ../deps/libb64-1.2

INC = -I$(LIBB64_HOME)/include
LIBPATH = -L$(LIBB64_HOME)/src
##LIBS = -lmhash -lb64 -ltag -lz -ljson
LIBS = -lpoppler -ljson -lmhash

CC = g++
CFLAGS = -Wno-long-long -Wundef -ansi -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -O2 -Wformat-security -Wmissing-format-attribute -Wno-non-virtual-dtor -fno-exceptions -fno-check-new -fno-common -fexceptions 

OBJ = pdf_main.o pdftagjson.o

pdftags: $(OBJ)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)
	mv pdftags ../

pdf_main.o: pdf_main.cpp pdftagjson.h
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/pdf_main.Tpo" -c -o $@ $<

pdftagjson.o: pdftagjson.cpp pdftagjson.h
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/imgtagjson.Tpo" -c -o $@ $<

clean:
	rm -f $(OBJ) .deps/* pdftags;
