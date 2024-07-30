# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(kcmdesktopfilegenerator kcmmetadatagenerator.cpp)
add_executable(KF6::kcmdesktopfilegenerator ALIAS kcmdesktopfilegenerator )
target_link_libraries(kcmdesktopfilegenerator Qt::Core)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6KCMUtils")
install(TARGETS kcmdesktopfilegenerator EXPORT KF6KCMUtilsToolingTargets DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF})
install(EXPORT KF6KCMUtilsToolingTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6KCMUtilsToolingTargets.cmake NAMESPACE KF6::)
