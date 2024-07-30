/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "exceptions.h"

#include "apijob.h"
#include "bugzilla_debug.h"

namespace Bugzilla
{
APIException::APIException(const QJsonDocument &document)
    : APIException(document.object())
{
}

APIException::APIException(const QJsonObject &object)
{
    if (object.isEmpty()) {
        return;
    }
    m_isError = object.value(QStringLiteral("error")).toBool(m_isError);
    m_message = object.value(QStringLiteral("message")).toString(m_message);
    m_code = object.value(QStringLiteral("code")).toInt(m_code);
    // Our bugzilla is a bit bugged. It doesn't necessarily set error to true
    // but instead keeps it at null. Because of this we need to possibly shimy
    // the bool to align with reality.
    if (object.value(QStringLiteral("error")).type() == QJsonValue::Null && m_code > 0 && !m_message.isNull()) {
        m_isError = true;
    }
    if (m_isError) {
        qCWarning(BUGZILLA_LOG) << "APIException:" << object.toVariantHash();
    }
}

APIException::APIException(const APIException &other)
    : m_isError(other.m_isError)
    , m_message(other.m_message)
    , m_code(other.m_code)
{
}

QString APIException::whatString() const
{
    return QStringLiteral("[%1] %2").arg(m_code).arg(m_message);
}

void APIException::maybeThrow(const QJsonDocument &document)
{
    APIException ex(document);

    if (ex.isError()) {
        ex.raise();
    }
}

ProtocolException::ProtocolException(const APIJob *job)
    : Exception()
    , m_job(job)
{
    qCWarning(BUGZILLA_LOG) << "ProtocolException:" << whatString() << job->error() << job->errorText() << job->errorString();
}

ProtocolException::ProtocolException(const ProtocolException &other)
    : m_job(other.m_job)
{
}

QString ProtocolException::whatString() const
{
    // String generally includes the error code, so no extra logic needed.
    return m_job->errorString();
}

void ProtocolException::maybeThrow(const APIJob *job)
{
    if (job->error() == KJob::NoError) {
        return;
    }
    throw ProtocolException(job);
}

Exception::~Exception()
{
    delete m_what;
}

const char *Exception::what() const noexcept
{
    strcpy(m_what, qUtf8Printable(whatString()));
    return m_what;
}

RuntimeException::RuntimeException(const QString &reason)
    : m_reason(reason)
{
    qCWarning(BUGZILLA_LOG) << "RuntimeException:" << whatString();
}

QString RuntimeException::whatString() const
{
    return m_reason;
}

} // namespace Bugzilla
