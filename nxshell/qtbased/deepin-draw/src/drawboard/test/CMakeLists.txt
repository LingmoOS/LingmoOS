# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置cmake参数
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置工程名字
project(drawboard-test)

#remove_definitions(-DDRAWSHARED_LIBRARY)

##set complier/link flags
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

# 设置Qt模块
set(QtModule Core Gui Widgets DBus PrintSupport Svg Concurrent LinguistTools)


# 查找Qt相关的库链接
find_package(Qt5 REQUIRED ${QtModule})

include_directories(${BASE_LIB_INCLUDEDIRS})

set(SRCDIRSTEST
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# 包含源码文件夹并获取其下所有源代码文件
foreach(item IN LISTS SRCDIRSTEST)
    include_directories(${item})
    list(APPEND SRC_CPPS_TEST    ${item}/*.cpp)
    list(APPEND SRC_HEADERS_TEST ${item}/*.h)
    list(APPEND SRC_QRCS_TEST    ${item}/*.qrc)
endforeach()
FILE (GLOB allSourceTest  ${SRC_CPPS_TEST})
FILE (GLOB allHeadersTest ${SRC_HEADERS_TEST})
FILE (GLOB allQrcsTest    ${SRC_QRCS_TEST})

add_executable(${PROJECT_NAME} ${allSourceTest} ${allHeadersTest} ${allQrcsTest})

target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES} ${CMAKE_DL_LIBS} ${BASE_LIB})

# 将工程与Qt模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})





