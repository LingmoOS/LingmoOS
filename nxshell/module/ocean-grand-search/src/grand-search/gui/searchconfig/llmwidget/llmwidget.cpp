// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "llmwidget.h"
#include "downloader.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QtConcurrent>
#include <QStandardPaths>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DMenu>
#include <DToolButton>
#include <DDialog>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

//static constexpr int TIMERBEGIN = 720;//5秒轮询安装和更新状态60分钟

static constexpr char MODELNAME[] = "yourong1.5B-Instruct-GGUF";

LLMWidget::LLMWidget(DWidget *parent)
    : DWidget(parent),
      m_pProcess(new QProcess)
{
    initUI();
    initConnect();
    installEventFilter(this);
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    m_installPath = homePath + "/.local/share/lingmo-modelhub/models";
    m_baseUrl = "https://www.modelscope.cn/models/uniontech-yourong";
    m_modelFileList << "/modelhub/config.json"
                    << "/modelhub/LICENSE"
                    << "/modelhub/template"
                    << "/modelhub/template_dsl"
                    << "/modelhub/template_func"
                    << "/yourong_1.5B_bf16_Q4_K_M.gguf";
}

LLMWidget::~LLMWidget()
{
    if (m_pProcess) {
        m_pProcess->terminate();
        m_pProcess->deleteLater();
    }
}

void LLMWidget::initUI()
{
    m_pLabelTheme = new DLabel;
    DFontSizeManager::instance()->bind(m_pLabelTheme, DFontSizeManager::T6, QFont::Medium);
    m_pLabelTheme->setElideMode(Qt::ElideRight);

    m_spinner = new DSpinner(this);
    m_spinner->setFixedSize(12, 12);
    m_spinner->hide();

    m_pLabelStatus = new DLabel(tr("NotInstalled"));
    m_pLabelStatus->setForegroundRole(QPalette::Text);
    DFontSizeManager::instance()->bind(m_pLabelStatus, DFontSizeManager::T8, QFont::Medium);

    auto topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(m_pLabelTheme, 0, Qt::AlignLeft);
    topLayout->addStretch();
    topLayout->addWidget(m_spinner, 0, Qt::AlignRight);
    topLayout->addWidget(m_pLabelStatus, 0, Qt::AlignRight);

    m_pLabelSummary = new DLabel;
    m_pLabelSummary->setForegroundRole(QPalette::Text);
    DFontSizeManager::instance()->bind(m_pLabelSummary, DFontSizeManager::T8, QFont::Normal);
    m_pLabelSummary->setElideMode(Qt::ElideRight);

    m_pManageModel = new ModelManageButton(tr("Install Model"), this);
    QPixmap pixmap = QApplication::style()->standardIcon(QStyle::SP_ArrowDown).pixmap(QSize(10, 10));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), DGuiApplicationHelper::instance()->applicationPalette().textTips());
    m_pManageModel->setIcon(pixmap);
    m_pManageModel->setProperty("modelStatus", Uninstall);
    DFontSizeManager::instance()->bind(m_pManageModel, DFontSizeManager::T8, QFont::Medium);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(10, 8, 10, 6);
    mainLayout->setSpacing(5);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_pLabelSummary, 0, Qt::AlignLeft);
    mainLayout->addWidget(m_pManageModel, 0, Qt::AlignLeft);

    m_updateAction = new QAction(tr("Update model"));
    m_uninstallAction = new QAction(tr("Uninstall model"));

    m_pMenu = new DMenu(this);
    m_pMenu->setMinimumWidth(92);
    m_pMenu->installEventFilter(this);
    m_pMenu->addAction(m_updateAction);
    m_pMenu->addAction(m_uninstallAction);

    setLayout(mainLayout);
}

void LLMWidget::initConnect()
{
    connect(m_pManageModel, &DCommandLinkButton::clicked, this, &LLMWidget::onClickedStatusBtn);
    connect(m_pMenu, &QMenu::triggered, this, &LLMWidget::onMoreMenuTriggered);

    connect(m_pProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));

}

void LLMWidget::paintEvent(QPaintEvent* e)
{
    DPalette pl = m_pLabelSummary->palette();
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().color(DPalette::BrightText);
    if (!isActiveWindow())
        color.setAlphaF(0.4);
    else
        color.setAlphaF(0.6);
    pl.setColor(QPalette::Text, color);
    m_pLabelSummary->setPalette(pl);

    pl = m_pLabelStatus->palette();
    if (m_pManageModel->property("modelStatus").toInt() == Install) {
        color = QColor(21, 190, 76);
        color.setAlphaF(1);
    } else
        color.setAlphaF(0.6);

    if (!isActiveWindow())
        color.setAlphaF(0.4);

    pl.setColor(QPalette::Text, color);
    m_pLabelStatus->setPalette(pl);

    DWidget::paintEvent(e);
}

void LLMWidget::onClickedStatusBtn()
{
    int modelStatus = m_pManageModel->property("modelStatus").toInt();
    switch (modelStatus) {
    case Install:
        onUninstall();
        break;
    case Uninstall: {
        m_pManageModel->setText(tr("Installing"));
        m_pManageModel->setEnabled(false);
        m_pManageModel->updateRectSize();
        m_spinner->show();
        m_spinner->start();
        onInstall();
        break;
    }
    case InstallAndUpdate:
        m_pMenu->exec(m_pManageModel->mapToGlobal(m_pManageModel->rect().bottomLeft() + QPoint(0, 5)));
        break;
    default:
        break;
    }
}

void LLMWidget::onMoreMenuTriggered(const QAction *action)
{
    if (action == m_uninstallAction) {
        onUninstall();
    } else {
//        onInstall();//更新模型也打开应用商店的下载界面去更新
    }
}

void LLMWidget::onInstall()
{
    //git下载
//    if (!m_pProcess->atEnd()) return;
//    if (m_pProcess->state() == QProcess::Running)
//        m_pProcess->waitForFinished();

//    QString program = "git";
//    QStringList arguments;
//    arguments << "clone" << "https://www.modelscope.cn/uniontech-yourong/yourong1.5B-Instruct-GGUF.git";

//    m_pProcess->setWorkingDirectory(m_installPath);
//    m_pProcess->start(program, arguments);

//    onDealInstalledModel();

    //http请求下载
    QDir destinationDir(m_installPath + "/." + MODELNAME);

    if (destinationDir.mkpath("gguf")) {
        qInfo() << "Directory created successfully:";
    } else {
        qWarning() << "Failed to create directory:" ;
        return;
    }

    destinationDir.cd("gguf");

    m_downloader.reset(new Downloader(destinationDir.absolutePath()));
    connect(m_downloader.data(), &Downloader::downloadFinished, this, &LLMWidget::onDownloadFinished);
    connect(m_downloader.data(), &Downloader::onDownloadProgress, this, &LLMWidget::onDownloadProgress);

    foreach (const QString &fileUrl, m_modelFileList) {
        QString url = m_baseUrl + "/" + MODELNAME + "/resolve/master" + fileUrl;
        m_downloader->addDownloadTask(QUrl(url));
    }
}

void LLMWidget::onUninstall()
{
    DDialog dlg(this);
    dlg.setIcon(QIcon(":icons/ocean-grand-search-setting.svg"));
    dlg.setMaximumWidth(380);
    dlg.setTitle(tr("Are you sure you want to delete this model?"));
    dlg.setMessage(tr("After uninstallation, functions such as AI Search and UOS AI Assistant will not work properly."));
    dlg.addButton(tr("Cancel", "button"), false, DDialog::ButtonNormal);
    dlg.addButton(tr("Confirm", "button"), true, DDialog::ButtonWarning);
    auto labelList = dlg.findChildren<QLabel *>();
    for (auto messageLabel : labelList) {
        if ("MessageLabel" == messageLabel->objectName())
            messageLabel->setFixedWidth(dlg.width() - 20);
    }
    if (DDialog::Accepted == dlg.exec()) {
        QDir installFile(m_installPath + "/YouRong-1.5B");

        if (installFile.exists()) {
            if (installFile.removeRecursively())
                qInfo() << "Directory removed successfully.";
            else
                qWarning() << "Failed to remove directory.";
        }
        checkInstallStatus();
    }
}

void LLMWidget::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (process) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            qInfo() << "Git clone successful.";
            if (onDealInstalledModel())
                checkInstallStatus();
        } else {
            qWarning() << "Git clone failed.";
        }
    }
}

bool LLMWidget::onDealInstalledModel()
{
    QDir installFile(m_installPath + MODELNAME);

    if (installFile.exists()) {
        QDir sourceDir(m_installPath + MODELNAME + "/modelhub");
        if (!sourceDir.exists()) {
            qWarning() << "Source directory does not exist.";
            return false;
        }

        QDir destinationDir(m_installPath + MODELNAME);

        if (destinationDir.mkpath("gguf")) {
            qInfo() << "Directory created successfully:";
        } else {
            qWarning() << "Failed to create directory:" ;
            return false;
        }

        destinationDir.cd("gguf");

        QStringList files = sourceDir.entryList(QDir::Files);
        for (const QString &fileName : files) {
            QString sourceFilePath = sourceDir.absoluteFilePath(fileName);
            QString destinationFilePath = destinationDir.absoluteFilePath(fileName);
            if (!QFile::rename(sourceFilePath, destinationFilePath)) {
                qWarning() << "Failed to move file" << sourceFilePath << "to" << destinationFilePath;
                return false;
            }
        }

        auto filesInfo = installFile.entryInfoList(QDir::Files);
        for (const QFileInfo &info : filesInfo) {
            if (info.suffix() == "gguf") {
                QString sourceFilePath = info.absoluteFilePath();
                if (!QFile::rename(sourceFilePath, destinationDir.absoluteFilePath(info.fileName()))) {
                    qWarning() << "Failed to move file:";
                    return false;
                }
            }
        }
    }
    return true;
}

void LLMWidget::onDownloadFinished()
{
    QString originalFolderPath = m_installPath + "/." + MODELNAME;
    QString targetFolderPath = m_installPath + "/YouRong-1.5B";

    QDir dir(originalFolderPath);

    if (dir.exists()) {
        if (dir.rename(originalFolderPath, targetFolderPath)) {
            qDebug() << "File downloaded successfully";
        } else {
            qDebug() << "The folder failed to download.";
        }
    }

    checkInstallStatus();
}

void LLMWidget::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal == 0)
        return;

    double progress = static_cast<double>(bytesReceived) / static_cast<double>(bytesTotal) * 100.0;
    progress = round(progress * 100) / 100;

    if (progress >= m_lastProgress) {
        m_pLabelStatus->setText(tr("Installing ") + QString::number(progress, 'f', 1) + "%");
        m_lastProgress = progress;
    }
}

void LLMWidget::checkInstallStatus()
{
    QDir installFile(m_installPath + "/YouRong-1.5B");

    m_pManageModel->setProperty("modelStatus", installFile.exists() ? Install : Uninstall);

    changeInstallStatus();
}

bool LLMWidget::onCloseEvent()
{
    if (m_pProcess->state() == QProcess::Running || (m_downloader && !m_downloader->isFinished())) {
        DDialog dlg(this);
        dlg.setIcon(QIcon(":icons/ocean-grand-search-setting.svg"));
        dlg.setMaximumWidth(380);
        dlg.setTitle(tr("Installing the UOS AI Large Language Model"));
        dlg.setMessage(tr("Exiting will cause the installation to fail, do you still want to exit?"));
        dlg.addButton(tr("Exit", "button"), false, DDialog::ButtonNormal);
        dlg.addButton(tr("Continue", "button"), true, DDialog::ButtonRecommend);
        auto labelList = dlg.findChildren<QLabel *>();
        for (auto messageLabel : labelList) {
            if ("MessageLabel" == messageLabel->objectName())
                messageLabel->setFixedWidth(dlg.width() - 20);
        }
        if (DDialog::Rejected != dlg.exec())
            return false;

        if (Uninstall == m_pManageModel->property("modelStatus").toInt()) {
            m_downloader->cancelDownloads();
            QDir installFile(m_installPath + "/." + MODELNAME);

            if (installFile.exists()) {
                if (installFile.removeRecursively())
                    qInfo() << "Directory removed successfully.";
                else
                    qWarning() << "Failed to remove directory.";
            }
            checkInstallStatus();
        }
    }

    return true;

}

void LLMWidget::changeInstallStatus()
{
    int modelStatus = m_pManageModel->property("modelStatus").toInt();
    switch (modelStatus) {
    case Install: {
        m_pManageModel->setText(tr("UnInstall Model"));
        m_pLabelStatus->setText(tr("Installed"));
        break;
        }
    case Uninstall: {
        m_pManageModel->setText(tr("Install Model"));
        m_pLabelStatus->setText(tr("Not Installed"));
        break;
        }
    default:
        break;
    }
    m_spinner->hide();
    m_spinner->stop();
    m_pManageModel->setEnabled(true);
    m_pManageModel->updateRectSize();
}

void LLMWidget::setText(const QString &theme, const QString &summary)
{
    m_pLabelTheme->setText(theme);
    m_pLabelSummary->setText(summary);
}
