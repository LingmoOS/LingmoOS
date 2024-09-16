// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "error_context.h"

#include <QString>

class ErrorContextData : public QSharedData
{
public:
    // default constructor
    ErrorContextData() {}
    // copy constructor
    ErrorContextData(const ErrorContextData &rhs)
        : QSharedData(rhs)
        , __alignment(rhs.__alignment)
        , m_code(rhs.m_code)
        , m_subCode(rhs.m_subCode)
        , m_errName(rhs.m_errName)
        , m_errMessage(rhs.m_errMessage)
    {
    }
    // copy assignment
    ErrorContextData &operator=(const ErrorContextData &rhs)
    {
        if (this != &rhs) {
            m_code = rhs.m_code;
            m_subCode = rhs.m_subCode;
            m_errName = rhs.m_errName;
            m_errMessage = rhs.m_errMessage;
        }
        return *this;
    }
    ~ErrorContextData() {}

    friend class ErrorContext;

private:
    int __alignment {0};
    // error code
    int m_code {0};
    // sub error code
    int m_subCode {0};
    // error name
    QString m_errName {};
    // error detail description
    QString m_errMessage {};
};

// default constructor
ErrorContext::ErrorContext(QObject* parent)
    : QObject (parent)
    , data(new ErrorContextData())
{
}

// overloaded constructor
ErrorContext::ErrorContext(int code, int subCode, const QString &errName, const QString &errMessage, QObject* parent)
    : QObject (parent)
    , data(new ErrorContextData())
{
    // data = new ErrorContextData();
    data->m_code = code;
    data->m_subCode = subCode;
    data->m_errName = errName;
    data->m_errMessage = errMessage;
}
// copy constructor
ErrorContext::ErrorContext(const ErrorContext &rhs)
    : data(rhs.data)
{
}
// assignment operator
ErrorContext &ErrorContext::operator=(const ErrorContext &rhs)
{
    if (this != &rhs) {
        data.operator = (rhs.data);
    }
    return *this;
}
ErrorContext::~ErrorContext() {}

// overloaded compare operator
bool ErrorContext::operator==(const ErrorContext &other) const
{
    return (data->m_code == other.getCode() && data->m_subCode == other.getSubCode() &&
            data->m_errName == other.getSubCode() && data->m_errMessage == getErrorMessage());
}

// get error code
int ErrorContext::getCode() const
{
    return data->m_code;
}
// set error code
void ErrorContext::setCode(int code)
{
    data->m_code = code;
}

// get sub error code
int ErrorContext::getSubCode() const
{
    return data->m_subCode;
}
// set sub error code
void ErrorContext::setSubCode(int subCode)
{
    data->m_subCode = subCode;
}

// get error name
QString ErrorContext::getErrorName() const
{
    return data->m_errName;
}

// set error name
void ErrorContext::setErrorName(const QString &errName)
{
    data->m_errName = errName;
}

// get error message
QString ErrorContext::getErrorMessage() const
{
    return data->m_errMessage;
}

// set error message
void ErrorContext::setErrorMessage(const QString &errMessage)
{
    data->m_errMessage = errMessage;
}

// convenient ! operator
bool operator!(const ErrorContext &ec) noexcept
{
    return !ec.operator bool();
}

// check if it's a valid error context
bool ErrorContext::isValid() const
{
    return operator bool();
}

// bool operator
ErrorContext::operator bool() const
{
    return (data->m_code != 0 || data->m_subCode != 0);
}

// reset error context
void ErrorContext::reset()
{
    data->m_code = 0;
    data->m_subCode = 0;
    data->m_errName = "";
    data->m_errMessage = "";
}
