# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

add_library(ksystemstats_plugin_gpu MODULE GpuPlugin.cpp GpuBackend.cpp GpuDevice.cpp AllGpus.cpp)
target_link_libraries(ksystemstats_plugin_gpu Qt::Core KF6::CoreAddons KF6::I18n KSysGuard::SystemStats UDev::UDev)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   target_sources(ksystemstats_plugin_gpu PRIVATE LinuxAmdGpu.cpp LinuxNvidiaGpu.cpp LinuxBackend.cpp NvidiaSmiProcess.cpp)
endif()

if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()

install(TARGETS ksystemstats_plugin_gpu DESTINATION ${KSYSTEMSTATS_PLUGIN_INSTALL_DIR})
