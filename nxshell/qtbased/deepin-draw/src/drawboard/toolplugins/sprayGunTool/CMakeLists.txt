# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

project("sprayguntool")

# 设置cmake参数
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
remove_definitions(-DDRAWSHARED_LIBRARY)

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/plugins)
# 设置工程名字

set(QtModule Core Gui Widgets DBus PrintSupport Svg Concurrent LinguistTools)

# 查找Qt相关的库链接
find_package(Qt5 REQUIRED ${QtModule})

# 定义测试用例文件及测试资源文件
set(AllFileDirs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    )
foreach(item IN LISTS AllFileDirs)
    include_directories(${item})
    list(APPEND allSourceFiles ${item}/*.cpp)
    list(APPEND allSourceFiles ${item}/*.c)
    list(APPEND allSourceFiles ${item}/*.h)
    list(APPEND allQRCFiles    ${item}/*.qrc)
endforeach()
FILE(GLOB allSourceG ${allSourceFiles})
FILE(GLOB allQRCG ${allQRCFiles})

FILE(GLOB targetTsFFiles ./translations/*.ts)
qt5_create_translation(tragetQmFFiles ${allSourceG} ${targetTsFFiles} OPTIONS -no-obsolete)
# 设置不删除生成的文件夹内容文件（设置此项是为了翻译后的文件不被删除）
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

add_library(${PROJECT_NAME} SHARED ${allSourceG} ${allQRCG} ${tragetQmFFiles})

#------------------------------ 添加第三方库  DTK -------------------------------

if(INDEPENDENTCMAKELISTS)
    # 使用第三方库需要用到的一个包
    find_package(PkgConfig REQUIRED)

    # 检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3rd_lib
    pkg_check_modules(3rd_lib REQUIRED
            dtkwidget dtkgui
            )
endif()

# 添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS} ${BASE_LIB_INCLUDEDIRS})

# 将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES} ${BASE_LIB})

#------------------------------添加第三方库end-------------------------------------

# 将工程与Qt模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})

# install Translation files:
install(FILES ${tragetQmFFiles} DESTINATION ${ToolPluginTransDir})

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${ToolPluginDir}
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION ${ToolPluginDir}
        INCLUDES DESTINATION allHeaders)
