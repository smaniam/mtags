APAR_HOME = ../deps/atomicparsley
APAR_OBJ_PATH = $(APAR_HOME)/obj_files
#APAR_HOME = ../deps/atomicparsley/src
#APAR_OBJ_PATH = $(APAR_HOME)

LIBB64_HOME = ../deps/libb64-1.2

INC = -I$(APAR_HOME) -I$(LIBB64_HOME)/include
LIBPATH = -L$(LIBB64_HOME)/src
LIBS = -lmhash -lb64

CC = g++

OBJ = $(APAR_OBJ_PATH)/APar_sha1.o $(APAR_OBJ_PATH)/APar_uuid.o $(APAR_OBJ_PATH)/AP_commons.o $(APAR_OBJ_PATH)/AtomicParsley_genres.o $(APAR_OBJ_PATH)/AP_iconv.o $(APAR_OBJ_PATH)/AP_AtomExtracts.o $(APAR_OBJ_PATH)/AtomicParsley.o

m4atags: m4atags.o $(APAR_OBJ_PATH)/main.o m4a_json.o
	$(CC) -g -O2 -Wall -o $@ m4atags.o m4a_json.o $(OBJ) $(LIBPATH) $(LIBS)
	mv m4atags ../

m4a_json.o: m4a_json.cpp
	$(CC) -g -O2 -Wall $(INC) -MT $@ -MD -MP -MF "./m4a_json.Tpo" -c -o $@ $<

m4atags.o: m4atags.cpp
	$(CC) -g -O2 -Wall $(INC) -MT $@ -MD -MP -MF "./$(APAR_OBJ_PATH)/m4atags.Tpo" -c -o $@ $<

clean:
	rm -f m4atags.o m4a_json.o m4a_json.Tpo m4atags.Tpo m4atags;
