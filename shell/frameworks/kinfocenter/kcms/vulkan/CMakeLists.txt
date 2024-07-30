# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

find_package(vulkaninfo)
set_package_properties(vulkaninfo PROPERTIES TYPE RUNTIME)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_vulkan\")

kinfocenter_add_kcm(kcm_vulkan main.cpp)
target_link_libraries(kcm_vulkan PRIVATE KF6::CoreAddons KF6::KCMUtilsQuick KF6::I18n KInfoCenterInternal)
