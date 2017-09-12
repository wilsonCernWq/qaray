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
  file(GLOB         MAIN      "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
  file(GLOB_RECURSE SCENE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/scene/*")
  file(GLOB        COMMON_SRC "${CMAKE_CURRENT_SOURCE_DIR}/common/*")
  file(GLOB          MATH_SRC "${CMAKE_CURRENT_SOURCE_DIR}/math/*")
  file(GLOB          GLOB_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*")
  foreach(f ${MAIN})
    list(REMOVE_ITEM GLOB_SRC ${f})
  endforeach()
  include_directories(${CMAKE_CURRENT_LIST_DIR})
  #
  #
  macro(qw_add_library name list)
    message(STATUS "[ ${name} ]")
    foreach(f ${${list}})
      message(STATUS "-- * ${f}")
    endforeach()
    add_library(${name} ${${list}})
  endmacro()
  #
  # add library
  # -- math
  qw_add_library(math_${PID} MATH_SRC)
  target_link_libraries(math_${PID} ${COMMON_LIBS})
  # -- common
  qw_add_library(common_${PID} COMMON_SRC)
  target_link_libraries(common_${PID} math_${PID} ${COMMON_LIBS})
  # -- scene
  #qw_add_library(scene_${PID} SCENE_SRC)
  #target_link_libraries(scene_${PID} common_${PID} math_${PID} ${COMMON_LIBS})
  # -- common
  qw_add_library(glob_${PID} GLOB_SRC)
  target_link_libraries(glob_${PID} 
    #scene_${PID} 
    common_${PID} 
    math_${PID} 
    ${COMMON_LIBS})
  #
  # add executable
  add_executable(raytracer_${PID} ${MAIN})
  target_link_libraries(raytracer_${PID} 
    common_${PID} 
    #scene_${PID} 
    math_${PID} 
    ${COMMON_LIBS})
endif(ENABLE_PACKET_PROJECT)
