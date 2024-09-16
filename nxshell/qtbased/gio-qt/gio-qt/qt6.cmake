# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

# Find the QtWidgets library
find_package(Qt6 REQUIRED COMPONENTS Core)

# Library
add_library(${PROJECT_NAME}_qt6 SHARED
    ${QGIO_PUBLIC_HEADER_FILES}
    ${QGIO_PRIVATE_HEADER_FILES}
    ${QGIO_PRIVATE_CPP_FILES}
)

set_target_properties(
    ${PROJECT_NAME}_qt6 PROPERTIES
    OUTPUT_NAME gio-qt6
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(${PROJECT_NAME}_qt6
PRIVATE
    ${GIOMM_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}/private
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    Qt6::Core
)

target_link_libraries(${PROJECT_NAME}_qt6 Qt6::Core ${GIOMM_LIBRARIES})

target_compile_definitions(${PROJECT_NAME}_qt6 PRIVATE
    QT_NO_KEYWORDS
)

install(TARGETS ${PROJECT_NAME}_qt6 DESTINATION ${CMAKE_INSTALL_LIBDIR})

## dev files
configure_file("gio-qt6.pc.in" "gio-qt6.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gio-qt6.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
