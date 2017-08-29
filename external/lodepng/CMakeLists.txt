#
# build LodePNG
#

ADD_LIBRARY(lodepng SHARED
   lodepng.cpp
   lodepng.h
   )

ADD_LIBRARY(lodepng_util SHARED
  lodepng_util.cpp
  lodepng_util.h
  )
target_link_libraries(lodepng_util lodepng)

SET(LodePNG_LIBRARIES lodepng lodepng_util PARENT_SCOPE)
SET(LodePNG_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} PARENT_SCOPE)

