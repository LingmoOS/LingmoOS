# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.11)

project(dde-application-wizard)

set(CMAKE_CXX_STANDARD 20) # for QCoro. Might switch back to 17 if we can switch to Qt6 and use Qt6's async/await
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # ensure adapter class can include launcheri1compat.h

find_package(Qt5 REQUIRED COMPONENTS Core LinguistTools)
find_package(Dtk REQUIRED COMPONENTS Core Gui)
find_package(QCoro5 COMPONENTS Core REQUIRED)
find_package(AppStreamQt REQUIRED)
find_package(PackageKitQt5 REQUIRED)

set(BIN_NAME dde-application-wizard-daemon-compat)

include(FeatureSummary)
include(GNUInstallDirs)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_subdirectory(systemd)
add_subdirectory(dbus)

set(SOURCE_FILES
    main.cpp
    pkutils.cpp pkutils.h
    dbus/launcher1compat.cpp dbus/launcher1compat.h
)

qt_add_dbus_adaptor(DBUS_ADAPTER_FILES dbus/org.deepin.dde.daemon.Launcher1.xml dbus/launcher1compat.h Launcher1Compat)

set(TRANSLATION_FILES
    translations/dde-application-wizard.ts
    translations/dde-application-wizard_bo.ts
    translations/dde-application-wizard_es.ts
    translations/dde-application-wizard_hu.ts
    translations/dde-application-wizard_it.ts
    translations/dde-application-wizard_ja.ts
    translations/dde-application-wizard_ko.ts
    translations/dde-application-wizard_pl.ts
    translations/dde-application-wizard_pt_BR.ts
    translations/dde-application-wizard_ru.ts
    translations/dde-application-wizard_uk.ts
    translations/dde-application-wizard_zh_CN.ts
    translations/dde-application-wizard_zh_TW.ts
    translations/dde-application-wizard_zh_HK.ts
)

qt_create_translation(TRANSLATED_FILES ${SOURCE_FILES} ${TRANSLATION_FILES})

add_executable(${BIN_NAME})

target_sources(${BIN_NAME}
PRIVATE
    ${SOURCE_FILES}
    ${DBUS_ADAPTER_FILES}
    ${TRANSLATED_FILES}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Dtk::Core
    Dtk::Gui
    QCoro::Core
    PK::packagekitqt5
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES polkit-1/rules.d/org.deepin.dde.application-wizard.rules DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/rules.d)
install(FILES ${TRANSLATED_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-application-wizard/translations)
