// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef Q_HTTP_SERVER_FWD
#define Q_HTTP_SERVER_FWD

#include <QHash>
#include <QString>

/*!
 * A map of request or response headers
 */
typedef QHash<QString, QString> HeaderHash;

// QHttpServer
class QHttpServer;
class QHttpConnection;
class QHttpRequest;
class QHttpResponse;

// Qt
class QTcpServer;
class QTcpSocket;

// http_parser
struct http_parser_settings;
struct http_parser;

#endif
