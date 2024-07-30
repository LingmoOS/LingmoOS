/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef TEMPLATEREPLY_H
#define TEMPLATEREPLY_H

#include <QBuffer>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <KTextTemplate/Context>
#include <KTextTemplate/Template>

class TemplateReply : public QNetworkReply
{
    Q_OBJECT
public:
    TemplateReply(const QNetworkRequest &req,
                  const QNetworkAccessManager::Operation op,
                  KTextTemplate::Template t,
                  KTextTemplate::Context c,
                  QObject *parent = 0);

    virtual void abort();
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual bool atEnd() const;
    virtual qint64 bytesAvailable() const;
    virtual bool canReadLine() const;
    virtual void close();
    virtual bool isSequential() const;
    virtual qint64 pos() const;
    virtual bool reset();
    virtual bool seek(qint64 pos);
    virtual void setReadBufferSize(qint64 size);
    virtual qint64 size() const;
    virtual bool waitForReadyRead(int msecs);
    virtual bool open(OpenMode mode);

private:
    KTextTemplate::Template m_t;
    KTextTemplate::Context m_c;
    QBuffer *m_buffer;
};

#endif
