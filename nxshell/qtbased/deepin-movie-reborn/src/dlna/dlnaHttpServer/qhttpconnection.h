// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef Q_HTTP_CONNECTION
#define Q_HTTP_CONNECTION

#include "qhttpserverapi.h"
#include "qhttpserverfwd.h"

#include <QObject>

/// @cond nodoc

class QHTTPSERVER_API QHttpConnection : public QObject
{
    Q_OBJECT

public:
    QHttpConnection(QTcpSocket *socket, QObject *parent = 0);
    virtual ~QHttpConnection();

    void write(const QByteArray &data);
    bool flush();
    void waitForBytesWritten(int msecs = 30000);
    bool isWritable();

Q_SIGNALS:
    void newRequest(QHttpRequest *, QHttpResponse *);
    void allBytesWritten();

private Q_SLOTS:
    void parseRequest();
    void responseDone();
    void socketDisconnected();
    void updateWriteCount(qint64);

private:
    static int MessageBegin(http_parser *parser);
    static int Url(http_parser *parser, const char *at, size_t length);
    static int HeaderField(http_parser *parser, const char *at, size_t length);
    static int HeaderValue(http_parser *parser, const char *at, size_t length);
    static int HeadersComplete(http_parser *parser);
    static int Body(http_parser *parser, const char *at, size_t length);
    static int MessageComplete(http_parser *parser);

private:
    QTcpSocket *m_socket;
    http_parser *m_parser;
    http_parser_settings *m_parserSettings;

    // Since there can only be one request at any time even with pipelining.
    QHttpRequest *m_request;

    QByteArray m_currentUrl;
    // The ones we are reading in from the parser
    HeaderHash m_currentHeaders;
    QString m_currentHeaderField;
    QString m_currentHeaderValue;

    // Keep track of transmit buffer status
    qint64 m_transmitLen;
    qint64 m_transmitPos;
};

/// @endcond

#endif
