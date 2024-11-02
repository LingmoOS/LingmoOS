option(DEBUG "debug" ON)
project(lingmosdk-datacollect)
if (DEBUG)
	add_definitions(-DDEBUG)  
endif()

add_library(lingmosdk-datacollect  SHARED libkydatacollect.c)
set_target_properties(lingmosdk-datacollect PROPERTIES VERSION 2.3.0 SOVERSION 1)

find_package(PkgConfig REQUIRED)

pkg_check_modules(DBUS-1 dbus-1)
target_include_directories(${PROJECT_NAME} PRIVATE ${DBUS-1_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${DBUS-1_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DBUS-1_LIBRARIES} libcjson.so)

pkg_check_modules(OPENSSL openssl)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES})


pkg_check_modules(KYLOG lingmosdk-log)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYLOG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYLOG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYLOG_LIBRARIES})

execute_process(COMMAND uname -m
            OUTPUT_VARIABLE ARCH_NAME
            OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(PREFIX "")
set(LIBS_INSTALL_DIR "${PREFIX}/usr/lib/${ARCH_NAME}-linux-gnu")
set(HEADERS_INSTALL_DIR "${PREFIX}/usr/include/lingmosdk/diagnosetest/")

install(TARGETS lingmosdk-datacollect LIBRARY DESTINATION ${LIBS_INSTALL_DIR})
install(FILES libkydatacollect.h DESTINATION ${HEADERS_INSTALL_DIR})
