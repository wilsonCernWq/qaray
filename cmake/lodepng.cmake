# ------------------------------------------------
# to include LodePNG library
# ------------------------------------------------
#
# include path
SET(LodePNG_INCLUDE_DIR ${LodePNG_ROOT_PATH})
#
# libraries
FIND_LIBRARY(LodePNG_release_LIBRARIES NAMES lodepng PATHS
  ${LodePNG_ROOT_PATH}/lib/Release${WIN_ARCH})
FIND_LIBRARY(LodePNG_util_release_LIBRARIES NAMES lodepng_util PATHS
  ${LodePNG_ROOT_PATH}/lib/Release${WIN_ARCH})
#
# process results
MARK_AS_ADVANCED(
  LodePNG_release_LIBRARIES
  LodePNG_util_release_LIBRARIES
  LodePNG_INCLUDE_DIR
  LodePNG_LIBRARIES)
#
# combine library pathes
SET(LodePNG_LIBRARIES
  ${LodePNG_release_LIBRARIES} ${LodePNG_util_release_LIBRARIES}
  )  
#
# printing messages
find_package_handle_standard_args(LodePNG
  DEFAULT_MSG
  LodePNG_INCLUDE_DIR
  LodePNG_LIBRARIES
  )

