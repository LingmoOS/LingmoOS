// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DFontSizeManager>

#include <QColor>
#include <QFont>
#include <QIcon>
#include <QUrl>

#include "imagedownloadwidget.h"

#define ICON_SIZE 112

ImageDownloadWidget::ImageDownloadWidget(QWidget *parent)
    : ProgressWidget(parent)
{
    initUI();
    initConnections();
}

void ImageDownloadWidget::initUI()
{
    m_iconLabel->setPixmap(QIcon::fromTheme("media-optical").pixmap(ICON_SIZE, ICON_SIZE));
    m_titleLabel->setText(tr("Downloading the V23 image file"));
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T4, QFont::Bold);
    m_tipLabel->setText(tr("You can continue using the computer without being affected"));
    DFontSizeManager::instance()->bind(m_tipLabel, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(m_etaLabel, DFontSizeManager::T8, QFont::Normal);
}

void ImageDownloadWidget::initConnections()
{
    connect(this, &ImageDownloadWidget::download, this, &ImageDownloadWidget::onStart);
    connect(this, &ImageDownloadWidget::download, [this] (QString url) {
        m_downloader.startDownload(url);
    });
    connect(this, &ImageDownloadWidget::abort, &m_downloader, &FileDownloader::abort);
    connect(&m_downloader, &FileDownloader::progress, this, &ProgressWidget::updateProgress);
    connect(&m_downloader, &FileDownloader::progress, this, &ImageDownloadWidget::updateEtaText);
    connect(&m_downloader, &FileDownloader::done, this, &ImageDownloadWidget::done);
    connect(&m_downloader, &FileDownloader::error, this, [this] {
        emit error(m_downloader.getError(), m_downloader.getErrorLog());
    });
}

void ImageDownloadWidget::updateEtaText(int p)
{
    if (m_initProgress < 0)
    {
        m_initProgress = p;
    }

    QString etaTimeString = p > m_initProgress ? secondsToReadableTime((time(nullptr) - m_startTime) * (100 - p) / (p - m_initProgress)) : tr("N/A");
    m_etaLabel->setText(tr("Time remaining: %1 / File size: %2 GB").arg(etaTimeString).arg(m_downloader.getTotalSize() / 1024. / 1024. / 1024., 0, 'f', 2));
}
