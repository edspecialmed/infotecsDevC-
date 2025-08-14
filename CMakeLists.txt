cmake_minimum_required(VERSION 3.10)
project(LoggerTestApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build the library as a shared library" OFF)

add_library(logger_lib 
    logger/Logger.cpp
)

target_include_directories(logger_lib PUBLIC logger)

if(WIN32 AND BUILD_SHARED_LIBS)
    target_compile_definitions(logger_lib PRIVATE LOGGER_EXPORTS)
endif()

add_executable(logger_tester main.cpp)


target_link_libraries(logger_tester PRIVATE logger_lib)


find_package(Threads REQUIRED)
target_link_libraries(logger_tester PRIVATE Threads::Threads)