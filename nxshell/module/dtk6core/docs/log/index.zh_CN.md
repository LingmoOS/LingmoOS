@page DLog dlog--DTK日志组件

# Dlog：DTK日志组件

## 简介

DTK日志组件，提供了日志的输出、日志级别的设置、日志文件的设置等功能。为了简化日志的使用，我们提供了两种使用方式，一种是使用宏，另一种是使用类。
但是更为建议使用宏的方式进行使用。因为使用宏可以输出更多的信息，比如文件名、函数名、行号等。

## 使用

### 宏

DTK日志组件提供了一系列的宏，用于输出日志。这些宏的定义在dloggerdefs.h中
如果需要使用宏，则需要在你的代码中包含`DLog`文件。
@note DTK更推荐使用CPP标准引入头文件的方式，即使用`#include <DLog>`的方式引入头文件，而不是使用`#include "xxxxx.h"`的方式引入头文件。

示例代码如下：

```cpp
#include <DLog>
#include <QCoreApplication>
DCORE_USE_NAMESPACE

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
#ifdef Q_OS_LINUX
    DLogManager::registerJournalAppender();
#endif
    DLogManager::registerConsoleAppender();
    dDebug() << "this is a debug message";
    return app.exec();
}
```

上述代码中，我们使用了dDebug()宏，用于输出一条debug级别的日志。在输出日志时，我们可以使用`<<`运算符，将多个变量输出到日志中。
注意，registerJournalAppender()仅在Linux平台下有效，因为journal仅在Linux平台下有效。而registerConsoleAppender()则是在所有平台下都有效。

### 类

使用类的方式需要先创建一个logger对象，然后使用logger对象的方法来输出日志，具体请参见logger文档

## 注意事项

### 日志级别

DTK使用的日志级别与Qt的日志级别一致，分别为：Trace、Debug、Info、Warning、Error、Fatal。其中Trace是最低级别，Fatal是最高级别。
与之不同的是journal的日志级别和DTK的日志级别不一致，journal的日志级别分别为：emergency、alert、critical、error、warning、notice、info、debug。
这是因为journal的日志级别是从syslog中继承过来的，而syslog的日志级别是从BSD的syslog中继承过来的。而DTK的日志级别是从Qt的日志级别中继承过来的。
下表是DTK日志级别和journal日志级别的对应关系
| DTK值       | 序号 | 含义                                  | syslog值     | syslog序号 |
|---------|----|-------------------------------------|-------------|----------|
| dTrace   | 0  | 追踪级别,可用于大部分不需要的记录,用于内部代码追踪          | 无           |          |
| dDebug   | 1  | 调试级别,用于软件的调试。                       | LOG_DEBUG   | 7        |
| dInfo    | 2  | 信息级别,可用于信息记录,这可能不仅对开发者有意义           | LOG_INFO    | 6        |
| dWarning | 3  | 警告,可以用来记录你的应用程序检测到的一些非致命的警告         | LOG_WARNING | 4        |
| dError   | 4  | 错误,可能是一个较大问题,导致你的程序工作出错,但不至于崩溃      | LOG_ERR     | 3        |
| dFatal   | 5  | 致命错误，用于不可恢复的错误，在写入日志记录后立即崩溃应用程序(终止) | LOG_EMERG   | 0        |
|         |    |                                     | LOG_ALERT   | 1        |
|         |    |                                     | LOG_CRIT    | 2        |
|         |    |                                     | LOG_NOTICE  | 6        |

### 日志文件

如果你选择使用journal作为日志输出，那么你需要在系统中安装systemd（一般的linux发行版都会自带systemd）。并且需要确保`/var/log/journal/`
目录的存在，否则journal会将日志写入到`/run/log/journal/`目录中。

如果你选择使用控制台作为日志输出，那么不会生成日志文件。

如果你选择使用文件作为日志输出，那么你需要确保你的日志文件所在的目录存在，否则会导致日志文件无法创建。

### 日志格式

如果你选择使用journal作为日志输出，且使用宏的方式进行日志输出，那么日志的格式为：
`[时间] [进程ID] [线程ID] [日志级别] [文件名:行号] [函数名] [日志内容]`，你仅需要关注日志内容即可。其他的信息都是journal自动添加的。


@defgroup dlog
@brief DTK日志组件
@details
    DTK日志组件，提供了日志的输出、日志级别的设置、日志文件的设置等功能。
    对于DTK日志组件，我们提供了两种使用方式，一种是使用宏，另一种是使用类。
    但是更为建议使用宏的方式进行使用
