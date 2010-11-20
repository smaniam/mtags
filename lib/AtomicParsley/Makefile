CC = g++

OBJ = obj_files/APar_sha1.o obj_files/APar_uuid.o obj_files/AP_commons.o obj_files/AtomicParsley_genres.o obj_files/AP_iconv.o obj_files/AP_AtomExtracts.o obj_files/AtomicParsley.o

m4atags: obj_files/m4atags.o obj_files/json_dump.o obj_files/main.o obj_files/m4a_json.o
	$(CC) -g -o $@ obj_files/m4atags.o obj_files/json_dump.o obj_files/m4a_json.o $(OBJ)

obj_files/json_dump.o: json_dump.cpp
	$(CC) -g -O2 -Wall -I../AtomicParsley -MT $@ -MD -MP -MF "./obj_files/json_dump.Tpo" -c -o $@ $<

obj_files/m4a_json.o: m4a_json.cpp
	$(CC) -g -O2 -Wall -I../AtomicParsley -MT $@ -MD -MP -MF "./obj_files/m4a_json.Tpo" -c -o $@ $<

obj_files/m4atags.o: m4atags.cpp
	$(CC) -g -O2 -Wall -I../AtomicParsley -MT $@ -MD -MP -MF "./obj_files/m4atags.Tpo" -c -o $@ $<
