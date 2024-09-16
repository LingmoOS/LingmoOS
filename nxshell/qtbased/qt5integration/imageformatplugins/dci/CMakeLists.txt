# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

dtk_add_plugin(
    NAME
        dci
    OUTPUT_DIR
        ${PLUGIN_OUTPUT_BASE_DIR}/imageformats
    INSTALL_DIR
        ${PLUGIN_INSTALL_BASE_DIR}/imageformats
    SOURCES
        main.cpp
        qdciiohandler.cpp
    HEADERS
        qdciiohandler.h
    DEPENDENCIES
        Dtk${VERSION_SUFFIX}::Gui
)
