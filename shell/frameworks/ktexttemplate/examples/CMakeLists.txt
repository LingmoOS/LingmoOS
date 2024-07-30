cmake_minimum_required(VERSION 3.5)

project(ktexttemplate_examples)

include(ExternalProject)

if (IS_ABSOLUTE ${CMAKE_PREFIX_PATH})
  set(prefixPath ${CMAKE_PREFIX_PATH})
else()
  set(prefixPath ${CMAKE_BINARY_DIR}/${CMAKE_PREFIX_PATH})
endif()

ExternalProject_Add(books
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/books
  INSTALL_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_PREFIX_PATH=${prefixPath}
)
ExternalProject_Add(codegen
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/codegen
  INSTALL_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_PREFIX_PATH=${prefixPath}
  DEPENDS books
)

ExternalProject_Add(contacts
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/contacts
  INSTALL_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_PREFIX_PATH=${prefixPath}
  DEPENDS codegen
)

if(FALSE) # remove once htmlapps is ported

# TODO port away from Qt5::XmlPatterns & Qt5::WebKit
ExternalProject_Add(htmlapps
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/htmlapps
  INSTALL_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_PREFIX_PATH=${prefixPath}
  DEPENDS contacts
)

endif()
