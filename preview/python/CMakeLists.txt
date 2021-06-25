cmake_minimum_required(VERSION 3.10.2)

if(BUILD_PYTHON_BINDING)
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
    message(WARNING "Python binding wil not be built missing prerequisite.")
  endif()
endif()
