// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DERROR_H
#define DERROR_H
#include "dtkcore_global.h"
#include <QString>
#include <QDebug>

DCORE_BEGIN_NAMESPACE
/**
 * @brief 对于错误的包装类
 */
class DError
{
public:
    /*!
     * @brief 默认构造函数
     * @attention 错误代码默认为-1，错误信息默认为空
     */
    DError() noexcept
        : m_code(-1)
        , m_msg()
    {
    }

    /*!
     * @brief 拷贝构造函数
     */
    DError(const DError &e) noexcept
        : m_code(e.m_code)
        , m_msg(e.m_msg)
    {
    }

    /*!
     * @brief 移动构造函数
     * @attention 移动后原对象不可用
     */
    DError(DError &&e) noexcept
        : m_code(e.m_code)
        , m_msg(std::move(e).m_msg)
    {
    }

    /*!
     * @brief 构造函数
     * @param[in] code 错误代码
     * @param[in] msg  错误信息
     */
    DError(qint64 code, const QString &msg) noexcept
        : m_code(code)
        , m_msg(msg)
    {
    }

    /*!
     * @brief 构造函数
     * @param[in] code 错误代码
     * @param[in] msg  错误信息
     * @attention 使用此构造函数后原错误信息不可用
     */
    DError(qint64 code, QString &&msg) noexcept
        : m_code(code)
        , m_msg(std::move(msg))
    {
    }

    /*!
     * @brief 重载拷贝赋值运算符
     */
    DError &operator=(const DError &e)
    {
        m_code = e.m_code;
        m_msg = e.m_msg;
        return *this;
    }

    /*!
     * @brief 重载移动赋值运算符
     * @attention 赋值后原对象不可用
     */
    DError &operator=(DError &&e)
    {
        m_code = e.m_code;
        m_msg = std::move(e).m_msg;
        return *this;
    }

    /*!
     * @brief 默认析构函数
     */
    ~DError() = default;

    /*!
     * @brief 获取错误代码
     * @return 错误代码
     */
    qint64 getErrorCode() const noexcept { return m_code; }

    /*!
     * @brief 设置错误代码
     * @param[in] code 错误代码
     */
    void setErrorCode(qint64 code) &noexcept { m_code = code; }

    /*!
     * @brief 获取错误信息
     * @return 错误信息的const引用
     */
    const QString &getErrorMessage() const & { return m_msg; }

    /*!
     * @brief 获取错误信息
     * @attention 函数返回错误信息后，原信息不可用
     * @return 错误信息
     */
    QString getErrorMessage() const && { return std::move(m_msg); }

    /*!
     * @brief 设置错误信息
     * @param[in] msg 错误信息
     */
    void setErrorMessage(const QString &msg) & { m_msg = msg; }

    /*!
     * @brief 重载相等运算符
     */
    friend bool operator==(const DError &x, const DError &y) noexcept { return x.m_code == y.m_code and x.m_msg == y.m_msg; }

    /*!
     * @brief 重载不等运算符
     */
    friend bool operator!=(const DError &x, const DError &y) noexcept { return !(x == y); }

    /*!
     * @brief 重载输出运算符
     */
    friend QDebug operator<<(QDebug debug, const DError &e)
    {
        debug << "Error Code:" << e.m_code << "Message:" << e.m_msg;
        return debug;
    }

private:
    qint64 m_code;
    QString m_msg;
};
DCORE_END_NAMESPACE
#endif
