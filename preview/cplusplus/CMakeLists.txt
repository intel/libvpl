cmake_minimum_required(VERSION 3.10.2)

if(BUILD_DEV)
  add_subdirectory("examples")
  add_subdirectory("test")
endif()
