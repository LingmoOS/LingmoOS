/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "dict_object.h"
#include <KLocalizedString>
#include <QDebug>
#include <QQuickWebEngineProfile>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlSchemeHandler>

class DictSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    DictSchemeHandler(QObject *parent)
        : QWebEngineUrlSchemeHandler(parent)
    {
    }
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        job->fail(QWebEngineUrlRequestJob::UrlInvalid);
        const QString word = job->requestUrl().path();
        Q_EMIT wordClicked(word);
    }

Q_SIGNALS:
    void wordClicked(const QString &word);
};

DictObject::DictObject(QObject *parent)
    : QObject(parent)
    , m_selectedDict(QStringLiteral("wn"))
{
    m_webProfile = new QQuickWebEngineProfile(this);
    auto *schemeHandler = new DictSchemeHandler(this);
    connect(schemeHandler, &DictSchemeHandler::wordClicked, this, &DictObject::lookup);
    connect(&m_engine, &DictEngine::definitionRecieved, this, [this](const QString &html) {
        Q_EMIT definitionFound(html);
    });
    connect(&m_engine, &DictEngine::dictErrorOccurred, this, &DictObject::slotDictErrorOccurred);
    m_webProfile->installUrlSchemeHandler("dict", schemeHandler);
}

void DictObject::lookup(const QString &word)
{
    m_hasError = false;
    Q_EMIT hasErrorChanged();

    if (!word.isEmpty()) {
        // Look up new definition
        Q_EMIT searchInProgress();
        m_engine.requestDefinition(word, m_selectedDict.toUtf8());
    }
}

QString DictObject::selectedDictionary() const
{
    return m_selectedDict;
}

void DictObject::setSelectedDictionary(const QString &dict)
{
    m_selectedDict = dict;
}

QQuickWebEngineProfile *DictObject::webProfile() const
{
    return m_webProfile;
}

bool DictObject::hasError() const
{
    return m_hasError;
}

QAbstractSocket::SocketError DictObject::errorCode() const
{
    return m_errorCode;
}

QString DictObject::errorString() const
{
    return m_errorString;
}

void DictObject::slotDictErrorOccurred(QAbstractSocket::SocketError socketError, const QString &errorString)
{
    m_hasError = true;
    m_errorCode = socketError;
    m_errorString = errorString;

    Q_EMIT errorCodeChanged();
    Q_EMIT errorStringChanged();
    Q_EMIT hasErrorChanged();
}

#include "dict_object.moc"
