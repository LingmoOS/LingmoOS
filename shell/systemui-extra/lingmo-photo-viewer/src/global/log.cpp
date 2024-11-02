#include "log.h"

#include <QDateTime>
#include <QFile>

void Log::msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    //判断当前允许显示和写入的级别
    if (type < Variable::g_logLevel) {
        return;
    }

    //写入日志文件
    echoToFile(type, msg);

    //打印到控制台
    fprintf(stdout, "%s\n", msg.toStdString().c_str());
    fflush(stdout);

    // 遇到致命错误，需要终止程序
    if (type == QtFatalMsg) {
        abort();
    }
}

void Log::echoToFile(QtMsgType type, const QString &msg)
{
    if (!Variable::g_logToFile) {
        return;
    }

    //写入哪个文件
    bool logToSecondFile = Variable::getSettings("logToSecondFile").toBool();
    QString filePath;
    if (logToSecondFile) {
        filePath = Variable::LOG_PATH.second;
    } else {
        filePath = Variable::LOG_PATH.first;
    }

    //如果不可写则不处理
    QFile logFile(filePath);
    if (!logFile.open(QIODevice::Append | QIODevice::Text | QIODevice::WriteOnly)) {
        return;
    }

    //获取当前时间
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString("yy.MM.dd hh:mm:ss +zzz");

    //构造写入内容
    QString outMsg;
    switch (type) {
    case QtDebugMsg:
        outMsg = QString("[%1 D][%2]: %3 \n").arg(timeStr).arg(getpid()).arg(msg);
        break;
    case QtInfoMsg:
        outMsg = QString("[%1 I][%2]: %3 \n").arg(timeStr).arg(getpid()).arg(msg);
        break;
    case QtWarningMsg:
        outMsg = QString("[%1 W][%2]: %3 \n").arg(timeStr).arg(getpid()).arg(msg);
        break;
    case QtCriticalMsg:
        outMsg = QString("[%1 C][%2]: %3 \n").arg(timeStr).arg(getpid()).arg(msg);
        break;
    case QtFatalMsg:
        outMsg = QString("[%1 F][%2]: %3 \n").arg(timeStr).arg(getpid()).arg(msg);
    }

    //写入
    logFile.write(outMsg.toLocal8Bit());
    logFile.close();

    //如果文件大小没超出限制则不处理
    QFileInfo info(filePath);
    if (info.size() < Variable::MAX_LOG_SIZE) {
        return;
    }

    //切换到另一个文件
    if (logToSecondFile) {
        QFile clearFile(Variable::LOG_PATH.first);
        if (clearFile.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
            Variable::setSettings("logToSecondFile", false);
            clearFile.close();
        }
    } else {
        QFile clearFile(Variable::LOG_PATH.second);
        if (clearFile.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
            Variable::setSettings("logToSecondFile", true);
            clearFile.close();
        }
    }
}
