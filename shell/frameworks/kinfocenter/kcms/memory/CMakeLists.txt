# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2024 Thomas Duckworth <tduck973564@gmail.com>

find_package(dmidecode)
set_package_properties(dmidecode PROPERTIES TYPE RUNTIME)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_memory\")

kinfocenter_add_kcm(kcm_memory main.cpp)
target_link_libraries(kcm_memory PRIVATE KF6::CoreAddons KF6::KCMUtilsQuick KF6::I18n KInfoCenterInternal)
