# Dtk6Log

`Dtk6Log` 是基于 `Qt6` 的 `C++`` 应用的简单、便捷、线程安全日志库

## 简单示例

```cpp
#include <QCoreApplication>
#include <QDebug>

#include <dloghelper.h>
#include <ConsoleAppender.h>

DLOG_CORE_USE_NAMESPACE
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    auto consoleAppender = new ConsoleAppender;
    consoleAppender->setFormat("[%{type:-7}] <%{Function}> %{message}\n");
    dlogger->registerAppender(consoleAppender);

    dInfo("Starting the application");

    dWarning() << "Something went wrong." << "Result code is" << -1;

    return 0;
}
```

## 项目中引入 Dtk6Log 

CMakeLists.txt 文件:
```cmake
find_package(Dtk6Log REQUIRED)
...

...
TARGET_LINK_LIBRARIES(${your_target} ... Dtk6::Log)
```

包含 `dloghelper.h` 然后根据需要的 appender 包含头文件:
```cpp
#include <dloghelper.h>
#include <ConsoleAppender.h>
```
