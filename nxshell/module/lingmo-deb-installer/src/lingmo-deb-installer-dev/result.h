// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESULT_H
#define RESULT_H

template <typename T>
class Result
{
public:
    static Result<T> ok(const T &value);
    static Result<T> err(const T &value);

    Result(const bool stat, const T &value);

    bool is_ok() const { return m_ok; }
    T unwrap() const { return m_value; }

private:
    bool m_ok;
    T m_value;
};

template <typename T>
Result<T> Result<T>::ok(const T &value)
{
    return {true, value};
}

template <typename T>
Result<T> Result<T>::err(const T &value)
{
    return {false, value};
}

template <typename T>
Result<T>::Result(const bool stat, const T &value)
    : m_ok(stat)
    , m_value(value)
{
}

#endif  // RESULT_H
