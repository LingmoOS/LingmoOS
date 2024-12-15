cmake_minimum_required(VERSION 3.18)
set(pluginName accounts)
file(GLOB_RECURSE accounts_SRCS
    "operation/*.cpp"
    "operation/*.h"
    "operation/qrc/${pluginName}.qrc"
)

add_library(${pluginName} MODULE
    ${accounts_SRCS}
)

pkg_check_modules(LINGMO_PW_CHECK REQUIRED liblingmo_pw_check)

find_package(PolkitQt6-1)
set(accounts_Libraries
    ocean-control-center
    ${QT_NS}::DBus
    ${DTK_NS}::Core
     ${DTK_NS}::Gui
     ${QT_NS}::Concurrent
     PolkitQt6-1::Agent
     ${LINGMO_PW_CHECK_LIBRARIES}
)

target_link_libraries(${pluginName} PRIVATE
    ${accounts_Libraries}
)

oceanui_install_plugin(NAME ${pluginName} TARGET ${pluginName})
