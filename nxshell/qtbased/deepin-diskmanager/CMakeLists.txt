cmake_minimum_required(VERSION 3.0)
project(deepin-diskmanager)


option(DOTEST "option for test" OFF)

# 是否开启单元测试编译
set(DOTEST ON)

option(DOAPP "option for test app" OFF)
set(DOAPP ON)

#compile flags
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")
  # Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT")
else()
  # -Wl, -O2 Enable linker optimizations
  # -Wl, --gc-sections Remove unused code resulting from -fdsta-sections and
  # -ffunction-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -Wl,--gc-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -Wl,--gc-sections")
endif()

# 检查内存泄漏代码
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O2")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -O2")
#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g -fsanitize=address -O2")
#set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -g -fsanitize=address -O2")
#set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g -fsanitize=address -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O2")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -O2")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -fsanitize=address -O2")
#set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -g -fsanitize=address -O2")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}  -g -fsanitize=address -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -z relro -z now -z noexecstack -pie")

ADD_SUBDIRECTORY(basestruct)
ADD_SUBDIRECTORY(log)
ADD_SUBDIRECTORY(application)
ADD_SUBDIRECTORY(service)

# 开启单元测试
if(DOTEST)
    if(DOAPP)
        # 添加gocv覆盖率文件的输出
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
        # 设置测试模块
        set(QT Core Gui Widgets Network DBus Sql LinguistTools Svg Test)
        find_package(Qt5 COMPONENTS ${QT} REQUIRED)
        # 添加子模块apptest
        add_subdirectory(tests)
    else()
    # 添加gocv覆盖率文件的输出
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
    # 设置测试模块
    set(QT Core Gui Widgets Network DBus Sql LinguistTools Svg Multimedia Test)
    find_package(Qt5 COMPONENTS ${QT} REQUIRED)
    # 添加子模块test
    add_subdirectory(test)
    endif()
endif()

