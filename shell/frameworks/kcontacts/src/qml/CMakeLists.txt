# SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(kcontactsqml URI "org.kde.contacts" CLASS_NAME KContactsQmlPlugin DEPENDENCIES QtQuick)

target_sources(kcontactsqml PRIVATE
    kcontactsqmlplugin.cpp
    types.h
)

target_link_libraries(kcontactsqml PRIVATE KF6Contacts Qt6::Qml)

ecm_finalize_qml_module(kcontactsqml)
