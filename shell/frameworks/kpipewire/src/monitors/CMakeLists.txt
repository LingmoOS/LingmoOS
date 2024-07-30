# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(KPipeWireMonitorDeclarative URI "org.kde.pipewire.monitor" GENERATE_PLUGIN_SOURCE VERSION 1.0)
target_sources(KPipeWireMonitorDeclarative PRIVATE mediamonitor.cpp)
target_link_libraries(KPipeWireMonitorDeclarative PRIVATE Qt::Quick PkgConfig::PipeWire KPipeWire)
ecm_finalize_qml_module(KPipeWireMonitorDeclarative)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
