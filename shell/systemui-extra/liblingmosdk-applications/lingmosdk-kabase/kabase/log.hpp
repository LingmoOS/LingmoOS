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

#ifndef LOG_H_
#define LOG_H_

#include <QString>

namespace kdk
{
namespace kabase
{

class Log
{
public:
    enum LogLevel {
        Debug = 0, /* 调试 */
        Info,      /* 信息 */
        Warning,   /* 警告 */
        Error,     /* 错误 */
        Fatal      /* 致命 */
    };

    Log();
    Log(LogLevel logLevel);
    ~Log();

    /*
     * 功能 : 操作符重载 , 基于 lingmosdk
     * 参数 : int 变量
     * 返回 : *this
     */
    const Log &operator<<(int value) const;
    const Log &operator<<(float value) const;
    const Log &operator<<(double value) const;
    const Log &operator<<(const char *str) const;
    const Log &operator<<(char *str) const;
    const Log &operator<<(std::string str) const;
    const Log &operator<<(QString str) const;
    const Log &operator<<(bool value) const;
    const Log &operator<<(void *ptr) const;

    /**
     * @brief 用于 Qt 注册日志函数 , 不应该单独调用
     *
     * @param type 日志类型
     * @param context 调用文件信息
     * @param msg 需要打印的日志
     *
     * @return 无
     */
    static void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    LogLevel m_logLevel;
};

extern Log debug;
extern Log info;
extern Log warning;
extern Log error;
extern Log fatal;

} /* namespace kabase */
} /* namespace kdk */

#endif
