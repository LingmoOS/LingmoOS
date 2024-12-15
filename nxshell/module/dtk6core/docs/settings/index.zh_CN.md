@page dsettings dsettings--dtk设置工具组件

# DSettings

## DSettings：dtk设置组件

[dsettings.h 详细文档](dsettings_8h.html)

项目目录结构如下：

```bash
├── CMakeLists.txt
├── data
│   └── settings.json
├── data.qrc
└── main.cpp
```

CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.1.0)                # 指定cmake最低版本

project(dsetting-example VERSION 1.0.0 LANGUAGES CXX)# 指定项目名称, 版本, 语言 cxx就是c++

set(CMAKE_CXX_STANDARD 11)                           # 指定c++标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)                  # 指定c++标准要求,至少为11以上

set(CMAKE_AUTORCC ON)                                # support qt resource file # 支持qt资源文件

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)                # 支持 clangd

if (CMAKE_VERSION VERSION_LESS "3.7.0")              # 如果cmake版本小于3.7.0
    set(CMAKE_INCLUDE_CURRENT_DIR ON)                # 设置包含当前目录
endif()

find_package(DtkCore REQUIRED)                       # 寻找Dtk组件Core

add_executable(${PROJECT_NAME}                       # 生成可执行文件
    main.cpp
    data.qrc
)

target_link_libraries(${PROJECT_NAME} PRIVATE        # 添加需要链接的共享库
    Dtk::Core
)
```

settings.json:

```json
{
    "groups": [{
        "key": "base",
        "name": "Basic settings",
        "groups": [{
                "key": "open_action",
                "name": "Open Action",
                "options": [{
                        "key": "alway_open_on_new",
                        "type": "checkbox",
                        "text": "Always Open On New Windows",
                        "default": true
                    },
                    {
                        "key": "open_file_action",
                        "name": "Open File:",
                        "type": "combobox",
                        "default": ""
                    }
                ]
            },
            {
                "key": "new_tab_windows",
                "name": "New Tab & Window",
                "options": [{
                        "key": "new_window_path",
                        "name": "New Window Open:",
                        "type": "combobox",
                        "default": ""
                    },
                    {
                        "key": "new_tab_path",
                        "name": "New Tab Open:",
                        "type": "combobox",
                        "default": ""
                    }
                ]
            }
        ]
    }]
}
```

data.qrc

```xml
<RCC>
    <qresource prefix="/">
        <file>data/settings.json</file>
    </qresource>
</RCC>
```

main.cpp

```cpp
#include <QCoreApplication>
#include <DSettings>
#include <qsettingbackend.h>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <QTemporaryFile>
#include <QDebug>

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 初始化一个存储后端
    QTemporaryFile tmpFile;
    tmpFile.open();
    QPointer<QSettingBackend> backend = new QSettingBackend(tmpFile.fileName());

    // 从json中初始化配置
    QPointer<DSettings> settings = DSettings::fromJsonFile(":/data/settings.json");
    settings->setBackend(backend);

    // 读取配置
    // 该组中包含一个base的root组,两个子组: open_action/new_tab_windows,每个子组有包含若干选项。
    // 对于"New Window Open:"这个配置,其完整的访问id为base.new_tab_windows.new_window_path。
    QPointer<DSettingsOption> opt = settings->option("base.new_tab_windows.new_window_path");
    qDebug() << opt->value();

    // 修改配置
    opt->setValue("Test");
    qDebug() << opt->value();

    // 获取所有keys
    QStringList keys = settings->keys();
    qDebug() << keys;

    // base.open_action对应的组
    QPointer<DSettingsGroup> group = settings->group("base.open_action");
    qDebug() << group->key();

    return a.exec();
}
```

编译运行:

```bash
cmake -Bbuild
cmake --build build
./build/dsetting-example
```

运行结果如下图:

![img](/docs/src/dsettings.png)

@defgroup dsettings
@brief dtk设置组件
