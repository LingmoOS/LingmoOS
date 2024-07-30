# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
# SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_kwinsupportinfo\")

kinfocenter_add_kcm(kcm_kwinsupportinfo main.cpp)
target_link_libraries(kcm_kwinsupportinfo PRIVATE KF6::CoreAddons KF6::KCMUtilsQuick KF6::I18n KInfoCenterInternal)
configure_file(kcm_kwinsupportinfo.json.in ${CMAKE_CURRENT_BINARY_DIR}/kcm_kwinsupportinfo.json @ONLY)
