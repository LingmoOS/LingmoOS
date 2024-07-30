# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

kcmutils_add_qml_kcm(kcm_lingmosearch SOURCES kcm.cpp)
target_link_libraries(kcm_lingmosearch PRIVATE
  KF6::CoreAddons
  KF6::KCMUtils
  KF6::KCMUtilsQuick
  KF6::Runner
  KF6::I18n
  Qt::DBus
)
