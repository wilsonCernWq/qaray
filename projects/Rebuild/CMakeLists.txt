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
  file(GLOB MAIN "main.cpp")
  file(GLOB CXX  "*.cpp")
  file(GLOB HXX  "*.h")
  foreach(f ${MAIN})
    list(REMOVE_ITEM CXX ${f})
  endforeach()
  #
  # add library
  add_library(common_${PID} ${CXX} ${HXX})
  target_link_libraries(common_${PID} ${COMMON_LIBS})
  #
  # add executable
  add_executable(raytracer_${PID} ${MAIN})
  target_link_libraries(raytracer_${PID} common_${PID} ${COMMON_LIBS})
endif(ENABLE_PACKET_PROJECT)
