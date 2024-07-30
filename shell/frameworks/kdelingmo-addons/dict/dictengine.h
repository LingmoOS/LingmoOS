/*
    SPDX-FileCopyrightText: 2007 Jeff Cooper <weirdsox11@gmail.com>
    SPDX-FileCopyrightText: 2007 Thomas Georgiou <TAGeorgiou@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <QMap>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QVariantMap>
#include <array>

/**
 * This class evaluates the basic expressions given in the interface.
 */

class DictEngine : public QObject
{
    Q_OBJECT

public:
    DictEngine(QObject *parent = nullptr);
    ~DictEngine() override;

Q_SIGNALS:
    /**
     * @param socketError the type of the last socket error
     * @param errorString a human-readable description of the last socket error
     */
    void dictErrorOccurred(QAbstractSocket::SocketError socketError, const QString &errorString);

    /**
     * @param loading @c true if the dict finder is downloading dict list from
     * the Internet, @c false otherwise.
     */
    void dictLoadingChanged(bool loading);

    void dictsRecieved(const QMap<QString, QString> &dicts);
    void definitionRecieved(const QString &html);

public Q_SLOTS:
    void requestDicts();
    void requestDefinition(const QString &query);
    void requestDefinition(const QString &query, const QString &server);
    void requestDefinition(const QString &query, const QByteArray &dictionary);
    void requestDefinition(const QString &query, const QString &server, const QByteArray &dictionary);

private Q_SLOTS:
    /**
     * Slot to asynchronously handle \readyRead signal emitted
     * when receiving new definitions.
     */
    void slotDefinitionReadyRead();

    /**
     * Slot to process definition data when any end response
     * listed in \m_definitionResponses is received, or to handle
     * \QTimer::timeout signal when no new data are received
     * from the socket and no end response is received.
     */
    void slotDefinitionReadFinished();

    void socketClosed();

private:
    void getDefinition();
    void getDicts();
    void setDict(const QByteArray &dict);
    void setServer(const QString &server);

    QHash<QString, QString> m_dictNameToDictCode;
    QTcpSocket *m_tcpSocket = nullptr;
    QString m_currentWord;
    QByteArrayList m_dictNames;
    QString m_serverName;
    QMap<QString, QMap<QString, QString>> m_availableDictsCache;

    /**
     * Stores temporarily received definition data
     */
    QByteArray m_definitionData;

    /**
     * When \QTimer::timeout is emitted, the existing socket will be closed
     * and deleted, and will emit \definitionRecieved to stop the loading
     * process.
     */
    QTimer m_definitionTimer;

    // https://datatracker.ietf.org/doc/html/rfc2229
    const std::array<QByteArray, 4> m_definitionResponses;
};
