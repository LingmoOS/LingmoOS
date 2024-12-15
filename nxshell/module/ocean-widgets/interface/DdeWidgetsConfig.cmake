# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Qt5 COMPONENTS Core Gui Widgets)

include(GNUInstallDirs)

include_directories(/usr/include/ocean-widgets)

set(COMMON_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)
link_libraries(ocean-widgets ${COMMON_LIBS})

function(install_ocean_widgets_plugin plugin)
    install(TARGETS ${plugin} DESTINATION ${CMAKE_INSTALL_LIBDIR}/ocean-widgets/plugins)
endfunction()
