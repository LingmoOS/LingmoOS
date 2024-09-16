@page global global--dtk全局工具组件

# DTk global：dtk全局工具类
## dsysinfo：dtk系统信息工具类

[dsysinfo.h 详细文档](dsysinfo_8h.html)<br>
这里放一个最小化使用dsysinfo的例子:<br>
cmake:

```cmake
cmake_minimum_required(VERSION 3.13) # cmake版本要求
set (VERSION "1.0.0" CACHE STRING "define project version") # 定义项目版本
set(BIN_NAME test) # 定义项目名称
project(test) # 定义项目名称
file(GLOB_RECURSE SRCS "*.h" "*.cpp")  # 定义项目源文件
find_package(Qt5 REQUIRED COMPONENTS Core) # 寻找Qt5
find_package(DtkCore REQUIRED) # 寻找DtkCore

add_executable(test
    main.cpp # 源文件
)

target_link_libraries(test PRIVATE
    ${DtkCore_LIBRARIES} # 链接DtkCore
   Qt5::Core # 链接Qt5
)

```
cpp:
```cpp
#include <DSysInfo> // 引入DSysInfo
#include <qdebug.h> // 引入qdebug.h
DCORE_USE_NAMESPACE // 使用dtkcore命名空间

int main(int argc, char **argv) {

  qDebug() << DSysInfo::deepinType(); // 打印deepin类型
  qDebug() << DSysInfo::ProductType(); // 打印产品类型
  return 0;
}
```
其余的组件使用方法类似，这里就不一一列举了

@defgroup dglobal
@brief dtk设置全局工具组件
@details
    dtk全局工具组件，提供了设置全局工具的功能。<br>
    包含以下功能:
      * dsysinfo：dtk系统信息工具类
      * dconfig：dtk配置文件工具类
