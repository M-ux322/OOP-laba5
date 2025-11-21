cmake_minimum_required(VERSION 3.14)

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(DynamicArray LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dynamic_array_app
    main.cpp
)

target_include_directories(dynamic_array_app PRIVATE .)

if(MSVC)
    target_compile_options(dynamic_array_app PRIVATE /W4)
else()
    target_compile_options(dynamic_array_app PRIVATE -Wall -Wextra -pedantic)
endif()