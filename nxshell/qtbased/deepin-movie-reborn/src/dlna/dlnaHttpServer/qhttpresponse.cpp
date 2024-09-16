// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qhttpresponse.h"

#include <QDateTime>
#include <QLocale>

#include "qhttpserver.h"
#include "qhttpconnection.h"

QHttpResponse::QHttpResponse(QHttpConnection *connection)
    // TODO: parent child relation
    : QObject(0),
      m_connection(connection),
      m_headerWritten(false),
      m_sentConnectionHeader(false),
      m_sentContentLengthHeader(false),
      m_sentTransferEncodingHeader(false),
      m_sentDate(false),
      m_keepAlive(true),
      m_last(false),
      m_useChunkedEncoding(false),
      m_finished(false)
{
   connect(m_connection, SIGNAL(allBytesWritten()), this, SIGNAL(allBytesWritten()));
}

QHttpResponse::~QHttpResponse()
{
}

void QHttpResponse::setHeader(const QString &field, const QString &value)
{
    if (!m_finished)
        m_headers[field] = value;
    else
        qWarning() << "QHttpResponse::setHeader() Cannot set headers after response has finished.";
}

void QHttpResponse::writeHeader(const char *field, const QString &value)
{
    //qDebug() << "writeHeader" << field << value.toUtf8();
    if (!m_finished) {
        m_connection->write(field);
        m_connection->write(": ");
        m_connection->write(value.toUtf8());
        m_connection->write("\r\n");
    } else
        qWarning()
            << "QHttpResponse::writeHeader() Cannot write headers after response has finished.";
}

void QHttpResponse::writeHeaders()
{
    if (m_finished)
        return;

    foreach(const QString & name, m_headers.keys()) {
        QString value = m_headers[name];
        if (name.compare("connection", Qt::CaseInsensitive) == 0) {
            m_sentConnectionHeader = true;
            if (value.compare("close", Qt::CaseInsensitive) == 0)
                m_last = true;
            else
                m_keepAlive = true;
        } else if (name.compare("transfer-encoding", Qt::CaseInsensitive) == 0) {
            m_sentTransferEncodingHeader = true;
            if (value.compare("chunked", Qt::CaseInsensitive) == 0)
                m_useChunkedEncoding = true;
        } else if (name.compare("content-length", Qt::CaseInsensitive) == 0)
            m_sentContentLengthHeader = true;
        else if (name.compare("date", Qt::CaseInsensitive) == 0)
            m_sentDate = true;

        /// @todo Expect case (??)

        //qDebug() << "writeHeaders" << name.toLatin1() << value.toLatin1();
        //writeHeader(name.toLatin1(), value.toLatin1());
        writeHeader(name.toLatin1(), value);
    }

    if (!m_sentConnectionHeader) {
        if (m_keepAlive && (m_sentContentLengthHeader || m_useChunkedEncoding)) {
            writeHeader("Connection", "keep-alive");
        } else {
            m_last = true;
            writeHeader("Connection", "close");
        }
    }

    if (!m_sentContentLengthHeader && !m_sentTransferEncodingHeader) {
        if (m_useChunkedEncoding)
            writeHeader("Transfer-Encoding", "chunked");
        else
            m_last = true;
    }

    // Sun, 06 Nov 1994 08:49:37 GMT - RFC 822. Use QLocale::c() so english is used for month and
    // day.
    if (!m_sentDate)
        writeHeader("Date",
                    QLocale::c().toString(QDateTime::currentDateTimeUtc(),
                                          "ddd, dd MMM yyyy hh:mm:ss") + " GMT");
}

bool QHttpResponse::isFinished()
{
    return m_finished;
}

bool QHttpResponse::isHeaderWritten()
{
    return m_headerWritten;
}

bool QHttpResponse::isWritable()
{
    return m_connection->isWritable();
}

void QHttpResponse::writeHead(int status)
{
    if (m_finished) {
        qWarning()
            << "QHttpResponse::writeHead() Cannot write headers after response has finished.";
        return;
    }

    if (m_headerWritten) {
        qWarning() << "QHttpResponse::writeHead() Already called once for this response.";
        return;
    }

    m_connection->write(
        QString("HTTP/1.1 %1 %2\r\n").arg(status).arg(STATUS_CODES[status]).toLatin1());

    writeHeaders();

    m_connection->write("\r\n");

    m_headerWritten = true;
}

void QHttpResponse::writeHead(StatusCode statusCode)
{
    writeHead(static_cast<int>(statusCode));
}

void QHttpResponse::write(const QByteArray &data)
{
    if (m_finished) {
        qWarning() << "QHttpResponse::write() Cannot write body after response has finished.";
        return;
    }

    if (!m_headerWritten) {
        qWarning() << "QHttpResponse::write() You must call writeHead() before writing body data.";
        return;
    }

    m_connection->write(data);
}

bool QHttpResponse::flush()
{
    return m_connection->flush();
}

void QHttpResponse::waitForBytesWritten(int msecs)
{
    m_connection->waitForBytesWritten(msecs);
}

void QHttpResponse::end(const QByteArray &data)
{
    if (m_finished) {
        qWarning() << "QHttpResponse::end() Cannot write end after response has finished.";
        return;
    }

    if (data.size() > 0)
        write(data);
    m_finished = true;

    Q_EMIT done();

    /// @todo End connection and delete ourselves. Is this a still valid note?
    deleteLater();
}

void QHttpResponse::connectionClosed()
{
    qWarning() << "QHttpResponse::connectionClosed()";
    m_finished = true;
    Q_EMIT done();
    deleteLater();
}
