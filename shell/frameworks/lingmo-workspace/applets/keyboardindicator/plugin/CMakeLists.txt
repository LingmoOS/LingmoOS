# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(keyboardindicatorplugin URI org.kde.lingmo.private.keyboardindicator GENERATE_PLUGIN_SOURCE)
target_sources(keyboardindicatorplugin
    PRIVATE
        keystate.cpp
)
target_link_libraries(keyboardindicatorplugin
    PRIVATE
        Qt::Qml
        KF6::GuiAddons
)
ecm_qt_declare_logging_category(keyboardindicatorplugin
    HEADER keyboardindicator_debug.h
    IDENTIFIER KEYBOARDINDICATOR_DEBUG
    CATEGORY_NAME org.kde.lingmo.keyboardindicator
    DESCRIPTION "keyboardindicator"
    EXPORT LINGMOWORKSPACE
)
ecm_finalize_qml_module(keyboardindicatorplugin)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
