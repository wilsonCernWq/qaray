option(ENABLE_PACKET_PROJECT "Build Ray Packet Project" OFF)
if(ENABLE_PACKET_PROJECT)
  #
  # get ptoject info
  get_filename_component(PID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  string(TOLOWER ${PID} PID)
  #
  # install dlls or shader
  file(GLOB SHADERS "*.glsl")
  set(COPYLIST ${COMMON_DLLS} ${SHADERS})
  if(EXECUTABLE_OUTPUT_PATH)
    DeployRepo("${COPYLIST}" "${EXECUTABLE_OUTPUT_PATH}")
  else()
    DeployRepo("${COPYLIST}" "${CMAKE_BINARY_DIR}")
  endif()
  #
  # include sources
  file(GLOB_RECURSE MATH_SRC   "${CMAKE_CURRENT_SOURCE_DIR}/math/*")
  #file(GLOB_RECURSE FB_SRC     "${CMAKE_CURRENT_SOURCE_DIR}/fb/*")
  #file(GLOB_RECURSE COMMON_SRC "${CMAKE_CURRENT_SOURCE_DIR}/common/*")
  #file(GLOB_RECURSE SCENE_SRC  "${CMAKE_CURRENT_SOURCE_DIR}/scene/*")
  file(GLOB GLOB_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*")
  file(GLOB MAIN     "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
  foreach(f ${MAIN})
    list(REMOVE_ITEM GLOB_SRC ${f})
  endforeach()
  include_directories(${CMAKE_CURRENT_LIST_DIR})
  #
  #
  set(ALL_LIBS "")
  macro(qw_add_library name)
    set(lib_name ${name}_${PID})
    set(src_list "")
    set(lnk_list "")
    string(TOUPPER ${name}_SRC src_list)
    message(STATUS "[ ${lib_name} ]")
    foreach(f ${${src_list}})
      file(RELATIVE_PATH rf ${CMAKE_CURRENT_SOURCE_DIR} ${f})
      message(STATUS "   * ${rf}")
    endforeach()
    foreach(l ${ARGN})
      set(lnk_list ${lnk_list} ${l}_${PID})
    endforeach()
    message(STATUS "link to ${lnk_list}")
    add_library(${name}_${PID} ${${src_list}})
    target_link_libraries(${name}_${PID} ${lnk_list} ${COMMON_LIBS})
    set(ALL_LIBS ${ALL_LIBS} ${name}_${PID})
  endmacro()
  #
  # add library
  qw_add_library(math)
  #qw_add_library(fb math)
  #qw_add_library(common math)
  #qw_add_library(scene common fb math)
  #qw_add_library(glob scene common fb math)
  qw_add_library(glob math)
  #
  # add executable
  add_executable(raytracer_${PID} ${MAIN})
  target_link_libraries(raytracer_${PID} ${ALL_LIBS} ${COMMON_LIBS})
endif(ENABLE_PACKET_PROJECT)
