# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(URI "org.lingmo.ds")
add_library(ocean-shell-plugin SHARED
    qmlplugin.h
    qmlplugin.cpp
    qmldir
)

target_link_libraries(ocean-shell-plugin
PUBLIC
    ocean-shell-frame
PRIVATE
    Qt${QT_VERSION_MAJOR}::QuickPrivate
)

string(REPLACE "." "/" URI_PATH ${URI})
set_target_properties(ocean-shell-plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/${URI_PATH}"
    OUTPUT_NAME ocean-shell-plugin
)
set(QML_IMPORT_PATH "${PROJECT_BINARY_DIR}/plugins" CACHE STRING "Qt Creator extra qml import paths")
add_custom_command(TARGET ocean-shell-plugin
POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E  copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/qmldir" "${PROJECT_BINARY_DIR}/plugins/${URI_PATH}"    #out-file
)

install(TARGETS ocean-shell-plugin EXPORT OCEANShellTargets DESTINATION "${QML_INSTALL_DIR}/${URI_PATH}")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/qmldir DESTINATION "${QML_INSTALL_DIR}/${URI_PATH}/")
