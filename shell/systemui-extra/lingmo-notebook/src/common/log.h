#ifndef LOG_H
#define LOG_H

#include <QString>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#endif // LOG_H