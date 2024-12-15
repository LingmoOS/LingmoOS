// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef DLOGGER_DEFINE_H
#define DLOGGER_DEFINE_H

#include "dlog_global.h"
#include <QLoggingCategory>

DLOG_CORE_BEGIN_NAMESPACE

class Logger;
class DLogHelper;
LIBDLOG_SHARED_EXPORT Logger *loggerInstance();
#define dlogger loggerInstance()

#define DLOG_CTX(category) QMessageLogContext(__FILE__, __LINE__, Q_FUNC_INFO, category)

// include DLog or dloghelper.h
#define dTrace DLogHelper(Logger::Trace, DLOG_CTX("default")).write
#define dDebug DLogHelper(Logger::Debug, DLOG_CTX("default")).write
#define dInfo DLogHelper(Logger::Info, DLOG_CTX("default")).write
#define dWarning DLogHelper(Logger::Warning, DLOG_CTX("default")).write
#define dError DLogHelper(Logger::Error, DLOG_CTX("default")).write
#define dFatal DLogHelper(Logger::Fatal, DLOG_CTX("default")).write

#define dCDebug(category) DLogHelper(Logger::Debug, DLOG_CTX(category)).write()
#define dCInfo(category) DLogHelper(Logger::Info, DLOG_CTX(category)).write()
#define dCWarning(category) DLogHelper(Logger::Warning, DLOG_CTX(category)).write()
#define dCError(category) DLogHelper(Logger::Error, DLOG_CTX(category)).write()
#define dCFatal(category) DLogHelper(Logger::Fatal, DLOG_CTX(category)).write()

#define dTraceTime                                         \
    DLogHelper helper(Logger::Trace, DLOG_CTX("default")); \
    helper.timing
#define dDebugTime                                         \
    DLogHelper helper(Logger::Debug, DLOG_CTX("default")); \
    helper.timing
#define dInfoTime                                         \
    DLogHelper helper(Logger::Info, DLOG_CTX("default")); \
    helper.timing

#define dAssert(cond) ((!(cond)) ? loggerInstance()->writeAssert(__FILE__, __LINE__, Q_FUNC_INFO, #cond) : qt_noop())
#define dAssertX(cond, msg) ((!(cond)) ? loggerInstance()->writeAssert(__FILE__, __LINE__, Q_FUNC_INFO, msg) : qt_noop())

DLOG_CORE_END_NAMESPACE

#endif // DLOGGER_DEFINE_H
