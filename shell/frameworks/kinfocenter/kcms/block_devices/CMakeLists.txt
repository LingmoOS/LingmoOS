# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

find_package(lsblk)
set_package_properties(lsblk PROPERTIES TYPE RUNTIME)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_block_devices\")

kinfocenter_add_kcm(kcm_block_devices main.cpp)
target_link_libraries(kcm_block_devices PRIVATE KF6::CoreAddons KF6::KCMUtilsQuick KF6::I18n KInfoCenterInternal)
