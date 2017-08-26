#
# build TinyXML
#

ADD_LIBRARY(TinyXML SHARED
  tinyxml.cpp 
  tinyxmlparser.cpp 
  tinyxmlerror.cpp 
  tinystr.cpp
  )

SET(TinyXML_LIBRARIES TinyXML PARENT_SCOPE)
SET(TinyXML_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../ PARENT_SCOPE)
