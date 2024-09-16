cmake_minimum_required(VERSION 3.10)

project(drawBoard)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    option(USE_DTK "use dtk as ui." ON)
    set(InstallHeaderDir ${PREFIX}/include/${PROJECT_NAME})
    set(InstallLibDir ${PREFIX}/lib)
    #玲珑环境BASE_LIB为空
    if("${BASE_LIB}" STREQUAL "")
        set(InstallTransDir ${PREFIX}/share/deepin-draw/translations)
        set(PluginDir ${PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}/plugins)
    else()
        set(InstallTransDir ${PREFIX}/share/${BASE_LIB}/translations)
        set(PluginDir ${PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}/${BASE_LIB}/plugins)
    endif()

    set(PluginDir ${PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}/${BASE_LIB}/plugins)
    set(PluginTransDir ${InstallTransDir}/plugins)

    set(ToolPluginDir ${PluginDir}/tools)
    set(ToolPluginTransDir ${PluginTransDir}/tools)
else()
    add_definitions(-DNO_DTK)
    set(CMAKE_MODULE_PATH $ENV{PATH})
    if(CMAKE_SYSTEM_NAME MATCHES "Windows")
        set(InstallHeaderDir ${PROJECT_BINARY_DIR}/include)
        set(InstallLibDir ${PROJECT_BINARY_DIR}/lib)
        set(InstallTransDir ${PROJECT_BINARY_DIR}/share/translations)

        set(PluginDir ${PROJECT_BINARY_DIR}/share/plugins)
        set(PluginTransDir ${InstallTransDir}/plugins)

        set(ToolPluginDir ${PluginDir}/tools)
        set(ToolPluginTransDir ${PluginTransDir}/tools)
    endif()
endif()
add_subdirectory(drawboard)
add_subdirectory(toolplugins)
#add_subdirectory(test)
#add_subdirectory(example)

