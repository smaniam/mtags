cmake_minimum_required(VERSION 2.6)
project(id3tags)
include_directories(${id3tags_SOURCE_DIR}/include)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wno-long-long -Wundef -ansi -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -O2 -Wformat-security -Wmissing-format-attribute -Wno-non-virtual-dtor -fno-exceptions -fno-check-new -fno-common -fexceptions")
SET(CMAKE_C_FLAGS_DEBUG "-DDEBUG -g3 -ggdb3")

include_directories(${id3tags_SOURCE_DIR}/../deps/libb64-1.2/include)
link_directories(${id3tags_SOURCE_DIR}/../deps/libb64-1.2/src)
add_executable(id3tags id3_main.cpp  id3tagjson.cpp)
target_link_libraries(id3tags mhash b64 tag z json)

INSTALL(TARGETS id3tags
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
