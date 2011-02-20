cmake_minimum_required(VERSION 2.6)
project(pdftags)
include_directories(${pdftags_SOURCE_DIR}/include)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wno-long-long -Wundef -ansi -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -O2 -Wformat-security -Wmissing-format-attribute -Wno-non-virtual-dtor -fno-exceptions -fno-check-new -fno-common -fexceptions")
SET(CMAKE_C_FLAGS_DEBUG "-DDEBUG -g3 -ggdb3")

include_directories(${pdftags_SOURCE_DIR}/../deps/libb64-1.2/include)
link_directories(${pdftags_SOURCE_DIR}/../deps/libb64-1.2/src)
add_executable(pdftags pdf_main.cpp  pdftagjson.cpp)
target_link_libraries(pdftags mhash poppler json)

INSTALL(TARGETS pdftags
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
