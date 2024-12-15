// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <ctime>

#include "filedownloader.h"
#include "../../widgets/progresswidget.h"

class ImageDownloadWidget: public ProgressWidget
{
    Q_OBJECT
public:
    ImageDownloadWidget(QWidget *parent = nullptr);
    const QString getFileName() const {
        return m_downloader.getFileName();
    }
    const QString getFilePath() const {
        return m_downloader.getFilePath();
    }

Q_SIGNALS:
    void download(QString url);
    void done();
    void error(QNetworkReply::NetworkError, QString);
    void abort();

public Q_SLOTS:
    void updateEtaText(int p);

private:
    FileDownloader m_downloader;

    void initUI();
    void initConnections();
};
