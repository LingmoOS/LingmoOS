# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
# SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>

configure_file(config-plasma-welcome.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-plasma-welcome.h)

add_executable(plasma-welcome)
ecm_add_qml_module(plasma-welcome URI "org.kde.plasma.welcome" GENERATE_PLUGIN_SOURCE)

target_sources(plasma-welcome PRIVATE
    main.cpp
    controller.cpp
    controller.h
    module.cpp
    module.h
    applicationInfo.cpp
    applicationInfo.h
    qmlconfig.h
)

ecm_target_qml_sources(plasma-welcome
    SOURCES
    qml/Main.qml
    qml/PageError.qml

    qml/common/GenericPage.qml
    qml/common/ScrollablePage.qml
    qml/common/KCMPage.qml
    qml/common/ApplicationIcon.qml

    qml/footers/Footer.qml
    qml/footers/FooterDefault.qml
    qml/footers/FooterUpdate.qml

    qml/mock/MockActivities.qml
    qml/mock/MockDesktop.qml
    qml/mock/MockKRunner.qml
    qml/mock/MockPanel.qml
    qml/mock/panel/MockAppletBase.qml
    qml/mock/panel/MockKickoffApplet.qml
    qml/mock/panel/MockTaskManager.qml
    qml/mock/panel/MockSystemTrayApplet.qml
    qml/mock/panel/MockSystemTrayIcon.qml
    qml/mock/panel/MockDigitalClockApplet.qml
    qml/mock/panel/MockShowDesktopApplet.qml

    qml/pages/PlasmaUpdate.qml
    qml/pages/plasmaupdate/Update.qml
    qml/pages/plasmaupdate/Beta.qml
    qml/pages/Live.qml
    qml/pages/Welcome.qml
    qml/pages/Network.qml
    qml/pages/network/PlasmaNM.qml
    qml/pages/SimpleByDefault.qml
    qml/pages/PowerfulWhenNeeded.qml
    qml/pages/powerfulwhenneeded/PlasmaFeatureButton.qml
    qml/pages/powerfulwhenneeded/Overview.qml
    qml/pages/powerfulwhenneeded/KRunner.qml
    qml/pages/powerfulwhenneeded/KDEConnect.qml
    qml/pages/powerfulwhenneeded/Activities.qml
    qml/pages/powerfulwhenneeded/Vaults.qml
    qml/pages/powerfulwhenneeded/SystemSettings.qml
    qml/pages/Discover.qml
    qml/pages/Feedback.qml
    qml/pages/Enjoy.qml
    qml/pages/enjoy/Supporters.qml

    RESOURCES
    assets/konqi-kde-hi.png
    assets/konqi-default.png
    assets/krita.png
    assets/endlesssky.png
)

target_compile_definitions(plasma-welcome PRIVATE "DISTRO_CUSTOM_PAGE_FOLDER=\"${DISTRO_CUSTOM_PAGE_FOLDER}\"")
target_compile_definitions(plasma-welcome PRIVATE "DISTRO_CUSTOM_INTRO_FILE=\"${DISTRO_CUSTOM_INTRO_FILE}\"")

target_link_libraries(plasma-welcome PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Qml
    Qt::Quick
    Qt::QuickControls2
    Qt::Svg
    KF6::I18n
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::ConfigWidgets
    KF6::DBusAddons
    KF6::KIOGui
    KF6::JobWidgets
    KF6::KCMUtils
    KF6::Service
    KF6::WindowSystem
)
kconfig_add_kcfg_files(plasma-welcome GENERATE_MOC config.kcfgc)
install(TARGETS plasma-welcome ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_finalize_qml_module(plasma-welcome)

add_subdirectory(kded)
