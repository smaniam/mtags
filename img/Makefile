LIBB64_HOME = ../deps/libb64-1.2

INC = -I../include -I$(LIBB64_HOME)/include
LIBPATH = -L$(LIBB64_HOME)/src
##LIBS = -lmhash -lb64 -ltag -lz -ljson
LIBS = -lexiv2 -lz -lexpat -ljson -lmhash

CC = g++
CFLAGS = -Wno-long-long -Wundef -ansi -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -O2 -Wformat-security -Wmissing-format-attribute -Wno-non-virtual-dtor -fno-exceptions -fno-check-new -fno-common -fexceptions 

OBJ = img_main.o imgtagjson.o

imgtags: $(OBJ)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

img_main.o: img_main.cpp imgtagjson.h
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/img_main.Tpo" -c -o $@ $<

imgtagjson.o: imgtagjson.cpp imgtagjson.h
	$(CC) -g $(INC) $(CFLAGS) -MT $@ -MD -MP -MF ".deps/imgtagjson.Tpo" -c -o $@ $<

clean:
	rm -f $(OBJ) .deps/* imgtags;
