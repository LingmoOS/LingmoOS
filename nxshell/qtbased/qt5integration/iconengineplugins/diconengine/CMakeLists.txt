# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

dtk_add_plugin(
    NAME
      dicon
    OUTPUT_DIR
      ${PLUGIN_OUTPUT_BASE_DIR}/iconengines
    INSTALL_DIR
      ${PLUGIN_INSTALL_BASE_DIR}/iconengines
    SOURCES
      main.cpp
    HEADERS
    DEPENDENCIES
      Qt${QT_VERSION_MAJOR}::GuiPrivate
      Dtk${VERSION_SUFFIX}::Gui
)
