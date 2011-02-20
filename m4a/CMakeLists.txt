cmake_minimum_required(VERSION 2.6)
project(m4atags)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
SET(CMAKE_C_FLAGS_DEBUG "-DDEBUG -g3 -ggdb3")

SET(APO ${m4atags_SOURCE_DIR}/../deps/atomicparsley/obj_files)
# TODO change for mac automatically
SET(OE .o)

include_directories(${m4atags_SOURCE_DIR}/../deps/libb64-1.2/include)
link_directories(${m4atags_SOURCE_DIR}/../deps/libb64-1.2/src)

include_directories(${m4atags_SOURCE_DIR}/../deps/atomicparsley)

include_directories(${m4atags_SOURCE_DIR}/include ${APO}/)
add_executable(m4atags m4a_json m4atags ${APO}/APar_sha1${OE} ${APO}/APar_uuid${OE} ${APO}/AP_commons${OE} ${APO}/AtomicParsley_genres${OE} ${APO}/AP_iconv${OE} ${APO}/AP_AtomExtracts${OE} ${APO}/AtomicParsley${OE})
target_link_libraries(m4atags mhash b64)

INSTALL(TARGETS m4atags
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
