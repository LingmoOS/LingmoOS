# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
dtk_add_plugin(
    NAME chameleon
    OUTPUT_DIR
        ${PLUGIN_OUTPUT_BASE_DIR}/styles
    INSTALL_DIR
        ${PLUGIN_INSTALL_BASE_DIR}/styles
    SOURCES
        main.cpp
        chameleonstyle.cpp
        dstyleanimation.cpp
    HEADERS
        chameleonstyle.h
        common.h
        dstyleanimation.h
    DEPENDENCIES
        Dtk${VERSION_SUFFIX}::Widget
        Qt${QT_VERSION_MAJOR}::CorePrivate
        Qt${QT_VERSION_MAJOR}::GuiPrivate
        Qt${QT_VERSION_MAJOR}::WidgetsPrivate
)
