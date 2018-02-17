cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project("tloimageeditor")
enable_testing()

# - Qt5Widgets required
# - see http://doc.qt.io/qt-5/cmake-manual.html
# - set CMAKE_PREFIX_PATH to qt5 install directory before/when running cmake
#   - eg: CMAKE_PREFIX_PATH=/path/to/Qt/5.7/gcc_64 cmake <arguments>
# - https://cmake.org/cmake/help/v3.10/manual/cmake-qt.7.html
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Widgets REQUIRED)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(flags "-std=c++14 -pedantic -Wall -Wextra -Werror")
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(flags "${flags} -Weverything -Wno-c++98-compat -Wno-global-constructors -Wno-padded -Wno-undefined-reinterpret-cast")
  endif()
  set(debug_flags "-g")
  #set(debug_flags "-g -fsanitize=address -fsanitize=undefined -O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls")
  #set(debug_flags "-g -fsanitize=memory -fsanitize=undefined -O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls")
else()
  set(flags "${CMAKE_CXX_FLAGS}")
  set(debug_flags "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

message(STATUS "tloimageeditor setting flags")
set(CMAKE_CXX_FLAGS "${flags}" CACHE STRING
    "Flags used by the compiler during all build types." FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${debug_flags}" CACHE STRING
    "Flags used by the compiler during debug builds." FORCE)

add_subdirectory(src)
