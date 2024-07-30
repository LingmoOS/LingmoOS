# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

add_subdirectory(core)

add_subdirectory(qtquick)
add_subdirectory(tools)

add_subdirectory(widgets)
ecm_qt_install_logging_categories(
    EXPORT KNEWSTUFF
    FILE knewstuff.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
