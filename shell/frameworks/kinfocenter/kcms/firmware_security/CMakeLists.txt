# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

find_package(fwupdmgr)
set_package_properties(fwupdmgr PROPERTIES TYPE RUNTIME)

find_package(aha)
set_package_properties(aha PROPERTIES TYPE RUNTIME)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_firmware_security\")

kinfocenter_add_kcm(kcm_firmware_security main.cpp)
target_link_libraries(kcm_firmware_security PRIVATE KF6::CoreAddons KF6::KCMUtilsQuick KF6::I18n KInfoCenterInternal)

install(FILES fwupdmgr.sh DESTINATION ${KDE_INSTALL_DATADIR}/kinfocenter/firmware_security)
