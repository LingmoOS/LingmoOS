# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

kcoreaddons_add_plugin(krunner_systemsettings SOURCES systemsettingsrunner.cpp systemsettingsrunner.h INSTALL_NAMESPACE kf6/krunner)
target_link_libraries(krunner_systemsettings
    systemsettings_debug
    KF6::CoreAddons
    KF6::KIOGui
    KF6::I18n
    KF6::JobWidgets
    KF6::Runner
    KF6::Service
    KF6::KCMUtils
    Lingmo::Activities
)
