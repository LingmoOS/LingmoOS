/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef DICT_OBJECT_H
#define DICT_OBJECT_H

#include "../../dict/dictengine.h"
#include <QObject>
#include <QQuickWebEngineProfile>

class DictObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickWebEngineProfile *webProfile READ webProfile CONSTANT)
    Q_PROPERTY(QString selectedDictionary READ selectedDictionary WRITE setSelectedDictionary)

    /**
     * @return @c true if there is a network error when finding the definition,
     * @c false otherwise.
     */
    Q_PROPERTY(bool hasError READ hasError NOTIFY hasErrorChanged)

    /**
     * @return the type of the last socket error
     */
    Q_PROPERTY(QAbstractSocket::SocketError errorCode READ errorCode NOTIFY errorCodeChanged)

    /**
     * @return a human-readable description of the last socket error
     */
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    explicit DictObject(QObject *parent = nullptr);

    QQuickWebEngineProfile *webProfile() const;

    QString selectedDictionary() const;
    void setSelectedDictionary(const QString &dict);

    bool hasError() const;
    QAbstractSocket::SocketError errorCode() const;
    QString errorString() const;

public Q_SLOTS:
    void lookup(const QString &word);

Q_SIGNALS:
    void searchInProgress();
    void definitionFound(const QString &html);

    void hasErrorChanged();
    void errorCodeChanged();
    void errorStringChanged();

private Q_SLOTS:
    void slotDictErrorOccurred(QAbstractSocket::SocketError socketError, const QString &errorString);

private:
    QString m_dataEngineName;
    QString m_selectedDict;

    DictEngine m_engine;
    QQuickWebEngineProfile *m_webProfile;

    bool m_hasError = false;
    QAbstractSocket::SocketError m_errorCode = QAbstractSocket::UnknownSocketError;
    QString m_errorString;
};

#endif
