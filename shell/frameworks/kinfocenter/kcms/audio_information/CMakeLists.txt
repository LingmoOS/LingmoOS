# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2024 Shubham Arora <shubhamarora@protonmail.com>

find_package(pipewire)
set_package_properties(pipewire PROPERTIES TYPE RUNTIME)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_audio_information\")

kinfocenter_add_kcm(kcm_audio_information main.cpp)
target_link_libraries(kcm_audio_information PRIVATE KF6::CoreAddons KF6::KCMUtilsQuick KF6::I18n KInfoCenterInternal)