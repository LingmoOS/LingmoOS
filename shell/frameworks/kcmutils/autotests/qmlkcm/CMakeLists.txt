# SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

kcmutils_add_qml_kcm(kcm_testqml SOURCES timesettings.cpp)

target_link_libraries(kcm_testqml
    Qt6::Core
    KF6::CoreAddons
    KF6::I18n
    KF6KCMUtilsQuick
)
