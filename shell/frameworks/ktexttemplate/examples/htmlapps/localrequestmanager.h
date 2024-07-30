/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef LOCALREQUESTMANAGER_H
#define LOCALREQUESTMANAGER_H

#include <QNetworkAccessManager>

#include <KTextTemplate/Engine>

class LocalRequestManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    LocalRequestManager(KTextTemplate::Engine *engine, QObject *parent = 0);

    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData = 0);

private:
    KTextTemplate::Engine *m_engine;
};

#endif
