# Dlog

Simple, convinient and thread safe logger for Qt-based C++ apps

## Short example

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

## Adding DLog to your project

Add this repo as a git submodule to your project. 


Include it to your CMakeLists.txt file:
```cmake
find_package(DtkLog REQUIRED)
...

...
TARGET_LINK_LIBRARIES(${your_target} ... Dtk::Log)
```

Include `dloghelper.h` and one or several appenders of your choice:
```cpp
#include <dloghelper.h>
#include <ConsoleAppender.h>
```
