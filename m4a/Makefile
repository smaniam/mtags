APAR_HOME = ../lib/AtomicParsley
APAR_OBJ_PATH = $(APAR_HOME)/obj_files

MHASH_HOME = ../lib/mhash

INC = -I$(APAR_HOME) -I$(MHASH_HOME)/include
LIBPATH = -L$(MHASH_HOME)/lib/.libs
LIBS = -lmhash

CC = g++

OBJ = $(APAR_OBJ_PATH)/APar_sha1.o $(APAR_OBJ_PATH)/APar_uuid.o $(APAR_OBJ_PATH)/AP_commons.o $(APAR_OBJ_PATH)/AtomicParsley_genres.o $(APAR_OBJ_PATH)/AP_iconv.o $(APAR_OBJ_PATH)/AP_AtomExtracts.o $(APAR_OBJ_PATH)/AtomicParsley.o

m4atags: m4atags.o $(APAR_OBJ_PATH)/main.o m4a_json.o
	$(CC) -g -O2 -Wall -o $@ m4atags.o m4a_json.o $(OBJ) $(LIBPATH) $(LIBS)

m4a_json.o: m4a_json.cpp
	$(CC) -g -O2 -Wall $(INC) -MT $@ -MD -MP -MF "./m4a_json.Tpo" -c -o $@ $<

m4atags.o: m4atags.cpp
	$(CC) -g -O2 -Wall $(INC) -MT $@ -MD -MP -MF "./$(APAR_OBJ_PATH)/m4atags.Tpo" -c -o $@ $<

clean:
	rm -f m4atags.o m4a_json.o m4a_json.Tpo m4atags.Tpo m4atags;
