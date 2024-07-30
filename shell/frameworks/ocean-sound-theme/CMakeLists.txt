# SPDX-FileCopyrightText: 2021 Benjamin Port <benjamin.port@enioka.com>
# SPDX-FileCopyrightText: 2023 Guilherme Marçal Silva <guimarcalsilva@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

project(ocean-sound-theme)
set(PROJECT_VERSION "6.1.80")

include(FeatureSummary)

set(KF6_MIN_VERSION "6.2.0")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)

install(DIRECTORY "ocean" DESTINATION ${KDE_INSTALL_SOUNDDIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
