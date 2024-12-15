cmake_minimum_required(VERSION 3.18)
set(pluginName lingmoid)
file(GLOB_RECURSE lingmoid_SRCS
    "operation/*.cpp"
    "operation/*.h"
    "operation/qrc/lingmoid.qrc"
)

add_library(${pluginName} MODULE
    ${lingmoid_SRCS}
)

set(lingmoid_Libraries
    ${DCC_FRAME_Library}
    ${QT_NS}::DBus
    ${QT_NS}::Concurrent
    ${DTK_NS}::Gui
)

target_link_libraries(${pluginName} PRIVATE
    ${lingmoid_Libraries}
)

oceanui_install_plugin(NAME ${pluginName} TARGET ${pluginName})
