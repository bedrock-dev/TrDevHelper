cmake_minimum_required(VERSION 3.19)
project(TrDevHelper)
set(CMAKE_CXX_STANDARD 14)
add_compile_options(/utf-8 /EHsc)
add_executable(trhelper app.cpp SymbolsReader.cpp CLI11.hpp UserFile.cpp md5.cpp tools.cpp)
target_include_directories(trhelper PUBLIC .)
target_link_libraries(trhelper dbghelp)