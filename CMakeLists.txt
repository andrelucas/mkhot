cmake_minimum_required(VERSION 3.8)

project(mkhot C CXX)

set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.58.0
    COMPONENTS
        filesystem program_options system thread
    REQUIRED
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

set(source
    mkhot.cpp
)

add_executable(mkhot
    ${source}
)

target_include_directories(mkhot
    PRIVATE
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(mkhot
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
)
