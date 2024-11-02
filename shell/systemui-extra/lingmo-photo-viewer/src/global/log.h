#ifndef LOG_H
#define LOG_H

#include "variable.h"


class Log
{
public:
    static void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    static void echoToFile(QtMsgType type, const QString &msg);
};

#endif // LOG_H
