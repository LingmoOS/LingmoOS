set(PROJ_NAME "cooperation-launcher")

# enable qt build support
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# add common library directory to the include path
include_directories(${CMAKE_SOURCE_DIR}/src/lib/common)

FILE(GLOB CPP_SRC
    "${CMAKE_SOURCE_DIR}/src/common/log.h"
    "${CMAKE_SOURCE_DIR}/src/common/logger.h"
    "${CMAKE_SOURCE_DIR}/src/common/logger.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/commonutils.h"
    "${CMAKE_SOURCE_DIR}/src/common/commonutils.cpp"
    *.h
    *.cpp
)

list(APPEND LINKLIBS cpplogging)
list(APPEND LINKLIBS sessionmanager)

if (WIN32)
    set(qt_required_components Core Network Widgets Sql WinExtras)
else()
    set(qt_required_components Core Network Widgets)
endif()
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${qt_required_components})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${qt_required_components})

add_executable(${PROJ_NAME} ${CPP_SRC})

target_link_libraries(${PROJ_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets
    ${LINKLIBS})
list(APPEND INSTALL_TARGETS ${PROJ_NAME})
list(APPEND INSTALL_TARGETS_PDB ${PROJ_NAME})

install(TARGETS ${PROJ_NAME}
        RUNTIME DESTINATION bin)

# Install *.pdb files
if(MSVC)
  foreach(INSTALL_TARGET_PDB ${INSTALL_TARGETS_PDB})
    install(FILES $<TARGET_PDB_FILE:${INSTALL_TARGET_PDB}> DESTINATION "${PROJECT_SOURCE_DIR}/bin")
  endforeach()
endif()
