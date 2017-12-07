set(PROJECT_ID qaray)
set(ALL_LIBS "") # all the libraries we built
include_directories(${CMAKE_CURRENT_LIST_DIR})
macro(qw_add_library name)
  #
  # initialization
  #
  set(lib_name ${name}_${PROJECT_ID}) # library target name
  set(src_list "") # library source file list
  set(lnk_list "") # library linkage list
  #
  # search for source files
  #
  # -- source files should be in the folder called ${name}
  string(TOUPPER ${name}_SRC src_list)
  file(GLOB_RECURSE ${src_list} "${CMAKE_CURRENT_SOURCE_DIR}/${name}/*")
  # -- debug the source files we found
  message(STATUS "[ ${lib_name} ]")
  foreach (f ${${src_list}})
    file(RELATIVE_PATH rf ${CMAKE_CURRENT_SOURCE_DIR} ${f})
    message(STATUS "   * ${rf}")
  endforeach ()
  #
  # search for linked libraries
  #
  # -- linked libraries are placed as extra arguments in the argument list
  foreach (l ${ARGN})
    set(lnk_list ${lnk_list} ${PROJECT_ID}_${l})
  endforeach ()
  # -- debug
  message(STATUS "link to ${lnk_list}")
  #
  # now create the library
  #
  add_library(${PROJECT_ID}_${name} ${${src_list}})
  target_link_libraries(${PROJECT_ID}_${name}
      ${lnk_list}
      ${COMMON_LIBS})
  set_target_properties(${PROJECT_ID}_${name}
      PROPERTIES
      COMPILE_FLAGS "${COMMON_COMPILE_FLAGS}"
      LINK_FLAGS "${COMMON_LINK_FLAGS}"
      CXX_STANDARD 11)
  #
  # register library
  #
  set(ALL_LIBS ${ALL_LIBS} ${PROJECT_ID}_${name})
endmacro()
#
# add library
#
qw_add_library(tasking)
qw_add_library(math)
qw_add_library(fb math)
qw_add_library(core math)
qw_add_library(samplers core math)
qw_add_library(scene core math)
qw_add_library(lights scene core math)
qw_add_library(materials scene core math)
qw_add_library(objects scene core math)
qw_add_library(textures scene core math)
qw_add_library(parser
    lights materials objects textures scene samplers core math)
qw_add_library(renderers
    lights materials objects textures scene samplers core fb math)
#
# search for executable source files
#
file(GLOB CXX "${CMAKE_CURRENT_SOURCE_DIR}/*")
file(GLOB MAIN "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
foreach (f ${MAIN})
  list(REMOVE_ITEM CXX ${f})
endforeach ()
#
# add executable
#
add_executable(${PROJECT_ID} ${MAIN} ${CXX})
target_link_libraries(${PROJECT_ID} ${ALL_LIBS} ${COMMON_LIBS})
set_target_properties(${PROJECT_ID}
    PROPERTIES
    COMPILE_FLAGS "${COMMON_COMPILE_FLAGS}"
    LINK_FLAGS "${COMMON_LINK_FLAGS}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    CXX_STANDARD 11)
#
# add other executables
#
add_executable(photon_vis exe/PhotonMapViz.cpp)
target_link_libraries(photon_vis ${COMMON_LIBS})
set_target_properties(photon_vis
    PROPERTIES
    COMPILE_FLAGS "${COMMON_COMPILE_FLAGS}"
    LINK_FLAGS "${COMMON_LINK_FLAGS}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    CXX_STANDARD 11)
