# SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
#
# SPDX-License-Identifier: GPL-2.0-or-later

set(kqml_SRCS
    main.cpp
)

add_executable(kqml ${kqml_SRCS})
target_compile_definitions(kqml PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(kqml
    Qt::Quick
    Qt::Gui
    Qt::Widgets
    KF6::I18n
    KF6::CoreAddons
    Plasma::PlasmaQuick
)

set_target_properties(kqml PROPERTIES WIN32_EXECUTABLE FALSE)

install(TARGETS kqml ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

kdoctools_create_manpage (man-kqml.1.docbook 1 INSTALL_DESTINATION ${KDE_INSTALL_MANDIR})
install(FILES completions/kqml.zsh RENAME _kqml DESTINATION ${KDE_INSTALL_ZSHAUTOCOMPLETEDIR})
