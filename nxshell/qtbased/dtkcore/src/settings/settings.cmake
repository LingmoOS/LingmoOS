if(LINUX)
  file(GLOB SETTINGS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backend/*.cpp
  )
  file(GLOB SETTINGS_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/../../include/settings/*.h 
    ${CMAKE_CURRENT_LIST_DIR}/../../include/settings/backend/*.h 
  )
else()
  file(GLOB SETTINGS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backend/dsettingsdconfigbackend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backend/qsettingbackend.cpp
  )
  file(GLOB SETTINGS_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/../../include/settings/*.h 
    ${CMAKE_CURRENT_LIST_DIR}/../../include/settings/backend/dsettingsdconfigbackend.h 
    ${CMAKE_CURRENT_LIST_DIR}/../../include/settings/backend/qsettingbackend.h
  )
endif()

if(DTK_VERSION_MAJOR)
  list(REMOVE_ITEM SETTINGS_SOURCES "${CMAKE_CURRENT_LIST_DIR}/backend/gsettingsbackend.cpp")
  list(REMOVE_ITEM SETTINGS_HEADERS "${CMAKE_CURRENT_LIST_DIR}/../../include/settings/backend/gsettingsbackend.h")
endif()

set(settings_SRC
  ${SETTINGS_HEADERS}
  ${SETTINGS_SOURCES}
)
