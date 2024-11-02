/*
 * Log 类中实现了 Qt 的日志注册函数并且调用了 lingmosdk-log 库中的日志模块，调用方需要链接 Qt库 及 lingmosdk-log 库
 */

#ifndef LOG_HPP_
#define LOG_HPP_

#include <qapplication.h>
#include <libkylog.h>
#include <QString>
#include <QByteArray>

namespace kabase
{

class KabaseLog
{
public:
    KabaseLog() = default;
    ~KabaseLog() = default;

    static void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        QByteArray localMsg = msg.toLocal8Bit();
        const char *message = localMsg.constData();
        const char *file = context.file ? context.file : "";
        const char *function = context.function ? context.function : "";

        switch (type) {
        case QtDebugMsg:
            klog_debug("%s (%s:%u,%s)\n", message, file, context.line, function);
            break;
        case QtInfoMsg:
            klog_info("%s (%s:%u,%s)\n", message, file, context.line, function);
            break;
        case QtWarningMsg:
            klog_warning("%s (%s:%u,%s)\n", message, file, context.line, function);
            break;
        case QtCriticalMsg:
            klog_err("%s (%s:%u,%s)\n", message, file, context.line, function);
            break;
        case QtFatalMsg:
            klog_emerg("%s (%s:%u,%s)\n", message, file, context.line, function);
            break;
        }

        return;
    };
};

}

#endif
