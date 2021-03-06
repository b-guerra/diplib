# CMakeLists file for DIPlib 3
cmake_minimum_required(VERSION 3.5)
if(POLICY CMP0068)
   cmake_policy(SET CMP0068 NEW)
endif()

project(DIPlib)

# The version number and latest copyright year. Update these values here, they're used all throughout the project.
set(PROJECT_VERSION_MAJOR "3")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "alpha")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
message(STATUS "PROJECT_VERSION = ${PROJECT_VERSION}")
add_definitions(-DDIP_VERSION_STRING="${PROJECT_VERSION}")
add_definitions(-DDIP_COPYRIGHT_YEAR="2017")

# Debug or Release?
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Static or dynamic?
set(DIP_SHARED_LIBRARY ON CACHE BOOL "Build a shared library (off for static library)")
if(DIP_SHARED_LIBRARY)
   set(BUILD_SHARED_LIBS On)
endif()

# Installation path
set(CMAKE_INSTALL_PREFIX "${CMAKE_BUILD_TYPE}" CACHE PATH "Installation directory")
set(DOCUMENTATION_OUTPUT share/doc/DIPlib)
set(DIPIMAGE_INSTALL_PATH share/DIPimage)
set(PYDIP_INSTALL_PATH lib/PyDIP)

# Compiler flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_VISIBILITY_PRESET hidden) # Hiding all functions, to avoid settings mismatch with the MEX files
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang") # also matchs "AppleClang"
   # using Clang C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wsign-conversion")
   #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native") # This is optimal for local usage.
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   # using GNU C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Wsign-conversion -Wno-unknown-pragmas")
   # -Wno-unknown-pragmas: "#pragma omp" causes a warning if OpenMP is not enabled.
   # "enum class DIP_EXPORT" causes a warning in GCC 5.4, fixed in 6.0.
   # "DIP_EXPORT" in forward class declaration sometimes causes a warning in GCC 6.0 and 7.0.
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes")
   #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native") # This is optimal for local usage; to see which flags are enabled: gcc -march=native -Q --help=target
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og") # Does some optimization that doesn't impact debugging.
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
   # using Intel C++
   # TODO: compiler flags for Intel compiler
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
   # using Visual Studio C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
   add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS) # Disable unchecked iterator warnings and unsafe (string manipulation) function warnings
   add_compile_options(/wd4251) # Disable "needs to have dll-interface" warning
endif()
set(CMAKE_INCLUDE_CURRENT_DIR 1)

# RPATH
#set(CMAKE_MACOSX_RPATH 1) # This is the default
#set(CMAKE_SKIP_BUILD_RPATH 0)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 1) # Prevent relinking when installing
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# Extend CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/tools")

# OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND OR OPENMP_FOUND) # OPENMP_FOUND for CMake <= 3.8
   set(DIP_ENABLE_MULTITHREADING ON CACHE BOOL "Enable multithreading support")
endif()

# Do we have __PRETTY_FUNCTION__ ?
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("int main() { char const* name = __PRETTY_FUNCTION__; return 0; }" HAS_PRETTY_FUNCTION)

# Control exceptions
set(DIP_ENABLE_STACK_TRACE ON CACHE BOOL "Stack trace information is added to exceptions (see the documentation for DIP_CATCH)")
set(DIP_ENABLE_ASSERT ON CACHE BOOL "Enable assertions (see the documentation for DIP_ASSERT)")

# Enable testing
# It's possible to disable DocTest using `DOCTEST_CONFIG_DISABLE`, but that would also disable any tests in code
# that uses DIPlib, hence we define a variable here that removes all of DocTest from the DIPlib sources.
set(DIP_ENABLE_DOCTEST ON CACHE BOOL "Turn off to not include doctest.h in the library headers")

# UFT-8 or plain old ASCII?
set(DIP_ENABLE_UNICODE ON CACHE BOOL "Enable UTF-8 encoded strings, if disabled, some text output will look more 'primitive'")

# Force 128-bit PRNG?
set(DIP_ALWAYS_128_PRNG OFF CACHE BOOL "Use the 128-bit PRNG code even if 128-bit integers are not natively supported by your platform")
if(DIP_ALWAYS_128_PRNG)
   set(FORCE_128_INT TRUE)
   set(HAS_128_INT TRUE)
else()
   check_cxx_source_compiles("int main() { int v = __SIZEOF_INT128__; return 0; }" HAS_128_INT)
endif()

# Image file format libraries: libics, libtiff
set(DIP_ENABLE_ICS ON CACHE BOOL "Enable ICS file support")
if(DIP_ENABLE_ICS)
   add_subdirectory(dependencies/libics EXCLUDE_FROM_ALL)
endif()
find_package(TIFF)
if(TIFF_FOUND)
   set(DIP_ENABLE_TIFF ON CACHE BOOL "Enable TIFF file support")
endif()

# Switch to link against the FFTW library for Fourier transforms
find_package(FFTW3)
if(FFTW3_FOUND)
   set(DIP_ENABLE_FFTW OFF CACHE BOOL "Enable linking against the FFTW library (GPL license) for Fourier transforms")
endif()

# Utility for automatically updating targets when a file is added to the source directories
# from: https://stackoverflow.com/a/39971448/7328782, but modified
# Creates a file called ${name} with the list of dependencies in it.
# The file is updated when the list of dependencies changes.
# If the file is updated, cmake will automatically reload.
function(update_deps_file name deps)
   # Normalize the list so it's the same on every machine
   list(REMOVE_DUPLICATES deps)
   foreach(dep IN LISTS deps)
      file(RELATIVE_PATH rel_dep "${CMAKE_CURRENT_SOURCE_DIR}" ${dep})
      list(APPEND rel_deps ${rel_dep})
   endforeach(dep)
   list(SORT rel_deps)
   # Split the list into lines, and add some CMake-valid syntax so it's ignored
   string(REPLACE ";" "\n" new_deps "${rel_deps}")
   set(new_deps "# Automatically generated, don't edit!\nset(${name}_bogus\n${new_deps}\n)\n")
   # Compare with the old file
   set(old_deps "")
   if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake")
      file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake" old_deps)
   endif()
   if(NOT old_deps STREQUAL new_deps)
      file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake" "${new_deps}")
   endif()
   # Include the file so it's tracked as a generation dependency (we don't need the content).
   include("${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake")
endfunction(update_deps_file)


### DIPlib library

file(GLOB DIPLIB_SOURCES src/*/*.h src/*/*.cpp include/*.h include/diplib/*.h include/diplib/library/*.h include/diplib/private/*.h)
update_deps_file("DIPlib_sources" "${DIPLIB_SOURCES}")
add_library(DIP src/support/matrix.cpp ${DIPLIB_SOURCES}) # The matrix.cpp file takes annoyingly long to compile, let's compile it first!
target_include_directories(DIP PUBLIC include)
include(GenerateExportHeader)
generate_export_header(DIP BASE_NAME dip)
# configuration
target_compile_definitions(DIP PRIVATE DIP_DEBUG_VERSION=$<CONFIG:Debug>)
if(DIP_ENABLE_MULTITHREADING)
   target_compile_options(DIP PRIVATE ${OpenMP_CXX_FLAGS})
   if(OpenMP_CXX_LIB_NAMES)
      target_link_libraries(DIP PRIVATE ${OpenMP_CXX_LIB_NAMES}) # OpenMP_CXX_LIBRARIES is empty?!
   else()
      target_link_libraries(DIP PRIVATE ${OpenMP_CXX_FLAGS}) # This is the case for CMake <= 3.8
   endif()
endif()
if(HAS_PRETTY_FUNCTION)
   target_compile_definitions(DIP PUBLIC DIP__HAS_PRETTY_FUNCTION)
endif()
if(DIP_ENABLE_STACK_TRACE)
   target_compile_definitions(DIP PUBLIC DIP__EXCEPTIONS_RECORD_STACK_TRACE)
endif()
if(DIP_ENABLE_ASSERT)
   target_compile_definitions(DIP PUBLIC DIP__ENABLE_ASSERT)
endif()
if(DIP_ENABLE_DOCTEST)
   target_include_directories(DIP PRIVATE dependencies/doctest)
   target_compile_definitions(DIP PRIVATE DIP__ENABLE_DOCTEST DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
                                          DIP__EXAMPLES_DIR="${PROJECT_SOURCE_DIR}/examples")
   if(DIP_SHARED_LIBRARY)
      target_compile_definitions(DIP PRIVATE DIP__DOCTEST_IN_SHARED_LIB)
   endif()
endif()
if(DIP_ENABLE_UNICODE)
   target_compile_definitions(DIP PRIVATE DIP__ENABLE_UNICODE)
endif()
if(FORCE_128_INT)
   target_compile_definitions(DIP PUBLIC DIP__ALWAYS_128_PRNG)
endif()
# Eigen
target_include_directories(DIP PRIVATE dependencies/eigen3)
target_compile_definitions(DIP PRIVATE EIGEN_MPL2_ONLY # This makes sure we only use parts of the Eigen library that use the MPL2 license or more permissive ones.
                                       EIGEN_DONT_PARALLELIZE) # This to prevent Eigen algorithms trying to run in parallel -- we parallelize at a larger scale.
# libics
if(DIP_ENABLE_ICS)
   target_link_libraries(DIP PRIVATE libics_static)
   target_compile_definitions(DIP PRIVATE DIP__HAS_ICS)
endif()
# libtiff
if(DIP_ENABLE_TIFF)
   target_link_libraries(DIP PRIVATE TIFF::TIFF)
   target_compile_definitions(DIP PRIVATE DIP__HAS_TIFF)
   #get_filename_component(TIFF_LIBS_TRUE ${TIFF_LIBRARIES} REALPATH)
   #install(PROGRAMS ${TIFF_LIBS_TRUE} DESTINATION lib)
endif()
# fftw
if(DIP_ENABLE_FFTW)
   target_include_directories(DIP PRIVATE ${FFTW3_INCLUDE_DIRS} dependencies/fftw)
   target_link_libraries(DIP PRIVATE ${FFTW3_LIBRARIES})
   target_compile_definitions(DIP PRIVATE DIP__HAS_FFTW)
endif()
#install
install(TARGETS DIP DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES ${PROJECT_BINARY_DIR}/dip_export.h DESTINATION include)
# To examine symbol table of the shared library on MacOSX: nm -g -p -U libDIP.dylib

# Uncomment next line to see compile times for each source file (make without -j).
#set_target_properties(DIP PROPERTIES RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")


### DIPlib unit tests

if(DIP_ENABLE_DOCTEST)
   add_executable(unit_tests EXCLUDE_FROM_ALL src/library/unit_tests.cpp)
   target_include_directories(unit_tests PRIVATE dependencies/doctest)
   target_link_libraries(unit_tests PRIVATE DIP)
   target_compile_definitions(unit_tests PRIVATE DIP__IMPLEMENT_UNIT_TESTS DIP__ENABLE_DOCTEST DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES)
   if(DIP_SHARED_LIBRARY)
      target_compile_definitions(unit_tests PRIVATE DIP__DOCTEST_IN_SHARED_LIB)
      if(APPLE)
         set_target_properties(unit_tests PROPERTIES INSTALL_RPATH "@loader_path")
      else()
         set_target_properties(unit_tests PROPERTIES INSTALL_RPATH "$ORIGIN")
      endif()
   else()
      include(dependencies/doctest/doctest_force_link_static_lib_in_target.cmake)
      doctest_force_link_static_lib_in_target(unit_tests DIP) # This pulls in all object files from the static DIP library
   endif()
   add_custom_target(check COMMAND ./unit_tests)
   add_dependencies(check unit_tests)
   add_custom_target(check_memory COMMAND valgrind ./unit_tests)
   add_dependencies(check_memory unit_tests)
endif()


### DIPviewer

# OpenGL, FreeGLUT, GLFW
find_package(OpenGL)
if(NOT APPLE)
   find_package(FreeGLUT)
endif()
if(FREEGLUT_FOUND)
   list(APPEND DIP_VIEWER_MANAGERS "FreeGLUT")
endif()
find_package(GLFW)
if(GLFW_FOUND)
   list(APPEND DIP_VIEWER_MANAGERS "GLFW")
endif()
if(OPENGL_FOUND AND (FREEGLUT_FOUND OR GLFW_FOUND))
   set(DIP_BUILD_DIPVIEWER ON CACHE BOOL "Build the viewer module")
endif()
if(WIN32 AND DIP_BUILD_DIPVIEWER)
   # On Windows, GLEW is required for OpenGL extensions
   find_package(GLEW REQUIRED)
endif()

if(DIP_BUILD_DIPVIEWER)
   file(GLOB DIP_VIEWER_SRC viewer/src/*.cpp viewer/src/manager/*.cpp viewer/src/manager/*.h viewer/include/*.h viewer/include/diplib/viewer/*.h)
   update_deps_file("DIPviewer_sources" "${DIP_VIEWER_SRC}")
   add_library(DIPviewer ${DIP_VIEWER_SRC})
   target_include_directories(DIPviewer PUBLIC viewer/include)
   generate_export_header(DIPviewer BASE_NAME dipviewer)
   target_link_libraries(DIPviewer PRIVATE DIP)
   if(FREEGLUT_FOUND)
      target_compile_definitions(DIPviewer PUBLIC DIP__HAS_FREEGLUT)
      target_link_libraries(DIPviewer PRIVATE FREEGLUT::FREEGLUT)
   endif()
   if(GLFW_FOUND)
      target_compile_definitions(DIPviewer PUBLIC DIP__HAS_GLFW)
      target_link_libraries(DIPviewer PRIVATE GLFW::GLFW)
   endif()
   if(WIN32)
      target_link_libraries(DIPviewer PRIVATE GLEW::GLEW)
   endif()
   if(TARGET OpenGL::GL)
      target_link_libraries(DIPviewer PRIVATE OpenGL::GL)
   else() # For older CMakes:
      target_link_libraries(DIPviewer PRIVATE ${OPENGL_LIBRARIES})
      target_include_directories(DIPviewer PUBLIC ${OPENGL_INCLUDE_DIR})
   endif()
   if(NOT WIN32)
      target_link_libraries(DIPviewer PRIVATE pthread)
   endif()
   if(APPLE)
      set_target_properties(DIPviewer PROPERTIES INSTALL_RPATH "@loader_path")
   else()
      set_target_properties(DIPviewer PROPERTIES INSTALL_RPATH "$ORIGIN")
   endif()
   install(TARGETS DIPviewer DESTINATION lib)
   install(DIRECTORY viewer/include/ DESTINATION include)
endif()


### PyDIP module for Python

set(DIP_BUILD_PYDIP ON CACHE BOOL "Build the PyDIP Python module")
if(DIP_BUILD_PYDIP)
   add_subdirectory(dependencies/pybind11 EXCLUDE_FROM_ALL)
   file(GLOB DIP_PYTHON_SRC pydip/*.cpp pydip/*.h)
   file(GLOB DIP_PYTHON_PY pydip/*.py)
   set(PyDIPdeps "${DIP_PYTHON_SRC}")
   list(APPEND PyDIPdeps "${DIP_PYTHON_PY}")
   update_deps_file("PyDIP_sources" "${PyDIPdeps}")
   pybind11_add_module(PyDIP_bin ${DIP_PYTHON_SRC})
   #target_compile_options(PyDIP_bin PRIVATE -Wno-deprecated-declarations)
   target_link_libraries(PyDIP_bin PRIVATE DIP)
   if(APPLE)
      set_target_properties(PyDIP_bin PROPERTIES INSTALL_RPATH "@loader_path/..")
   else()
      set_target_properties(PyDIP_bin PROPERTIES INSTALL_RPATH "$ORIGIN/..")
   endif()
   install(TARGETS PyDIP_bin DESTINATION ${PYDIP_INSTALL_PATH})
   # Install .py files
   install(FILES ${DIP_PYTHON_PY} DESTINATION ${PYDIP_INSTALL_PATH})
   add_custom_target(PyDIP DEPENDS PyDIP_bin)

   if(DIP_BUILD_DIPVIEWER)
      file(GLOB DIP_PYTHON_VIEWER_SRC pydip/viewer/*.cpp pydip/viewer/*.h)
      set(PyDIPviewerdeps "${DIP_PYTHON_VIEWER_SRC}")
      update_deps_file("PyDIPviewer_sources" "${PyDIPviewerdeps}")
      pybind11_add_module(PyDIPviewer ${DIP_PYTHON_VIEWER_SRC})
      #target_compile_options(PyDIPviewer PRIVATE -Wno-deprecated-declarations)
      target_link_libraries(PyDIPviewer PRIVATE DIP DIPviewer)
      if(APPLE)
         set_target_properties(PyDIPviewer PROPERTIES INSTALL_RPATH "@loader_path/..")
      else()
         set_target_properties(PyDIPviewer PROPERTIES INSTALL_RPATH "$ORIGIN/..")
      endif()
      install(TARGETS PyDIPviewer DESTINATION ${PYDIP_INSTALL_PATH})
      add_dependencies(PyDIP PyDIPviewer)
   endif()
endif()


### DIPimage toolbox for MATLAB

set(MATLAB_ADDITIONAL_VERSIONS "R2016a=9.0") # This is necessary because my version of MATLAB is very recent and my version of CMake is not. Ajust as necessary.
find_package(Matlab OPTIONAL_COMPONENTS MX_LIBRARY MAIN_PROGRAM)
if(Matlab_FOUND)
   set(DIP_BUILD_DIPIMAGE ON CACHE BOOL "Build the DIPimage toolbox")
endif()

if(DIP_BUILD_DIPIMAGE)
   # Install M-files
   install(DIRECTORY dipimage/ DESTINATION ${DIPIMAGE_INSTALL_PATH} FILES_MATCHING PATTERN "*.m") # could also do PATTERN "*.cpp" EXCLUDE

   # Compile and install MEX-files in dipimage/@dip_image/private
   file(GLOB DIP_IMAGE_PRIVATE dipimage/@dip_image/private/*.cpp)
   set(DIPIMAGE_DEPS "${DIP_IMAGE_PRIVATE}")
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/../../../../lib/")
   else()
      set(LIB_REL_PATH "$ORIGIN/../../../../lib/") # doesn't do anything on Windows, so this should be OK
   endif()
   set(DIP_IMAGE_PRIVATE_TARGETS "")
   foreach(file ${DIP_IMAGE_PRIVATE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      if(DIP_ENABLE_UNICODE)
         target_compile_definitions(MEX_${target} PRIVATE DIP__ENABLE_UNICODE)
      endif()
      set(DIP_IMAGE_PRIVATE_TARGETS ${DIP_IMAGE_PRIVATE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIP_IMAGE_PRIVATE_TARGETS} LIBRARY DESTINATION ${DIPIMAGE_INSTALL_PATH}/@dip_image/private
                                                RUNTIME DESTINATION ${DIPIMAGE_INSTALL_PATH}/@dip_image/private)
                                                # By specifying LIBRARY and RUNTIME, but not ARCHIVE, we manage to install
                                                # the .so/.dylib (LIBRARY) and the .dll (RUNTIME), but not the .lib (ARCHIVE)
                                                # that comes with the .dll but we don't need.

   # Compile and install MEX-files in dipimage/@dip_image
   file(GLOB DIP_IMAGE dipimage/@dip_image/*.cpp)
   list(APPEND DIPIMAGE_DEPS "${DIP_IMAGE}")
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/../../../lib/")
   else()
      set(LIB_REL_PATH "$ORIGIN/../../../lib/")
   endif()
   set(DIP_IMAGE_TARGETS "")
   foreach(file ${DIP_IMAGE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      if(DIP_ENABLE_UNICODE)
         target_compile_definitions(MEX_${target} PRIVATE DIP__ENABLE_UNICODE)
      endif()
      set(DIP_IMAGE_TARGETS ${DIP_IMAGE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIP_IMAGE_TARGETS} LIBRARY DESTINATION ${DIPIMAGE_INSTALL_PATH}/@dip_image
                                        RUNTIME DESTINATION ${DIPIMAGE_INSTALL_PATH}/@dip_image)

   # Compile and install MEX-files in dipimage/private
   file(GLOB DIPIMAGE_PRIVATE dipimage/private/*.cpp)
   list(APPEND DIPIMAGE_DEPS "${DIPIMAGE_PRIVATE}")
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/../../../lib/")
   else()
      set(LIB_REL_PATH "$ORIGIN/../../../lib/")
   endif()
   set(DIPIMAGE_PRIVATE_TARGETS "")
   foreach(file ${DIPIMAGE_PRIVATE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      if(DIP_ENABLE_UNICODE)
         target_compile_definitions(MEX_${target} PRIVATE DIP__ENABLE_UNICODE)
      endif()
      set(DIPIMAGE_PRIVATE_TARGETS ${DIPIMAGE_PRIVATE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIPIMAGE_PRIVATE_TARGETS} LIBRARY DESTINATION ${DIPIMAGE_INSTALL_PATH}/private
                                               RUNTIME DESTINATION ${DIPIMAGE_INSTALL_PATH}/private)

   # Compile and install MEX-files in dipimage/
   file(GLOB DIPIMAGE dipimage/*.cpp)
   list(APPEND DIPIMAGE_DEPS "${DIPIMAGE}")
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/../../lib/")
   else()
      set(LIB_REL_PATH "$ORIGIN/../../lib/")
   endif()
   set(DIPIMAGE_TARGETS "")
   foreach(file ${DIPIMAGE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      if(DIP_ENABLE_UNICODE)
         target_compile_definitions(MEX_${target} PRIVATE DIP__ENABLE_UNICODE)
      endif()
      set(DIPIMAGE_TARGETS ${DIPIMAGE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIPIMAGE_TARGETS} LIBRARY DESTINATION ${DIPIMAGE_INSTALL_PATH}
                                       RUNTIME DESTINATION ${DIPIMAGE_INSTALL_PATH})

   # Keep a file with all dependencies
   update_deps_file("DIPimage_sources" "${DIPIMAGE_DEPS}")

   # Tell DIPlib library code that it might link against MATLAB (needed to fix a bug with MATLAB integration)
   target_compile_definitions(DIP PRIVATE DIP__DUILDING_DIPIMAGE)

   # Create the Contents.m file
   install(CODE "execute_process(COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nodisplay -r \"makecontents '${CMAKE_INSTALL_PREFIX}/${DIPIMAGE_INSTALL_PATH}'; exit\" WORKING_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/tools\")")

   # DIPimage user manual
   find_program(PANDOC_EXECUTABLE pandoc)
   find_program(PANDOC_CROSSREF_EXECUTABLE pandoc-crossref)
   if(PANDOC_EXECUTABLE AND PANDOC_CROSSREF_EXECUTABLE)
      execute_process(
            COMMAND ${PANDOC_EXECUTABLE} --version
            OUTPUT_VARIABLE pandoc_version
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
      if(pandoc_version MATCHES "^pandoc ([0-9]*)\\.([0-9]*)")
         set(PANDOC_MAJOR "${CMAKE_MATCH_1}")
         set(PANDOC_MINOR "${CMAKE_MATCH_2}")
         message(STATUS "Found pandoc version ${PANDOC_MAJOR}.${PANDOC_MINOR}")
      else()
         message(STATUS "Found pandoc but could not determine version")
      endif()
      unset(pandoc_version)

      if(PANDOC_MAJOR LESS "2")
         # pandoc 1 has a --smart option, which was removed in pandoc 2.
         set(PANDOC_SMART_OPTION "--smart")
      else()
         # pandoc 2 defines +smart extension to markdown instead of --smart.
         set(PANDOC_SMART_OPTION "--from=markdown+smart")
      endif()

      set(DUM_PDF_NAME "${CMAKE_CURRENT_BINARY_DIR}/DIPimageUserManual.pdf")
      add_custom_command(
            OUTPUT ${DUM_PDF_NAME}
            COMMAND ${PANDOC_EXECUTABLE} *.md --output=${DUM_PDF_NAME}
            --template=template.tex ${PANDOC_SMART_OPTION}
            --number-sections --toc --toc-depth=2
            --filter=pandoc-crossref
            --highlight-style=kate
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dipimage/user_manual"
            DEPENDS dipimage/user_manual/*.md dipimage/user_manual/template.tex
      )
      # Code hightlight style alternative: tango -- light background color, but colors are poorly chosen
      add_custom_target(dum ALL DEPENDS ${DUM_PDF_NAME})
      install(FILES ${DUM_PDF_NAME} DESTINATION ${DOCUMENTATION_OUTPUT})
   endif()
endif()


### Examples

file(GLOB DIP_EXAMPLES examples/*.cpp)
set(EXAMPLES_TARGETS "")
set(EXAMPLES_REQUIRE_VIEWER
    display
    fileviewer
    viewer
)
foreach(file ${DIP_EXAMPLES})
   get_filename_component(target ${file} NAME_WE)
   list(FIND EXAMPLES_REQUIRE_VIEWER ${target} RES)
   if((${RES} EQUAL -1) OR DIP_BUILD_DIPVIEWER) # Don't build the examples that need DIPviewer if we haven't built DIPviewer.
      add_executable(${target} EXCLUDE_FROM_ALL ${file})
      target_compile_definitions(${target} PRIVATE DIP__EXAMPLES_DIR="${PROJECT_SOURCE_DIR}/examples")
      target_link_libraries(${target} DIP -lpthread)
      if(NOT (${RES} EQUAL -1))
         target_link_libraries(${target} DIPviewer)
      endif()
      if(DIP_SHARED_LIBRARY)
         if(APPLE)
            set_target_properties(${target} PROPERTIES INSTALL_RPATH "@loader_path")
         else()
            set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN")
         endif()
      endif()
      set(EXAMPLES_TARGETS ${EXAMPLES_TARGETS} ${target})
   endif()
endforeach()

add_custom_target(examples DEPENDS ${EXAMPLES_TARGETS})


### DIPlib documentation

set(DOXYGEN_SKIP_DOT On)
find_package(Doxygen)
if(DOXYGEN_FOUND)
   configure_file(src/documentation/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
   add_custom_target(
         apidoc
         COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}"
         COMMAND "${DOXYGEN_EXECUTABLE}" Doxyfile
         COMMAND ${CMAKE_COMMAND} -E echo "Fixing header names and other things..."
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/fix_doc_includes.sh" "${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}/html"
         WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
   )
endif(DOXYGEN_FOUND)


### Packaging

if(CMAKE_HOST_WIN32)
   # On Windows we use the NSIS generator
   # TODO: This must be tested and cofigured properly
   set(CPACK_GENERATOR "NSIS")
#elseif(CMAKE_HOST_APPLE)
   # On MacOS we use the Bundle generator
   #set(CPACK_GENERATOR "Bundle")
   #CPACK_BUNDLE_NAME
   #CPACK_BUNDLE_ICON
   #CPACK_BUNDLE_PLIST
   #CPACK_BUNDLE_STARTUP_COMMAND # What would the startup command be???
else()
   # On Linux (or any other platform) we use a tar file
   set(CPACK_GENERATOR "TBZ2")
endif()
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_VENDOR "The DIPlib Project")
set(CPACK_PACKAGE_CONTACT "https://github.com/DIPlib/DIPlib")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
string(TIMESTAMP TODAYS_DATE "%Y%m%d" UTC)
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}-${TODAYS_DATE}")
#CPACK_PACKAGE_ICON
include(CPack)
# TODO: The package doesn't contain any of the dependencies: libtiff, libz, OpenMP, C++ stdlib, etc.
#       Including these is highly dependent on the configuration of the system used for compiling.
#       Posible solutions:
#        - `install(PROGRAMS ${TIFF_LIBRARIES} DESTINATION lib)`
#          We'd then need to modify the library names and the search locations using `install_name_tool`,
#          see http://doc.qt.io/qt-4.8/deployment-mac.html#linking-the-application-to-qt-as-frameworks
#        - `set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})`
#          to link against static libraries where possible, but this requires we also link against
#          libjpeg libz, and who knows what else. And it doesn't fix the problems with OpenMP, stdlib, etc.
#        - Include libtiff, libjpeg and libz in the repository, just like libics.
#          (And if we include libjpeg any way, we might as well port the JPEG reader over from the old dipIO.)
#        See also: https://cmake.org/cmake/help/v3.5/manual/cmake-packages.7.html#creating-relocatable-packages


### Status report

message("")
message("--------------------------------------")
message("   DIPlib CONFIGURATION REPORT")
message("")

message(" * C++ compiler: ${CMAKE_CXX_COMPILER_ID}")

if(CMAKE_BUILD_TYPE MATCHES "^Release$")
   message(" * C++ compiler flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
   message(" * Configured for release")
elseif(CMAKE_BUILD_TYPE MATCHES "^Debug$")
   message(" * C++ compiler flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
   message(" * Configured for debugging")
endif()

if(DIP_SHARED_LIBRARY)
   message(" * Building shared library")
else()
   message(" * Building static library")
endif()
message(" * Library installed to: ${CMAKE_INSTALL_PREFIX}/lib")
message(" * Header files installed to: ${CMAKE_INSTALL_PREFIX}/include")

if(DOXYGEN_FOUND)
   message(" * Documentation installed to: ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}")
   message("   => use the 'apidoc' target")
endif()

if(DIP_ENABLE_MULTITHREADING)
   message(" * Using OpenMP for multithreading")
else()
   message(" * Multithreading disabled")
endif()

if(DIP_ENABLE_STACK_TRACE)
   message(" * Stack trace recording enabled")
else()
   message(" * Stack trace recording disabled")
endif()
if(DIP_ENABLE_ASSERT)
   message(" * Asserts enabled")
else()
   message(" * Asserts disabled")
endif()
if(DIP_ENABLE_UNICODE)
   message(" * Unicode support enabled")
else()
   message(" * Unicode support disabled")
endif()
if(HAS_128_INT)
   message(" * Using 128-bit PRNG")
else()
   message(" * Using 64-bit PRNG")
endif()

if(DIP_ENABLE_ICS)
   message(" * ICS file IO enabled")
else()
   message(" * ICS file IO disabled")
endif()
if(DIP_ENABLE_TIFF)
   message(" * TIFF file IO enabled")
else()
   message(" * TIFF file IO disabled")
endif()

if(DIP_ENABLE_FFTW)
   message(" * Using FFT from FFTW3 library (GPL)")
else()
   message(" * Using built-in FFT")
endif()

if(DIP_ENABLE_DOCTEST)
   message(" * Unit test code included")
   message("   => use the 'check' target")
else()
   message(" * Unit test code excluded")
endif()

if(DIP_BUILD_DIPVIEWER)
   message(" * DIPviewer module added to 'all' with window managers: " ${DIP_VIEWER_MANAGERS})
else()
   message(" * DIPviewer module not configured")
endif()

if(DIP_BUILD_PYDIP)
   message(" * PyDIP module added to 'all' target")
   message(" * PyDIP installed to: ${CMAKE_INSTALL_PREFIX}/${PYDIP_INSTALL_PATH}")
else()
   message(" * PyDIP module not configured")
endif()

if(DIP_BUILD_DIPIMAGE)
   message(" * DIPimage toolbox added to 'all' target")
   message(" * DIPimage toolbox installed to: ${CMAKE_INSTALL_PREFIX}/${DIPIMAGE_INSTALL_PATH}")
   if(DUM_PDF_NAME)
      message(" * DIPimage User Manual installed to: ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}")
   else()
      message(" * DIPimage User Manual cannot be created")
   endif()
else()
   message(" * DIPimage toolbox not configured")
endif()

if(DOXYGEN_FOUND)
   message(" * Doxygen found, 'apidoc' target available")
   message(" * Documentation installed to: ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}")
else()
   message(" * Doxygen not found, 'apidoc' target not available")
endif()

if(CMAKE_GENERATOR MATCHES "Unix Makefiles")
   message("")
   message("Next, type 'make; make check; make install'")
endif()

message("")
message("--------------------------------------")
message("")
