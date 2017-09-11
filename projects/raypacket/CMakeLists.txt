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
    DeployRepo(${COPYLIST} ${EXECUTABLE_OUTPUT_PATH})
  else()
    DeployRepo("${COPYLIST}" "${CMAKE_BINARY_DIR}")
  endif()
  #
  # include sources
  file(GLOB MATH_CXX  "math/*.cpp")
  file(GLOB MATH_HXX  "math/*.h")
  file(GLOB SCENE_CXX "scene/*.cpp")
  file(GLOB SCENE_HXX "scene/*.h")
  file(GLOB CXX       "*.cpp")
  file(GLOB HXX       "*.h")
  file(GLOB MAIN      "main.cpp")
  foreach(f ${MAIN})
    list(REMOVE_ITEM CXX ${f})
  endforeach()
  include_directories(${CMAKE_CURRENT_LIST_DIR})
  #
  # add library
  # -- math
  add_library(math_${PID} ${MATH_CXX} ${MATH_HXX})
  target_link_libraries(math_${PID} ${COMMON_LIBS})
  # -- scene
  add_library(scene_${PID} ${SCENE_CXX} ${SCENE_HXX})
  target_link_libraries(scene_${PID} math_${PID} ${COMMON_LIBS})
  # -- common
  add_library(common_${PID} ${CXX} ${HXX})
  target_link_libraries(common_${PID} scene_${PID} math_${PID} ${COMMON_LIBS})
  #
  # add executable
  add_executable(raytracer_${PID} ${MAIN})
  target_link_libraries(raytracer_${PID} 
    common_${PID} 
    scene_${PID} 
    math_${PID} 
    ${COMMON_LIBS})
endif(ENABLE_PACKET_PROJECT)
