# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置cmake参数
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置工程名字
project(deepin-draw-test)

option(DEEPINDRAW_TEST_OFFSCREENT "test in off screen." ON)
if(DEEPINDRAW_TEST_OFFSCREENT)
    add_definitions(-DTEST_OFFSCREENT)
endif()

option(DEEPINDRAW_TEST_DELETE_ITEM "test delete items fuctions" ON)
if(DEEPINDRAW_TEST_DELETE_ITEM)
    add_definitions(-DTEST_DELETE_ITEM)
endif()

option(DEEPINDRAW_TEST_BLUR_ITEM "test blur fuctions" ON)
if(DEEPINDRAW_TEST_BLUR_ITEM)
    add_definitions(-DTEST_BLUR_ITEM)
endif()

option(DEEPINDRAW_TEST_RECT_ITEM "test rectangle item fuctions" ON)
if(DEEPINDRAW_TEST_RECT_ITEM)
    add_definitions(-DTEST_RECT_ITEM)
endif()

option(DEEPINDRAW_TEST_ELLIPSE_ITEM "test ellipse item fuctions" ON)
if(DEEPINDRAW_TEST_ELLIPSE_ITEM)
    add_definitions(-DTEST_ELLIPSE_ITEM)
endif()

option(DEEPINDRAW_TEST_START_ITEM "test star item fuctions" ON)
if(DEEPINDRAW_TEST_START_ITEM)
    add_definitions(-DTEST_START_ITEM)
endif()

option(DEEPINDRAW_TEST_PICTURE_ITEM "test picture item fuctions" ON)
if(DEEPINDRAW_TEST_PICTURE_ITEM)
    add_definitions(-DTEST_PICTURE_ITEM)
endif()

option(DEEPINDRAW_TEST_TRIANGLE_ITEM "test triangle item fuctions" ON)
if(DEEPINDRAW_TEST_TRIANGLE_ITEM)
    add_definitions(-DTEST_TRIANGLE_ITEM)
endif()

option(DEEPINDRAW_TEST_POLYGON_ITEM "test polygon item fuctions" ON)
if(DEEPINDRAW_TEST_POLYGON_ITEM)
    add_definitions(-DTEST_POLYGON_ITEM)
endif()

option(DEEPINDRAW_TEST_LINE_ITEM "test line item fuctions" ON)
if(DEEPINDRAW_TEST_LINE_ITEM)
    add_definitions(-DTEST_LINE_ITEM)
endif()

option(DEEPINDRAW_TEST_PEN_ITEM "test pen item fuctions" ON)
if(DEEPINDRAW_TEST_PEN_ITEM)
    add_definitions(-DTEST_PEN_ITEM)
endif()

option(DEEPINDRAW_TEST_TEXT_ITEM "test text item fuctions" ON)
if(DEEPINDRAW_TEST_TEXT_ITEM)
    add_definitions(-DTEST_TEXT_ITEM)
endif()
option(DEEPINDRAW_TEST_LAYER_ITEM "test layer item fuctions" ON)
if(DEEPINDRAW_TEST_LAYER_ITEM)
    add_definitions(-DTEST_LAYER_ITEM)
endif()

option(DEEPINDRAW_TEST_CUT_ITEM "test cut fuctions" ON)
if(DEEPINDRAW_TEST_CUT_ITEM)
    add_definitions(-DTEST_CUT_ITEM)
endif()

option(DEEPINDRAW_TEST_SCANLE_ITEM "test scanle item fuctions" ON)
if(DEEPINDRAW_TEST_SCANLE_ITEM)
    add_definitions(-DTEST_SCANLE_ITEM)
endif()

option(DEEPINDRAW_TEST_Z_ITEM "test z item fuctions" ON)
if(DEEPINDRAW_TEST_Z_ITEM)
    add_definitions(-DTEST_Z_ITEM)
endif()

option(DEEPINDRAW_TEST_ITEMALIGNMENT_ITEM "test item alignment fuctions" ON)
if(DEEPINDRAW_TEST_ITEMALIGNMENT_ITEM)
    add_definitions(-DTEST_ITEMALIGNMENT_ITEM)
endif()

option(DEEPINDRAW_TEST_FUNCTION_ITEM "test more other fuctions" ON)
if(DEEPINDRAW_TEST_FUNCTION_ITEM)
    add_definitions(-DTEST_FUNCTION_ITEM)
endif()

option(DEEPINDRAW_TEST_DIALOG_ITEM "test dialog fuctions" ON)
if(DEEPINDRAW_TEST_DIALOG_ITEM)
    add_definitions(-DTEST_DIALOG)
endif()

# 设置Qt模块
set(QtModule Core Gui Widgets DBus PrintSupport Svg Concurrent LinguistTools Test)

# 查找Qt相关的库链接
find_package(Qt5 REQUIRED ${QtModule})

#set complier/link flags
#if(DEEPINDRAW_TEST)
#    message(DEEPINDRAW_TESTDEEPINDRAW_TESTDEEPINDRAW_TESTDEEPINDRAW_TEST)
#    #0.DEEPINDRAW_TEST mean that we should creat the lib that can get cover info
#    #mips not suport -fsanitize
#    if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
#        set(CMAKE_CXX_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage")
#        set(CMAKE_C_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage")
#        set(CMAKE_Ｌ_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage -lgcov")
#    else()
#        message(DEEPINDRAW_TESTDEEPINDRAW_TESTDEEPINDRAW_TESTDEEPINDRAW_TEST1)
#        set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -O1 -fprofile-arcs -ftest-coverage")
#        set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -O1 -fprofile-arcs -ftest-coverage")
#        set(CMAKE_Ｌ_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -O1 -fprofile-arcs -ftest-coverage -lgcov")
#    endif()
#else()
#    #1.no DEEPINDRAW_TEST mean that we just need one clearly lib(not include -ftest-coverage or -fsanitize)
#    if(CMAKE_BUILD_TYPE MATCHES "Debug")
#        #DEBUG时不做优化方便调试
#        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
#        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
#    else()
#        #加速编译优化参数
#        if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
#            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi")
#            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi")
#            SET(CMAKE_EXE_LINKER_FLAGS  "-pie")
#        else()
#            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections")
#            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3  -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections")
#            SET(CMAKE_EXE_LINKER_FLAGS  "-pie")
#        endif()
#    endif()

#    #2.sometime we also want to test deepin-draw memery info
#    if(DEEPINDRAW_SANITIZERS)
#        set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#        set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    endif()
#endif()
#注意mips不支持-fsanitize
if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    #set(CMAKE_CXX_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage")
    #set(CMAKE_C_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage")
    set(CMAKE_Ｌ_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage")

    #mips not support -fsanitize,so only -lgcov when link ${BASE_LIB}.
    set(CMAKE_EXE_LINKER_FLAGS    "-lgcov")
else()
   #set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -O1 -fprofile-arcs -ftest-coverage")
   set(CMAKE_CXX_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage")
   #set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -O1 -fprofile-arcs -ftest-coverage")
    set(CMAKE_Ｌ_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -O1 -fprofile-arcs -ftest-coverage")

    # because ${BASE_LIB} lib is compile with -fsanitize and -lgcov,so we should add those flag when link it.
    set(CMAKE_EXE_LINKER_FLAGS    "-fsanitize=undefined,address,leak -lgcov")

    # notice code that we should output memery use info.
    add_definitions(-DENABLE_FSANITIZE)
endif()

include_directories(${BASE_DRAW_INCLUDEDIRS} ${BASE_LIB_INCLUDEDIRS} ${DRAW_BOARD_LIB_INCLUDEDIRS})

# 定义测试用例文件及测试资源文件
set(AllTestFileDirs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/testItems
    ${CMAKE_CURRENT_SOURCE_DIR}/testUIs
    ${CMAKE_CURRENT_SOURCE_DIR}/testResource
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-draw/drawfiles
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-draw/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-draw/attribution
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-draw/attribution/stylewidgets
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-draw/drawtools
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-draw/drawitems
    )
foreach(item IN LISTS AllTestFileDirs)
    include_directories(${item})
    list(APPEND allTestSourceFiles ${item}/*.cpp)
    list(APPEND allTestSourceFiles ${item}/*.c)
    list(APPEND allTestSourceFiles ${item}/*.h)
    list(APPEND allTestQRCFiles    ${item}/*.qrc)
endforeach()
FILE(GLOB allTestSource ${allTestSourceFiles})
FILE(GLOB allTestQRC ${allTestQRCFiles})


add_executable(${PROJECT_NAME} ${allTestQRC} ${allTestSource} )

target_include_directories(${PROJECT_NAME} PUBLIC ${BASE_LIB_INCLUDEDIRS})
target_link_libraries(${PROJECT_NAME} gmock gmock_main gtest gtest_main pthread ${CMAKE_DL_LIBS} ${BASE_LIB})

#------------------------------ 添加第三方库  DTK -------------------------------

# 使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)

# 检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3RMODULES
pkg_check_modules(3RMODULES REQUIRED
        dtkwidget dtkgui
        )

# 添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${PROJECT_NAME} PUBLIC ${3RMODULES_INCLUDE_DIRS})

# 将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${PROJECT_NAME} ${3RMODULES_LIBRARIES})

#------------------------------添加第三方库end-------------------------------------

# 将工程与Qt模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})
