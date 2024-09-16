#cmake版本设置 
cmake_minimum_required(VERSION 3.7)
project(deepin-fcitxconfigtool-plugin)

# 环境设置
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)#moc 预编译包含QObject文件，生成xx_moc.h cpp
set(CMAKE_AUTORCC ON)#开启rcc 编译资源文件qrc
set(CMAKE_AUTOUIC ON)#开启uic 编译转换*.ui文件生存设计界面对应的.h .cpp文件
set(CMAKE_CXX_STANDARD 14)

#加载子目录
add_subdirectory(src)
add_subdirectory(tests)

#调用外部程序  生成qm文件
execute_process(COMMAND bash translate_generation.sh
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

install(FILES com.deepin.fcitx-config.gschema.xml DESTINATION share/glib-2.0/schemas)
install(CODE "execute_process(COMMAND glib-compile-schemas ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)")
