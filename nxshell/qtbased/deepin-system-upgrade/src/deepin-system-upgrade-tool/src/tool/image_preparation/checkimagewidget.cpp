// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QVersionNumber>

#include <thread>
#include <iostream>
#include <cstdio>

#include "checkimagewidget.h"
#include "../../core/utils.h"
#include "../../core/constants.h"
#include "../../core/dbusworker.h"


CheckImageWidget::CheckImageWidget(QWidget *parent)
    : QWidget(parent)
    , m_canceled(false)
    , m_mainLayout(new QVBoxLayout(this))
    , m_spinnerWidget(new SpinnerWidget(this))
{
    initUI();
    initConnections();
}

void CheckImageWidget::initUI()
{
    m_spinnerWidget->setMinimumSize(100, 100);
    m_spinnerWidget->setMessage(tr("Verifying the integrity…"));
    m_mainLayout->addWidget(m_spinnerWidget, 0, Qt::AlignCenter);
}

void CheckImageWidget::initConnections()
{
    connect(this, &CheckImageWidget::Cancel, &CheckImageWidget::onCanceled);
    connect(this, &CheckImageWidget::CheckImported, this, &CheckImageWidget::onCheckImported);
    connect(this, &CheckImageWidget::CheckDownloaded, this, &CheckImageWidget::onCheckDownloaded);
    connect(DBusWorker::getInstance(), &DBusWorker::CheckResult, [this] (bool passed) {
        // 因为后端被杀掉时也会发信号，所以取消时不处理
        if (!m_canceled) {
            emit CheckDone(passed);
        }
    });
    connect(this, &CheckImageWidget::CheckDone, this, [this](bool state) {
        qInfo() << "check sum result:" << state;
        m_spinnerWidget->stop();
        // Handle download failure
        if (!state)
        {
            SourceInfo info = DBusWorker::getInstance()->getSource();
            const QString isoName = QUrl(info.addr).fileName();
            QDir downloadDir = QDir::home();
            downloadDir.mkpath(kDownloadPath);
            downloadDir.cd(kDownloadPath);
            const QString downloadedIsoPath = downloadDir.filePath(isoName);

            if (downloadedIsoPath == m_isoPath)
            {
                QFile(m_isoPath).remove();
            }
        }
    });
}

void CheckImageWidget::onCanceled()
{
    m_canceled = true;
    m_spinnerWidget->stop();
    DBusWorker::getInstance()->StopUpgrade();
}

void CheckImageWidget::onCheckImported(const QString path)
{
    m_canceled = false;
    m_isoPath = path;

    m_spinnerWidget->start();
    DBusWorker::getInstance()->CheckISO(path);
}

void CheckImageWidget::onCheckDownloaded()
{
    m_canceled = false;
    m_spinnerWidget->start();
    SourceInfo info = DBusWorker::getInstance()->getSource();
    const QString isoName = QUrl(info.addr).fileName();
    QDir downloadDir = QDir::home();
    downloadDir.mkpath(kDownloadPath);
    downloadDir.cd(kDownloadPath);
    const QString isoPath = downloadDir.filePath(isoName);
    emit CheckImported(isoPath);
}
