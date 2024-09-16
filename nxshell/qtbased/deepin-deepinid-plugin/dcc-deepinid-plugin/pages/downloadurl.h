// Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QMap>
#include <QMutex>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
class QTemporaryFile;
QT_END_NAMESPACE

class DownloadUrl : public QObject
{
    Q_OBJECT
public:
    explicit DownloadUrl(QObject *parent = nullptr);
    virtual ~DownloadUrl();
    void downloadFileFromURL(const QString &url, const QString &filePath, bool fullname = false);

Q_SIGNALS:
    void fileDownloaded(const QString &fileName);

public Q_SLOTS:
    void onDownloadFileComplete(QNetworkReply *reply);
    void onDownloadFileError(const QString &url, const QString &fileName);

private:
    QString m_finalPath;
    QNetworkAccessManager *m_manager;
    QTemporaryFile *m_file;
    bool m_isReady;
    QMap<QString, QString> m_retryMap;
    QMutex m_downloadlock;
};
