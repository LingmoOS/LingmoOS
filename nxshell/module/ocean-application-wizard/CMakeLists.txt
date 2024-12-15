# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.11)

project(ocean-application-wizard)

set(CMAKE_CXX_STANDARD 20) # for QCoro. Might switch back to 17 if we can switch to Qt6 and use Qt6's async/await
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # ensure adapter class can include launcheri1compat.h

find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)
find_package(Dtk6 REQUIRED COMPONENTS Core Gui)
find_package(QCoro6 COMPONENTS Core REQUIRED)
find_package(AppStreamQt REQUIRED)
find_package(PackageKitQt6 REQUIRED)

set(BIN_NAME ocean-application-wizard-daemon-compat)

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

qt_add_dbus_adaptor(DBUS_ADAPTER_FILES dbus/org.lingmo.ocean.daemon.Launcher1.xml dbus/launcher1compat.h Launcher1Compat)

set(TRANSLATION_FILES
    translations/ocean-application-wizard.ts
    translations/ocean-application-wizard_bo.ts
    translations/ocean-application-wizard_es.ts
    translations/ocean-application-wizard_hu.ts
    translations/ocean-application-wizard_it.ts
    translations/ocean-application-wizard_ja.ts
    translations/ocean-application-wizard_ko.ts
    translations/ocean-application-wizard_pl.ts
    translations/ocean-application-wizard_pt_BR.ts
    translations/ocean-application-wizard_ru.ts
    translations/ocean-application-wizard_uk.ts
    translations/ocean-application-wizard_zh_CN.ts
    translations/ocean-application-wizard_zh_TW.ts
    translations/ocean-application-wizard_zh_HK.ts
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
    Dtk6::Core
    Dtk6::Gui
    QCoro::Core
    PK::packagekitqt6
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES polkit-1/rules.d/org.lingmo.ocean.application-wizard.rules DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/rules.d)
install(FILES ${TRANSLATED_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/ocean-application-wizard/translations)
