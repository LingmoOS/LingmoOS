/*
 * liblingmosdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <libkylog.h>

#include <QFile>

#include "log.hpp"

namespace kdk
{
namespace kabase
{

Log::Log() {}

Log::Log(LogLevel logLevel)
{
    this->m_logLevel = logLevel;
}

Log::~Log() {}

const Log &Log::operator<<(int value) const
{
    switch (this->m_logLevel) {
    case LogLevel::Debug:
        klog_debug("%d\n", value);
        break;
    case LogLevel::Info:
        klog_info("%d\n", value);
        break;
    case LogLevel::Warning:
        klog_warning("%d\n", value);
        break;
    case LogLevel::Error:
        klog_err("%d\n", value);
        break;
    case LogLevel::Fatal:
        klog_emerg("%d\n", value);
        break;
    default:
        break;
    }

    return *this;
}

const Log &Log::operator<<(const char *str) const
{
    switch (this->m_logLevel) {
    case LogLevel::Debug:
        klog_debug("%s\n", str);
        break;
    case LogLevel::Info:
        klog_info("%s\n", str);
        break;
    case LogLevel::Warning:
        klog_warning("%s\n", str);
        break;
    case LogLevel::Error:
        klog_err("%s\n", str);
        break;
    case LogLevel::Fatal:
        klog_emerg("%s\n", str);
        break;
    default:
        break;
    }

    return *this;
}

const Log &Log::operator<<(float value) const
{
    switch (this->m_logLevel) {
    case LogLevel::Debug:
        klog_debug("%f\n", value);
        break;
    case LogLevel::Info:
        klog_info("%f\n", value);
        break;
    case LogLevel::Warning:
        klog_warning("%f\n", value);
        break;
    case LogLevel::Error:
        klog_err("%f\n", value);
        break;
    case LogLevel::Fatal:
        klog_emerg("%f\n", value);
        break;
    default:
        break;
    }

    return *this;
}

const Log &Log::operator<<(double value) const
{
    switch (this->m_logLevel) {
    case LogLevel::Debug:
        klog_debug("%lf\n", value);
        break;
    case LogLevel::Info:
        klog_info("%lf\n", value);
        break;
    case LogLevel::Warning:
        klog_warning("%lf\n", value);
        break;
    case LogLevel::Error:
        klog_err("%lf\n", value);
        break;
    case LogLevel::Fatal:
        klog_emerg("%lf\n", value);
        break;
    default:
        break;
    }

    return *this;
}

const Log &Log::operator<<(void *ptr) const
{
    switch (this->m_logLevel) {
    case LogLevel::Debug:
        klog_debug("%p\n", ptr);
        break;
    case LogLevel::Info:
        klog_info("%p\n", ptr);
        break;
    case LogLevel::Warning:
        klog_warning("%p\n", ptr);
        break;
    case LogLevel::Error:
        klog_err("%p\n", ptr);
        break;
    case LogLevel::Fatal:
        klog_emerg("%p\n", ptr);
        break;
    default:
        break;
    }

    return *this;
}

const Log &Log::operator<<(std::string str) const
{
    const char *temp = str.c_str();
    return this->operator<<(temp);
}

const Log &Log::operator<<(char *str) const
{
    const char *temp = str;
    return this->operator<<(temp);
}

const Log &Log::operator<<(QString str) const
{
    std::string temp = str.toStdString();
    return this->operator<<(temp);
}

const Log &Log::operator<<(bool value) const
{
    if (value == true) {
        return this->operator<<("true");
    } else {
        return this->operator<<("false");
    }
}

void Log::logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static size_t extendSize = 128;

    QByteArray localMsg = msg.toLocal8Bit();
    const char *message = localMsg.constData();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    size_t len = strlen(message) + strlen(file) + strlen(function);

    char *mem = (char *)malloc(len + extendSize);
    if (mem == NULL) {
        error << "kabase : Log module failed to request memory !";
        return;
    }
    memset(mem, 0x00, len + extendSize);

    sprintf(mem, "%s (%s:%u,%s)", message, file, context.line, function);

    switch (type) {
    case QtDebugMsg:
        debug << mem;
        break;
    case QtInfoMsg:
        info << mem;
        break;
    case QtWarningMsg:
        warning << mem;
        break;
    case QtCriticalMsg:
        error << mem;
        break;
    case QtFatalMsg:
        fatal << mem;
    default:
        break;
    }

    free(mem);

    return;
}

Log debug(Log::LogLevel::Debug);
Log info(Log::LogLevel::Info);
Log warning(Log::LogLevel::Warning);
Log error(Log::LogLevel::Error);
Log fatal(Log::LogLevel::Fatal);

} /* namespace kabase */
} /* namespace kdk */
