set(OUTER_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/dconfig.cpp
  ${CMAKE_CURRENT_LIST_DIR}/dsgapplication.cpp
  ${CMAKE_CURRENT_LIST_DIR}/dsysinfo.cpp
  ${CMAKE_CURRENT_LIST_DIR}/dlicenseinfo.cpp
  ${CMAKE_CURRENT_LIST_DIR}/dsecurestring.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ddesktopentry.cpp
)

if (NOT DTK_VERSION_MAJOR)
    list(APPEND OUTER_SOURCE ${CMAKE_CURRENT_LIST_DIR}/dtkcore_global.cpp)
endif()

set(OUTER_HEADER
  ${CMAKE_CURRENT_LIST_DIR}/../include/global/dtkcore_global.h
  ${CMAKE_CURRENT_LIST_DIR}/../include/global/dconfig.h
  ${CMAKE_CURRENT_LIST_DIR}/../include/global/dsgapplication.h
  ${CMAKE_CURRENT_LIST_DIR}/../include/global/dsysinfo.h
  ${CMAKE_CURRENT_LIST_DIR}/../include/global/dlicenseinfo.h
  ${CMAKE_CURRENT_LIST_DIR}/../include/global/dsecurestring.h
  ${CMAKE_CURRENT_LIST_DIR}/../include/global/ddesktopentry.h
)

if(LINUX)
  if(DEFINED D_DSG_APP_DATA_FALLBACK)
      add_definitions(-DD_DSG_APP_DATA_FALLBACK="${D_DSG_APP_DATA_FALLBACK}")
  endif()
  list(APPEND OUTER_SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/dconfigfile.cpp
  )
  list(APPEND OUTER_HEADER
    ${CMAKE_CURRENT_LIST_DIR}/../include/global/dconfigfile.h
  )
#   generic dbus interfaces
  if(NOT DEFINED DTK_DISABLE_DBUS_CONFIG)
    include(${CMAKE_CURRENT_LIST_DIR}/dbus/dbus.cmake)
    list(APPEND glob_SRC ${dbus_SRCS})
  else()
    add_definitions(-DD_DISABLE_DBUS_CONFIG)
  endif()
else()
    add_definitions(-DD_DISABLE_DCONFIG)
endif()

list(APPEND glob_SRC
  ${OUTER_HEADER}
  ${OUTER_SOURCE}
)
