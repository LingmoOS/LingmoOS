# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Concurrent)
dtk_add_plugin(
    NAME
      dsvgicon
    OUTPUT_DIR
      ${PLUGIN_OUTPUT_BASE_DIR}/iconengines
    INSTALL_DIR
      ${PLUGIN_INSTALL_BASE_DIR}/iconengines
    SOURCES
      qsvgiconengine.cpp
      main.cpp
    HEADERS
      qsvgiconengine.h
    DEPENDENCIES
      Qt${QT_VERSION_MAJOR}::GuiPrivate
      Dtk${VERSION_SUFFIX}::Gui
      Qt${QT_VERSION_MAJOR}::Concurrent
)
