cmake_minimum_required(VERSION 3.10.2)

project(pyvpl)
set(TARGET pyvpl)

find_package(VPL REQUIRED)
find_package(pybind11 REQUIRED)

set(PYBIND11_CPP_STANDARD -std=c++17)
# With this definition we force the ABI version to be unique and not risk
# crashes on different modules. (workaround for RS5-10582; see also
# https://github.com/pybind/pybind11/issues/2898)
add_definitions(-DPYBIND11_COMPILER_TYPE="_librs_abi")

file(GLOB PYVPL_SRC "*.hpp" "*.cpp")

find_package(Python3 COMPONENTS Interpreter Development)

if(CMAKE_VERSION VERSION_LESS 3.12)
  find_package(PythonInterp REQUIRED)
  find_package(PythonLibs REQUIRED)
  set(PYTHON_INSTALL_DIR
      "${CMAKE_INSTALL_LIBDIR}/python"
      CACHE PATH "Installation directory for Python bindings")
  set(CMAKECONFIG_PY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/pyvpl")
else()
  find_package(Python REQUIRED COMPONENTS Interpreter Development)
  # set(PYTHON_INSTALL_DIR "${Python_SITEARCH}/pyvpl" CACHE PATH "Installation
  # directory for Python bindings")
  set(PYTHON_INSTALL_DIR
      "${CMAKE_INSTALL_LIBDIR}/python"
      CACHE PATH "Installation directory for Python bindings")
  set(CMAKECONFIG_PY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/pyvpl")
endif()

pybind11_add_module(pyvpl ${PYVPL_SRC})
target_link_libraries(pyvpl PRIVATE VPL::dispatcher)
set_property(TARGET pyvpl PROPERTY CXX_STANDARD 17)

if(UNIX)
  find_package(PkgConfig)
  if(PkgConfig_FOUND)
    # note: pkg-config version for libva is *API* version
    pkg_check_modules(PKG_LIBVA libva>=1.2 libva-drm>=1.2)
    if(PKG_LIBVA_FOUND)
      target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
      set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
      set(THREADS_PREFER_PTHREAD_FLAG TRUE)
      find_package(Threads REQUIRED)
      target_link_libraries(${TARGET} PRIVATE ${PKG_LIBVA_LIBRARIES}
                                              ${PKG_THREAD_LIBRARIES})
      target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})
    else()
      message(WARNING "libva not found: building without libVA support")
    endif()
  else()
    message(WARNING "PkgConfig not found: building without libVA support")
  endif()
endif()

target_link_libraries(${TARGET} PRIVATE VPL::dispatcher)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 17)

set_target_properties(${TARGET} PROPERTIES VERSION ${VPL_INTERFACE_VERSION})
set_target_properties(${TARGET} PROPERTIES FOLDER Wrappers/python)

install(
  TARGETS ${TARGET}
  EXPORT vpl_pythonTargets
  RUNTIME DESTINATION ${PYTHON_INSTALL_DIR} COMPONENT runtime
  LIBRARY DESTINATION ${PYTHON_INSTALL_DIR} COMPONENT runtime)
