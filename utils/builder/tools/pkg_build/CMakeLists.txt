cmake_minimum_required(VERSION 3.5)
project(lingmo-pkgbuild)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 创建 repo_manager 库
add_library(repo_manager STATIC
    repo_manager/src/repo_manager.cpp
)

target_include_directories(repo_manager PUBLIC 
    repo_manager/include
)

# 主程序
add_executable(lingmo-pkgbuild 
    src/main.cpp
    src/lingmo_pkgbuild.cpp
)

target_include_directories(lingmo-pkgbuild PRIVATE include)
target_link_libraries(lingmo-pkgbuild PRIVATE repo_manager)

# 仓库管理工具
add_executable(lingmo-repotool
    repo_manager/src/main.cpp
)

target_link_libraries(lingmo-repotool PRIVATE repo_manager)

# 添加对 unistd.h 的检查
include(CheckIncludeFile)
check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H)
endif()

# 添加翻译文件
add_subdirectory(po)

# 安装可执行文件
install(TARGETS lingmo-pkgbuild lingmo-repotool
    RUNTIME DESTINATION bin
) 