# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

add_definitions(-DTRANSLATION_DOMAIN=\"kinfocenter\")
add_library(KInfoCenterInternal SHARED CommandOutputContext.cpp CommandOutputContext.h)
target_link_libraries(KInfoCenterInternal Qt::Qml KF6::I18n KF6::CoreAddons)
# Disable legacy stuff to get rid of some deprecation warnings. Notably duplicated QProcess::finished overloads.
target_compile_definitions(KInfoCenterInternal PRIVATE -DQT_DISABLE_DEPRECATED_BEFORE=0x050e00)
target_include_directories(KInfoCenterInternal INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

# NB: this is a private library that may not be linked to. Symbol visibility is of no concern. Neither is versioning.
set_target_properties(KInfoCenterInternal
    PROPERTIES
        C_VISIBILITY_PRESET default
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF)

install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kinfocenter/private)
install(TARGETS KInfoCenterInternal ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
