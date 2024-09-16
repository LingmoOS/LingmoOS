\mainpage dtkcore
@brief dtk

# dtkcore

## 简介

dtkcore 是一个基于Qt的C++库，它提供了一些常用的工具类，以及一些基础的模块，如日志、插件、网络、线程、数据库、文件、图形、音频、视频、系统信息等

## 使用

现在的dtkcore>=5.6版本使用CMake来管理各个模块，所以使用dtkcore时，需要先安装CMake(CMake>=3.10),然后需要在你的CMake项目中引入dtkcore的CMake模块，如下：

```cmake
find_package(Dtk6Core REQUIRED)

target_link_libraries(<target>
    Dtk6::Core
)
```

```qmake
QT += dtkcore
```

```bash
pkg-config --cflags --libs dtk6core

# pkgconfig find Dtk6Core in qmake
# CONFIG += link_pkgconfig
# PKGCONFIG += dtk6core

# pkgconfig find Dtk6Core in cmake
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(Dtk6Core REQUIRED IMPORTED_TARGET dtk6core)
# target_link_libraries(<target> PkgConfig::Dtk6Core )
```

以上示例仅为最小示例，并不能单独作为CMake项目使用，需要你自己添加其他的CMake模块，如Qt的CMake模块，以及你自己的CMake模块.
@note 注意：dtkcore的QMake模块会自动引入Qt5的QMake模块，所以不需要再次引入Qt5的QMake模块,但是在使用CMake的时候必须手动引入Qtcore的CMake模块

## 文档

阅读文档建议从模块页面开始，模块页面提供了dtkcore的各个模块的简介，以及各个模块的使用示例。
@subpage DLog

dtkcore的文档使用doxygen管理,由deepin_doc_doc_go_sig提供维护支持, 如果你也想加入sig,请访问[deepin_doc_doc_go_sig](https://matrix.to/#/#deepin_doc_doc_go:matrix.org)

## 许可

dtkcore使用LGPLv3许可证，你可在此许可证下自由使用dtkcore <br>
dtkcore的文档使用[CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/)许可证，你可在此许可证下自由使用dtkcore的文档,但是转发或者引用时必须注明出处。
