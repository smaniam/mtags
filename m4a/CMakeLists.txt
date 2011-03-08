cmake_minimum_required(VERSION 2.6)
project(m4atags)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -fstack-protector-all")
SET(CMAKE_C_FLAGS_DEBUG "-DDEBUG -g3 -ggdb3")

#SET(APO ${m4atags_SOURCE_DIR}/../deps/atomicparsley/obj_files)
SET(APO ${m4atags_SOURCE_DIR}/../deps/wez-atomicparsley-3b67584ebdb2/src)
# TODO change for mac automatically
SET(OE .o)

include_directories(${m4atags_SOURCE_DIR}/../deps/libb64-1.2/include)
link_directories(${m4atags_SOURCE_DIR}/../deps/libb64-1.2/src)

include_directories(${m4atags_SOURCE_DIR}/../deps/wez-atomicparsley-3b67584ebdb2/src)

include_directories(${m4atags_SOURCE_DIR}/include ${APO}/)
#add_executable(m4atags m4a_json m4atags ${APO}/APar_sha1${OE} ${APO}/APar_uuid${OE} ${APO}/AP_commons${OE} ${APO}/AtomicParsley_genres${OE} ${APO}/AP_iconv${OE} ${APO}/AP_AtomExtracts${OE} ${APO}/AtomicParsley${OE})
add_executable(m4atags m4a_json m4atags ${APO}/sha1${OE} ${APO}/arrays${OE} ${APO}/CDtoc${OE} ${APO}/compress${OE} ${APO}/id3v2${OE} ${APO}/iconv${OE} ${APO}/uuid${OE} ${APO}/util${OE} ${APO}/metalist${OE} ${APO}/parsley${OE})
target_link_libraries(m4atags mhash b64 z)

INSTALL(TARGETS m4atags
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
