cmake_minimum_required(VERSION 3.0)

project(%{APPNAMEID})

set(QT_MIN_VERSION "5.9.0")
set(KF6_MIN_VERSION "5.42.0")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED
    COMPONENTS
        Core
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED
    COMPONENTS
        KIO
)

find_package(PlasmaPotdProvider CONFIG)
set_package_properties(PlasmaPotdProvider PROPERTIES
    DESCRIPTION "Plasma Picture-of-the-day Provider library"
    TYPE REQUIRED
)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
