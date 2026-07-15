cmake_minimum_required(VERSION 3.20)

project(kwin4_effect_shapecorners VERSION 0.6.2 LANGUAGES CXX)

include(cmake/default-vars.cmake)

find_package(ECM 6 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH}
        ${ECM_KDE_MODULE_DIR}
)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core DBus Widgets)
find_package(KF6Config REQUIRED)
find_package(KF6ConfigWidgets REQUIRED)
find_package(KF6KCMUtils REQUIRED)
find_package(KWin REQUIRED COMPONENTS kwin)
find_package(KWinDBusInterface CONFIG REQUIRED)

message(STATUS "Found KWin Version: ${KWin_VERSION}")
include(cmake/info.cmake)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
