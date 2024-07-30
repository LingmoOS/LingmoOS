# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

find_package(wayland-info)
set_package_properties(wayland-info PROPERTIES
    TYPE RUNTIME
    URL "https://gitlab.freedesktop.org/wayland/wayland-utils"
    PURPOSE "Wayland KCM")

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_wayland\")

kinfocenter_add_kcm(kcm_wayland main.cpp)
target_link_libraries(kcm_wayland PRIVATE KF6::CoreAddons KF6::KCMUtilsQuick KF6::I18n KInfoCenterInternal)
