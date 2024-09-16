set(PROJ_NAME "dde-cooperation-transfer")

# enable qt build support
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

configure_file(
    "../config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

FILE(GLOB CPP_SRC
    *.h
    *.cpp
    ${CMAKE_SOURCE_DIR}/src/singleton/*.h
    ${CMAKE_SOURCE_DIR}/src/singleton/*.cpp
    ${CMAKE_SOURCE_DIR}/src/base/*.h
    ${CMAKE_SOURCE_DIR}/src/base/*.cpp
)

set(qt_required_components Core Gui Network Widgets)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${qt_required_components})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${qt_required_components})
if (CMAKE_SYSTEM MATCHES "Linux")
    find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJ_NAME}
        MANUAL_FINALIZATION
        ${CPP_SRC}
    )
else()
    add_executable(${PROJ_NAME}
        ${CPP_SRC}
    )
endif()

target_include_directories(${PROJ_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJ_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src/lib/cooperation)

target_link_libraries(${PROJ_NAME}
  PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::Widgets
  cooperation-transfer
)

if (CMAKE_SYSTEM MATCHES "Linux")
    target_link_libraries(${PROJ_NAME}
        PRIVATE
        Dtk${DTK_VERSION_MAJOR}::Widget
    )
endif()

install(TARGETS ${PROJ_NAME} DESTINATION bin)
