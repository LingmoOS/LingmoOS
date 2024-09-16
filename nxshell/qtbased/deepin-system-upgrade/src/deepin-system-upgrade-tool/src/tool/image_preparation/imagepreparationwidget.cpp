// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DDialog>
#include <DFileDialog>
#include <DFloatingMessage>
#include <DMessageManager>

#include <QFileInfo>
#include <QMimeData>
#include <QPixmap>
#include <QButtonGroup>
#include <QStandardPaths>
#include <QStyle>

#include <iostream>

#include "../mainwindow.h"
#include "imagepreparationwidget.h"
#include "../application.h"
#include "../../core/dbusworker.h"
#include "../../widgets/alertdialog.h"

ImagePreparationWidget::ImagePreparationWidget(QWidget *parent)
    : BaseContainerWidget(parent, 1)
    , m_noNetworkMessage(new DFloatingMessage())
    , m_networkConnected(false)
    , m_stackedLayout(new QStackedLayout(this))
    , m_imageMethodWidget(new RetrieveImageMethodWidget(this))
    , m_checkImageWidget(new CheckImageWidget(this))
    , m_checkImageResultWidget(new CheckImageResultWidget(this))
    , m_checkNetworkThread(nullptr)
    , m_imageDownloadWidget(new ImageDownloadWidget(this))
    , m_errorMessageWidget(new ErrorMessageWidget(this))
    , m_softwareCheckProgressWidget(new SoftwareCheckProgressWidget(this))
    , m_softwareTableWidget(new SoftwareEvaluationWidget(this))
    , m_softwareOfflineResultWidget(new SoftwareEvaluationOfflineWidget(this))
{
    setAcceptDrops(true);
    initUI();
    initConnections();

}

void ImagePreparationWidget::initUI()
{
    m_noNetworkMessage->setIcon(QIcon(":icons/p2v_warning_normal.svg"));
    m_noNetworkMessage->setMessage(tr("Failed to connect to the server. Please check your network connection or select a local V23 image file."));

    m_stackedLayout->addWidget(m_imageMethodWidget);
    m_stackedLayout->addWidget(m_checkImageWidget);
    m_stackedLayout->addWidget(m_checkImageResultWidget);
    m_stackedLayout->addWidget(m_imageDownloadWidget);
    m_stackedLayout->addWidget(m_errorMessageWidget);
    m_stackedLayout->addWidget(m_softwareCheckProgressWidget);
    m_stackedLayout->addWidget(m_softwareTableWidget);
    m_stackedLayout->addWidget(m_softwareOfflineResultWidget);
    m_errorMessageWidget->setTitle(tr("Download failed"));

    // Show bottom buttons.
    showAllButtons();

    m_spacerItem->changeSize(0, 0, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    m_contentLayout->addLayout(m_stackedLayout, 10);
}

void ImagePreparationWidget::initConnections()
{
    // Setting bottom button actions.
    connect(m_cancelButton, &DPushButton::clicked, this, [this] {
        if (m_stackedLayout->currentWidget() == m_imageDownloadWidget)
        {
            AlertDialog dlg(this, tr("Do you want to exit the downloading?"));
            if (dlg.exec() == DDialog::Accepted)
            {
                emit m_imageDownloadWidget->abort();
            }
        }
        else if (m_stackedLayout->currentWidget() == m_checkImageWidget)
        {
            AlertDialog dlg(this, tr("Do you want to exit the image file checking?"));
            if (dlg.exec() == DDialog::Accepted)
            {
                emit m_checkImageWidget->Cancel();
                jumpImageMethodWidget();
            }
        }
        else if (m_stackedLayout->currentWidget() == m_softwareCheckProgressWidget)
        {
            AlertDialog dlg(this, tr("Do you want to exit software evaluation?"));
            if (dlg.exec() == DDialog::Accepted)
            {
                DBusWorker::getInstance()->StopUpgrade();
                m_softwareTableWidget->clearAppInfos();
                emit m_checkImageWidget->CheckDone(true);
            }
        }
        else
        {
            MainWindow::getInstance()->close();
        }
    });
    connect(m_suggestButton, &DPushButton::clicked, this, [this] {
        if (m_stackedLayout->currentWidget() == m_imageMethodWidget)
        {
            // Setting button styles beforehand to avoid overwritting button style settings under error situations.
            if (m_imageMethodWidget->isLocalFileSelected())
            {
                // Show bottom buttons.
                setCancelButtonStyle();
                // Integrity check of image
                 m_stackedLayout->setCurrentWidget(m_checkImageWidget);
                 emit m_checkImageWidget->CheckImported(m_imageMethodWidget->getFileUrl());
            }
            else
            {
                if (m_networkConnected)
                {
                    setCancelButtonStyle();
                    SourceInfo info = DBusWorker::getInstance()->getSource();
                    m_stackedLayout->setCurrentWidget(m_imageDownloadWidget);
                    emit m_imageDownloadWidget->download(info.addr);
                }
                else
                {
                    showNoNetworkMessage();
                    checkNetwork();
                }
            }
        }
        // Todo(Yutao Meng): change button styles
        else if (m_stackedLayout->currentWidget() == m_checkImageResultWidget)
        {
            if (m_checkImageResultWidget->getResult())
            {
                // 判断网络，如果网络可用，进入评估阶段，如果不可用，进入提示界面
                if (m_networkConnected)
                {
                    emit DBusWorker::getInstance()->Assess(m_checkImageResultWidget->getResultIsoPath());
                    m_stackedLayout->setCurrentWidget(m_softwareCheckProgressWidget);
                    m_suggestButton->setVisible(false);
                    m_cancelButton->setFixedSize(250, 36);
                    m_cancelButton->setText(tr("Cancel"));
                } else {
                    m_stackedLayout->setCurrentWidget(m_softwareOfflineResultWidget);
                    showAllButtons();
                    m_suggestButton->setText(tr("Start Upgrade"));
                }

            }
            else
            {
                // Retry
                jumpImageMethodWidget();
            }
        }
        else if (m_stackedLayout->currentWidget() == m_softwareTableWidget || m_stackedLayout->currentWidget() == m_softwareOfflineResultWidget )
        {
            // Show warning for laptops which is powered by battery rather than AC power.
            if (DBusWorker::getInstance()->IsOnBattery())
            {
                showPowerWarning();
            }
            else
            {
                DBusWorker::getInstance()->SetMigrateAppsList(m_softwareTableWidget->getMigratelist());
            }
        }
        else if (m_stackedLayout->currentWidget() == m_errorMessageWidget)
        {
            // Retry
            jumpImageMethodWidget();
        }
        else
        {
            qCritical() << "can not handle for widget:"<< m_stackedLayout->currentWidget()->metaObject()->className();
        }
    });
    connect(m_softwareCheckProgressWidget, &SoftwareCheckProgressWidget::CheckDone, [this] {
        m_softwareTableWidget->fillTable();
        m_stackedLayout->setCurrentWidget(m_softwareTableWidget);
        showAllButtons();
        m_suggestButton->setText(tr("Start Upgrade"));
    });
    connect(m_imageMethodWidget, &RetrieveImageMethodWidget::FileAdded, this, [this] () {
        activateWindow();
        m_suggestButton->setEnabled(true);
    });
    connect(m_imageMethodWidget, &RetrieveImageMethodWidget::FileClear, this, [this] () {
        if (m_imageMethodWidget->isLocalFileSelected())
            m_suggestButton->setEnabled(false);
    });
    connect(m_imageMethodWidget, &RetrieveImageMethodWidget::ToggleLocalButton, this, [this] (bool checked) {
        // Only enable "Next" button when a local image is loaded or "download from internet" option is used.
        if (checked)
        {
            if (m_imageMethodWidget->getFileUrl().length() > 0)
            {
                m_suggestButton->setEnabled(true);
            }
            else
            {
                m_suggestButton->setEnabled(false);
            }
        }
        else
        {
            m_suggestButton->setEnabled(true);
        }

    });
    connect(m_checkImageWidget, &CheckImageWidget::CheckDone, this, [this](bool passed) {
        emit m_checkImageResultWidget->SetResult(passed);
        if (m_imageMethodWidget->isLocalFileSelected())
        {
            emit m_checkImageResultWidget->ReadImageInfo(m_imageMethodWidget->getFileUrl());
        }
        else
        {
            emit m_checkImageResultWidget->ReadImageInfo(m_imageDownloadWidget->getFilePath());
        }
        m_stackedLayout->setCurrentWidget(m_checkImageResultWidget);
        m_spacerItem->changeSize(0, 0, QSizePolicy::Preferred, QSizePolicy::Preferred);

        showAllButtons();
        if (passed)
        {
            m_suggestButton->setText(tr("Next"));
        }
        else
        {
            m_suggestButton->setText(tr("Reselect"));
        }
    });
    connect(m_imageDownloadWidget, &ImageDownloadWidget::done, [this] {
        SourceInfo info = DBusWorker::getInstance()->getSource();
        QString checksumUrlStr = QUrl(info.addr).adjusted(QUrl::RemoveFilename).toString() + QString("SHA256SUMS");
        // Image integrity check
        m_stackedLayout->setCurrentWidget(m_checkImageWidget);
        emit m_checkImageWidget->CheckDownloaded();
    });
    connect(m_imageDownloadWidget, &ImageDownloadWidget::error, [this](QNetworkReply::NetworkError err, QString errLog) {
        // Ignore error signal when current view is retrive image method widget.
        if (m_stackedLayout->currentWidget() == m_imageMethodWidget)
            return;

        if (err == QNetworkReply::OperationCanceledError)
        {
            // Ignore download cancelation error, and jump to retrive image method widget directly.
            jumpImageMethodWidget();
            m_suggestButton->setEnabled(true);
            return;
        }
        // Jump to download error view.
        qDebug() << "setting up error widget";
        showAllButtons();
        m_cancelButton->setFixedSize(120, 36);
        m_spacerItem->changeSize(0, 0);

        m_suggestButton->setText(tr("Reselect"));
        m_suggestButton->setVisible(true);

        QStringList errorList;
        errorList << tr("Errors:") << errLog;
        m_errorMessageWidget->setErrorLog(errorList.join(QString("\n")));
        m_stackedLayout->setCurrentWidget(m_errorMessageWidget);
        errorList << tr("Error code:") << QVariant::fromValue(err).toString();
        qCritical() << errorList.join(QString("\n"));
    });
    connect(DBusWorker::getInstance(), &DBusWorker::EvaluateSoftwareError, [this] (const QString errorLog) {
       m_cancelButton->setText(tr("Exit"));
       m_errorMessageWidget->setTitle(tr("Software evaluation failed"));
       m_errorMessageWidget->setErrorLog(errorLog);
       m_stackedLayout->setCurrentWidget(m_errorMessageWidget);
    });
    connect(DBusWorker::getInstance(), &DBusWorker::GetMigrateListDone, [this]() {
        emit StartUpgrade(m_checkImageResultWidget->getResultIsoPath());
    });
}

void ImagePreparationWidget::checkNetwork()
{
    if (m_checkNetworkThread == nullptr || m_checkNetworkThread->isFinished())
    {
        m_checkNetworkThread = new CheckSizeThread(DBusWorker::getInstance()->getSource().addr);
        connect(m_checkNetworkThread, &CheckSizeThread::done, this, [this] {
            m_networkConnected = true;
        });
        connect(m_checkNetworkThread, &CheckSizeThread::error, this, [this] (QString errStr){
            qDebug() << "check network error message:" << errStr;
            m_networkConnected = false;
        });
        m_checkNetworkThread->start();
    }
}

void ImagePreparationWidget::showNoNetworkMessage()
{
    qCritical() << QString("cannot connect to %1").arg(DBusWorker::getInstance()->getSource().addr);
    m_noNetworkMessage->close();
    m_noNetworkMessage->show();
    DMessageManager::instance()->sendMessage(m_imageMethodWidget, m_noNetworkMessage);
}

void ImagePreparationWidget::setCancelButtonStyle()
{
    showAllButtons();
    qDebug() << "setting up check or download widgets";
    m_cancelButton->setFixedSize(250, 36);
    m_cancelButton->setText(tr("Cancel"));
    m_suggestButton->setVisible(false);
}

void ImagePreparationWidget::showAllButtons()
{
    m_cancelButton->setVisible(true);
    m_cancelButton->setText(tr("Exit"));
    m_cancelButton->setFixedSize(120, 36);
    m_suggestButton->setVisible(true);
    m_suggestButton->setText(tr("Next"));
    m_suggestButton->setFixedSize(120, 36);
    m_buttonSpacerItem->changeSize(5, 0);
}

void ImagePreparationWidget::jumpImageMethodWidget()
{
    checkNetwork();
    showAllButtons();
    m_cancelButton->setFixedSize(120, 36);
    m_suggestButton->setEnabled(true);
    m_stackedLayout->setCurrentWidget(m_imageMethodWidget);
}

void ImagePreparationWidget::showPowerWarning() {
    DDialog dlg(this);
    MainWindow *window = MainWindow::getInstance();

    dlg.move(window->x() + (window->width() - dlg.width()) / 2, window->y() + (window->height() - dlg.height()) / 2);
    dlg.setTitle(tr("Your computer is not plugged in, please plug in a power cord first and then upgrade."));
    dlg.addButton(tr("OK"));
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.exec();
}
