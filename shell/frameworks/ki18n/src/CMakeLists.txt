# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(i18n)
add_subdirectory(localedata)
if (TARGET Qt6::Qml)
    add_subdirectory(localedata-qml)
endif()

ecm_qt_install_logging_categories(
    EXPORT KI18N
    FILE ki18n.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
