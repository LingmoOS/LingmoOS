# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

# Find the QtWidgets library
find_package(Qt5 ${QT_MINIMUM_VERSION} CONFIG REQUIRED Core)

# Library
add_library(${PROJECT_NAME}_qt5 SHARED
    ${QGIO_PUBLIC_HEADER_FILES}
    ${QGIO_PRIVATE_HEADER_FILES}
    ${QGIO_PRIVATE_CPP_FILES}
)

set_target_properties(
    ${PROJECT_NAME}_qt5 PROPERTIES
    OUTPUT_NAME gio-qt
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(${PROJECT_NAME}_qt5
PRIVATE
    ${GIOMM_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}/private
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    Qt5::Core
)

target_link_libraries(${PROJECT_NAME}_qt5 Qt5::Core ${GIOMM_LIBRARIES})

target_compile_definitions(${PROJECT_NAME}_qt5 PRIVATE
    QT_NO_KEYWORDS
)

install(TARGETS ${PROJECT_NAME}_qt5 DESTINATION ${CMAKE_INSTALL_LIBDIR})

## dev files
configure_file("gio-qt.pc.in" "gio-qt.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gio-qt.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
