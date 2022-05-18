cmake_minimum_required(VERSION 3.10.2)

find_package(Python3 COMPONENTS Interpreter Development)
find_package(pybind11)
if(Python3_Interpreter_FOUND
   AND Python3_Development_FOUND
   AND pybind11_FOUND)
  add_subdirectory("binding")
  if(BUILD_DEV)
    add_subdirectory("examples")
  endif()
else()
  message(
    SEND_ERROR
      "python prerequisites not found: set BUILD_PYTHON_BINDING=OFF to skip building python bindings"
  )
endif()
