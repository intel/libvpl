cmake_minimum_required(VERSION 3.10.2)

add_subdirectory("cplusplus")
if(BUILD_PYTHON_BINDING)
  add_subdirectory("python")
endif()
