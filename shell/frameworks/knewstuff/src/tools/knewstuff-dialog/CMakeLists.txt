# SPDX-FileCopyrightText: Dan Leinir Turthra Jensen <admin@leinir.dk>
# SPDX-License-Identifier: BSD-2-Clause

set(knewstuff-dialog_SRCS
    main.cpp
    knsrcmodel.cpp
)

qt_add_resources(RESOURCES resources.qrc)

add_executable(knewstuff-dialog6 ${knewstuff-dialog_SRCS} ${RESOURCES})

target_link_libraries(knewstuff-dialog6
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    KF6::ConfigCore
    KF6::I18n
    KF6::NewStuffCore
)

install(TARGETS knewstuff-dialog6 ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.knewstuff-dialog6.desktop DESTINATION ${KDE_INSTALL_APPDIR})
