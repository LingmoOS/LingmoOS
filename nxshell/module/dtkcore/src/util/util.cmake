if(LINUX)
  set(UTILS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/dtimeunitformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dabstractunitformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/ddisksizeformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/ddbussender.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/drecentmanager.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dnotifysender.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dpinyin.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dexportedinterface.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dvtablehook.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dthreadutils.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dtimedloop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dfileservices_linux.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dexportedinterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ddbusinterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ddbusextendedabstractinterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ddbusextendedpendingcallwatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dtextencoding.cpp
  )
else()
  set(UTILS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/dtimeunitformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dabstractunitformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/ddisksizeformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/ddbussender.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/drecentmanager.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dnotifysender.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dpinyin.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dexportedinterface.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dvtablehook.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dthreadutils.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dtimedloop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dfileservices_dummy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dexportedinterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ddbusinterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ddbusextendedabstractinterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ddbusextendedpendingcallwatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dtextencoding.cpp
  )
endif()
file(GLOB UTILS_HEADERS
  ${PROJECT_SOURCE_DIR}/include/util/*.h
)

set(PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/ddbusinterface_p.h
    ${CMAKE_CURRENT_LIST_DIR}/ddbusextendedpendingcallwatcher_p.h)

if(DTK_VERSION_MAJOR)
  list(REMOVE_ITEM UTILS_SOURCES "${CMAKE_CURRENT_LIST_DIR}/dtimedloop.cpp")
  list(REMOVE_ITEM UTILS_HEADERS "${PROJECT_SOURCE_DIR}/include/util/dtimedloop.h") # no longer be used
  list(REMOVE_ITEM UTILS_HEADERS "${PROJECT_SOURCE_DIR}/include/util/dasync.h")
endif()

set(utils_SRC 
  ${UTILS_HEADERS}
  ${PRIVATE_HEADERS}
  ${UTILS_SOURCES}
  ${CMAKE_CURRENT_LIST_DIR}/util.qrc
)
