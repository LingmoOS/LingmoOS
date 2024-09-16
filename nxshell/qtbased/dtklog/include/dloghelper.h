// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DLOGHELPER_H
#define DLOGHELPER_H

#include <dlog_global.h>
#include "Logger.h"

#include <QString>
#include <QDebug>

DLOG_CORE_BEGIN_NAMESPACE
class DLogHelperPrivate;
class LIBDLOG_SHARED_EXPORT DLogHelper : public QObject
{
    Q_DISABLE_COPY(DLogHelper)
public:
    DLogHelper(Logger::LogLevel level, const QMessageLogContext &context, QObject *parent = nullptr);
    ~DLogHelper();

    void write(const char *msg, ...) Q_ATTRIBUTE_FORMAT_PRINTF(2, 3);
    void write(const QString &msg);
    QDebug write();

    void timing(const QString &msg, QObject *context = nullptr);

    static Logger::LogLevel levelFromQtMsgType(QtMsgType mt);
    static QtMsgType qtMsgTypeFromLogLevel(Logger::LogLevel lvl);

    Q_DECLARE_PRIVATE(DLogHelper)
};

DLOG_CORE_END_NAMESPACE

#endif // DLOGHELPER_H
