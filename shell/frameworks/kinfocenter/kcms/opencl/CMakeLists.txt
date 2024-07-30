# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
# SPDX-FileCopyrightText: 2022 Linus Dierheimer <linus@dierheimer.de>
#
# SPDX-License-Identifier: BSD-3-Clause

find_package(clinfo)
set_package_properties(clinfo PROPERTIES TYPE RUNTIME)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_opencl\")

kinfocenter_add_kcm(kcm_opencl main.cpp)
target_link_libraries(kcm_opencl PRIVATE KF6::CoreAddons KF6::KCMUtilsQuick KF6::I18n KInfoCenterInternal)
