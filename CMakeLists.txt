project("tloimageeditor")
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)
enable_testing()

# - Qt5Widgets required
# - see http://doc.qt.io/qt-5/cmake-manual.html
# - set CMAKE_PREFIX_PATH to qt5 install directory before/when running cmake
#   - eg: CMAKE_PREFIX_PATH=/path/to/Qt/5.7/gcc_64 cmake <arguments>
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(tlo)
add_executable(tloimageeditor tloimageeditor.cpp)
target_link_libraries(tloimageeditor tloimageeditorcore)