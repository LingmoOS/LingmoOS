cmake_minimum_required(VERSION 3.16)

project(%{APPNAMELC})

set(REQUIRED_KF_VERSION "6.0.0")
find_package(ECM ${REQUIRED_KF_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(KF6 ${REQUIRED_KF_VERSION} REQUIRED COMPONENTS
    I18n
    TextEditor
)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
