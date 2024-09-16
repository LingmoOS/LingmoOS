# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

dtk_add_plugin(
    NAME
        dsvg
    OUTPUT_DIR
        ${PLUGIN_OUTPUT_BASE_DIR}/imageformats
    INSTALL_DIR
        ${PLUGIN_INSTALL_BASE_DIR}/imageformats
    SOURCES
        main.cpp
        qsvgiohandler.cpp
    HEADERS
        qsvgiohandler.h
    DEPENDENCIES
        Dtk${VERSION_SUFFIX}::Gui
)
