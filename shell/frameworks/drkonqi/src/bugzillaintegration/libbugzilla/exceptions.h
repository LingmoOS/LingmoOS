/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <QException>

class QJsonDocument;
class QJsonObject;

namespace Bugzilla
{
class APIJob;

/**
 * Root class for exceptions. Simply a QException which has the what backed
 * by a QString.
 */
class Exception : public QException
{
public:
    using QException::QException;
    ~Exception() override;

    virtual QString whatString() const = 0;
    const char *what() const noexcept override;

private:
    char *m_what = nullptr;
};

/**
 * Generic runtime exception.
 */
class RuntimeException : public Exception
{
public:
    explicit RuntimeException(const QString &reason);
    RuntimeException *clone() const override
    {
        return new RuntimeException(*this);
    }
    QString whatString() const override;

private:
    QString m_reason;
};

/**
 * Translates an API error into an exception for easy handling.
 * Specifically when the API sends an error object in the body attempting to
 * access the JSON blob through one of the convenience accessors
 * (e.g. job.object()) will instead raise an exception.
 */
class APIException : public Exception
{
public:
    explicit APIException(const QJsonDocument &document);
    explicit APIException(const QJsonObject &object);
    APIException(const APIException &other);

    void raise() const override
    {
        throw *this;
    }
    APIException *clone() const override
    {
        return new APIException(*this);
    }
    QString whatString() const override;

    bool isError() const
    {
        return m_isError;
    }

    static void maybeThrow(const QJsonDocument &document);

private:
    bool m_isError = false;
    QString m_message;
    int m_code = -1;
};

/**
 * Translates an KJob/APIJob error into an excpetion for easy handling.
 */
class ProtocolException : public Exception
{
public:
    explicit ProtocolException(const APIJob *job);
    ProtocolException(const ProtocolException &other);

    void raise() const override
    {
        throw *this;
    }
    ProtocolException *clone() const override
    {
        return new ProtocolException(*this);
    }
    QString whatString() const override;

    static void maybeThrow(const APIJob *job);

private:
    const APIJob *m_job = nullptr;
};

} // namespace Bugzilla

#endif // EXCEPTIONS_H
