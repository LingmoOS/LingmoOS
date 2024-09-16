// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DetectedStatusWidget.h"
#include "commontools.h"
#include "BtnLabel.h"
#include "DDLog.h"

#include <DStyle>
#include <DCommandLinkButton>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DApplication>
#include <DDesktopServices>

#include <QString>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QThread>
#include <QSizePolicy>
#include <QPainterPath>
#include <QLoggingCategory>

#define MARGIN_LR          20
#define MARGIN_TB          12
#define ICON_BT_SIZE       36
#define ICON_LABEL_SIZE    96
#define INIT_HEIGHT        120
#define FONT_WEIGHT        63
#define PROGRESS_HEIGHT    8
#define PROGRESS_WIDTH     569
#define BUTTON_WIDTH       120
#define SPACE_15           15
#define SPACE_10           10
#define SPACE_9            9
#define SPACE_5            5

#ifdef OS_BUILD_V23
const QString SERVICE_NAME = "org.deepin.dde.ShutdownFront1";
const QString DEVICE_SERVICE_PATH = "/org/deepin/dde/ShutdownFront1";
const QString DEVICE_SERVICE_INTERFACE = "org.deepin.dde.ShutdownFront1";
#else
const QString SERVICE_NAME = "com.deepin.dde.shutdownFront";
const QString DEVICE_SERVICE_PATH = "/com/deepin/dde/shutdownFront";
const QString DEVICE_SERVICE_INTERFACE = "com.deepin.dde.shutdownFront";
#endif

using namespace DDLog;

DetectedStatusWidget::DetectedStatusWidget(QWidget *parent)
    : DFrame(parent)
    , mp_PicLabel(new DLabel(this))
    , mp_UpdateLabel(new TitleLabel(this))
    , mp_TimeLabel(new DLabel(this))
    , mp_ModelLabel(new TipsLabel(this))
    , mp_RebootLabel(new DLabel(this))
    , mp_FeedBackLabel(new DLabel(this))
    , mp_BackupPathLabel(new DLabel(this))
    , mp_InstallButton(new DSuggestButton(this))
    , mp_ReDetectedSgButton(new DSuggestButton(this))
    , mp_BackupSgButton(new DSuggestButton(this))
    , mp_CancelButton(new DPushButton(this))
    , mp_ReDetectedIconButton(new DIconButton(this))
    , mp_Progress(new DProgressBar(this))
    , mp_HLayoutTotal(nullptr)
    , mp_HLayoutButton(nullptr)
    , mp_VLayoutLabel(nullptr)
    , mp_HLayoutLabel(nullptr)
{
    initUI();
    initConnect();
}

DetectedStatusWidget::~DetectedStatusWidget()
{
    DELETE_PTR(mp_HLayoutButton);
    DELETE_PTR(mp_VLayoutLabel);
    DELETE_PTR(mp_HLayoutTotal);

    if (mp_HLayoutLabel) {
        mp_HLayoutLabel->deleteLater();
        mp_HLayoutLabel = nullptr;
    }
}

void DetectedStatusWidget::setDetectFinishUI(const QString &size, const QString &model, bool hasInstall)
{
    hideAll();
    // Icon Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/update.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    // 发现5个驱动可安装更新  和  机器类型
    QString updateStr = QObject::tr("%1 driver updates available").arg(size);
    mp_UpdateLabel->setText(updateStr);
    mp_UpdateLabel->setElideMode(Qt::ElideRight);
    mp_ModelLabel->setText(model);
    mp_ModelLabel->setElideMode(Qt::ElideRight);
    mp_ModelLabel->setToolTip(model);
    mp_TimeLabel->setText(QObject::tr("Time checked: %1").arg(CommonTools::getSystemTime()));
    mp_TimeLabel->setElideMode(Qt::ElideRight);

    // 布局
    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_VLayoutLabel->addWidget(mp_TimeLabel);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_VLayoutLabel->addWidget(mp_ModelLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addSpacing(SPACE_15);
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);
    mp_HLayoutTotal->addSpacing(SPACE_10);

    // icon button 和 一键安装
    if (hasInstall) {
        mp_InstallButton->setEnabled(true);
    } else {
        mp_InstallButton->setEnabled(false);
    }

    mp_HLayoutButton->addStretch();
    mp_HLayoutButton->addWidget(mp_ReDetectedIconButton);
    mp_HLayoutButton->addSpacing(SPACE_10);
    mp_HLayoutButton->addWidget(mp_InstallButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    // 显示控件
    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_TimeLabel->show();
    mp_ModelLabel->show();
    mp_ReDetectedIconButton->show();
    mp_InstallButton->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setDownloadUI(const DriverType &driverType, const QString &speed
                                         , const QString &downloadSize, const QString &totalSize, int progressValue)
{
    hideAll();
    // Icon Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/update.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    // 下载驱动  和  机器类型
    QString ts = DApplication::translate("QObject", CommonTools::getDriverType(driverType).toStdString().data());
    QString downloadStr = QObject::tr("Downloading drivers for %1...").arg(ts);
    mp_UpdateLabel->setText(downloadStr);
    QString speedStr = QObject::tr("Download speed: %1 Downloaded %2/%3")
                       .arg(speed)
                       .arg(downloadSize)
                       .arg(totalSize);
    mp_ModelLabel->setText(speedStr);
    mp_Progress->setValue(progressValue);

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(SPACE_15);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_10);
    mp_VLayoutLabel->addWidget(mp_Progress);
    mp_VLayoutLabel->addSpacing(SPACE_10);
    mp_VLayoutLabel->addWidget(mp_ModelLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    // 取消
    mp_HLayoutTotal->addSpacing(100);
    if (progressValue > 90)
        mp_CancelButton->setEnabled(false);
    else
        mp_CancelButton->setEnabled(true);
    mp_HLayoutButton->addWidget(mp_CancelButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    // 显示控件
    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_ModelLabel->show();
    mp_Progress->show();
    mp_CancelButton->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setInstallUI(const DriverType &driverType, const QString &driverDescription, int progressValue)
{
    hideAll();
    // Icon Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/update.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    // 安装驱动  和  驱动描述
    QString ts = DApplication::translate("QObject", CommonTools::getDriverType(driverType).toStdString().data());
    QString installStr = QObject::tr("Installing drivers for %1...").arg(ts);
    mp_UpdateLabel->setText(installStr);
    mp_ModelLabel->setText(driverDescription);
    mp_Progress->setValue(progressValue);

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(SPACE_15);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_10);
    mp_VLayoutLabel->addWidget(mp_Progress);
    mp_VLayoutLabel->addSpacing(SPACE_10);
    mp_VLayoutLabel->addWidget(mp_ModelLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    // 取消
    mp_HLayoutButton->addSpacing(100);
    mp_CancelButton->setEnabled(false);
    mp_HLayoutButton->addWidget(mp_CancelButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    // 显示控件
    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_ModelLabel->show();
    mp_Progress->show();
    mp_CancelButton->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setInstallSuccessUI(const QString &success, const QString &failed)
{
    hideAll();
    // Icon Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/latest.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    // 成功失败个数  和  重启
    QString successStr;
    if (failed.toInt() > 0) {
        successStr = QObject::tr("%1 drivers installed, %2 drivers failed").arg(success).arg(failed);
    } else {
        successStr = QObject::tr("%1 drivers installed").arg(success);
    }

    mp_UpdateLabel->setText(successStr);

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(11);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_9);
    mp_VLayoutLabel->addWidget(mp_RebootLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    // icon button 和一键安装
//    mp_InstallButton->setEnabled(false);
    mp_HLayoutButton->addStretch();
    mp_HLayoutButton->addWidget(mp_ReDetectedIconButton);
    mp_HLayoutButton->addSpacing(SPACE_10);
    mp_HLayoutButton->addWidget(mp_InstallButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    // 显示控件
    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_RebootLabel->show();
    mp_ReDetectedIconButton->show();
    mp_InstallButton->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setInstallFailedUI()
{
    hideAll();
    // Icon Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/update failed.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    // 驱动安装失败  和  反馈
    QString failedStr = QObject::tr("Failed to install drivers");
    mp_UpdateLabel->setText(failedStr);

    // icon button 和一键安装
    mp_InstallButton->setEnabled(false);

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(SPACE_15);

    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_9);
    mp_VLayoutLabel->addWidget(mp_FeedBackLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    mp_HLayoutButton->addStretch();
    mp_HLayoutButton->addWidget(mp_ReDetectedIconButton);
    mp_HLayoutButton->addSpacing(SPACE_10);
    mp_HLayoutButton->addWidget(mp_InstallButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    // 显示控件
    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_FeedBackLabel->show();
    mp_ReDetectedIconButton->show();
    mp_InstallButton->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setNetworkErrorUI(const QString &speed, int progressValue)
{
    hideAll();
    // Icon Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/erro-96.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    // 网络异常 和 下载速度
    QString networkErrStr = QObject::tr("Network error. Reconnecting...");
    mp_UpdateLabel->setText(networkErrStr);
    QString speedStr = QObject::tr("Download speed: %1")
                       .arg(speed);
    mp_ModelLabel->setText(speedStr);
//    mp_Progress->setValue(progressValue);

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(SPACE_15);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_10);
    mp_VLayoutLabel->addWidget(mp_Progress);
    mp_VLayoutLabel->addSpacing(SPACE_10);
    mp_VLayoutLabel->addWidget(mp_ModelLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    // 取消

    mp_HLayoutTotal->addSpacing(100);
    mp_CancelButton->setEnabled(true);
    mp_HLayoutButton->addWidget(mp_CancelButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    // 显示控件
    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_ModelLabel->show();
    mp_Progress->show();
    mp_CancelButton->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setNoUpdateDriverUI(const QString &model)
{
    hideAll();
    // Icon Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/latest.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    // 发现5个驱动可安装更新  和  机器类型
    QString updateStr = QObject::tr("Your drivers are up to date");
    mp_UpdateLabel->setText(updateStr);
    mp_ModelLabel->setText(model);
    mp_TimeLabel->setText(QObject::tr("Time checked: %1").arg(CommonTools::getSystemTime()));

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(SPACE_15);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_VLayoutLabel->addWidget(mp_TimeLabel);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_VLayoutLabel->addWidget(mp_ModelLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    // 重新检测
    mp_HLayoutButton->addStretch();
    mp_HLayoutButton->addWidget(mp_ReDetectedSgButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    // 显示控件
    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_TimeLabel->show();
    mp_ModelLabel->show();
    mp_ReDetectedSgButton->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::refreshUI(Status pageType)
{
    qCInfo(appLog) << pageType;
    switch (pageType) {
    case ST_NOT_INSTALL: {
        this->setDetectFinishUI("5", "fffffff", true);
        break;
    }
    case ST_DOWNLOADING: {
        this->setDownloadUI(DR_Bluetooth, "7.00MB/s", "28.96MB", "192.78MB", 28.96 * 100 / 192.78);
        break;
    }
    case ST_INSTALL: {
        this->setInstallUI(DR_Bluetooth, "Audio Driver Audio Driver Audio Driver", 75);
        break;
    }
    case ST_SUCESS: {
        this->setInstallSuccessUI("4", "2");
        break;
    }
    case ST_FAILED: {
        this->setInstallFailedUI();
        break;
    }
    case ST_NetWorkErr: {
        this->setNetworkErrorUI("0.00MB/s", 40);
        break;
    }
    case ST_DRIVER_IS_NEW: {
        this->setNoUpdateDriverUI("ddddddddddddddddddddddddddddddd");
        break;
    }
    }
}

void DetectedStatusWidget::setInstallBtnEnable(bool enable)
{
    if (mp_InstallButton) {
        mp_InstallButton->setEnabled(enable);
    }
}

void DetectedStatusWidget::setBackupBtnEnable(bool enable)
{
    if (mp_BackupSgButton) {
        mp_BackupSgButton->setEnabled(enable);
    }
}

void DetectedStatusWidget::setNoBackupDriverUI(int backableSize, int backedupSize)
{
    hideAll();
    // Icon Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/latest.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    int total = backableSize + backedupSize;
    QString updateStr = QObject::tr("All drivers have been backed up");
    mp_UpdateLabel->setText(updateStr);
    mp_ModelLabel->setText(QObject::tr("A total of %1 drivers, of which %2 have been backed up").arg(total).arg(backedupSize));

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(SPACE_15);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_HLayoutLabel->addWidget(mp_ModelLabel);
    mp_HLayoutLabel->addSpacing(SPACE_5);
    mp_HLayoutLabel->addWidget(mp_BackupPathLabel);
    mp_VLayoutLabel->addLayout(mp_HLayoutLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    // 重新检测
    mp_HLayoutButton->addStretch();
    mp_HLayoutButton->addWidget(mp_ReDetectedSgButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    // 显示控件
    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_ModelLabel->show();
    mp_ReDetectedSgButton->show();
    mp_BackupPathLabel->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setBackableDriverUI(int backableSize, int backedupSize)
{
    hideAll();

    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/backup_96.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    int total = backableSize + backedupSize;
    if (backedupSize <= 0) {
        mp_UpdateLabel->setText(QObject::tr("You have %1 drivers that can be backed up, it is recommended to do so immediately").arg(backableSize));
        mp_ModelLabel->setText("");
    } else {
        mp_UpdateLabel->setText(QObject::tr("You have %1 drivers that can be backed up").arg(backableSize));
        mp_ModelLabel->setText(QObject::tr("A total of %1 drivers, of which %2 have been backed up").arg(total).arg(backedupSize));
    }

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(SPACE_15);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_HLayoutLabel->addWidget(mp_ModelLabel);
    if (backedupSize > 0) {
        mp_HLayoutLabel->addSpacing(SPACE_5);
        mp_HLayoutLabel->addWidget(mp_BackupPathLabel);
    }
    mp_VLayoutLabel->addLayout(mp_HLayoutLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    mp_HLayoutButton->addStretch();
    mp_HLayoutButton->addWidget(mp_ReDetectedIconButton);
    mp_HLayoutButton->addSpacing(SPACE_10);
    mp_HLayoutButton->addWidget(mp_BackupSgButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    mp_BackupSgButton->setEnabled(false);

    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_ModelLabel->show();
    mp_ReDetectedIconButton->show();
    mp_BackupSgButton->show();
    if (backedupSize > 0) {
        mp_BackupPathLabel->show();
    }
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setBackingUpDriverUI(const QString &driverDescription, int totalValue, int progressValue)
{
    hideAll();

    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/backup_96.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    mp_UpdateLabel->setText(QObject::tr("Backing up the %1 driver, a total of %2 drivers").arg(progressValue).arg(totalValue));
    mp_ModelLabel->setText(QObject::tr("Backing up: %1").arg(driverDescription));
    mp_Progress->setValue(progressValue * 10);

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(SPACE_15);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_VLayoutLabel->addWidget(mp_Progress);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_VLayoutLabel->addWidget(mp_ModelLabel);
    mp_VLayoutLabel->addStretch();

    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    // 取消
    mp_HLayoutButton->addStretch();
    mp_HLayoutButton->addWidget(mp_CancelButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_ModelLabel->show();
    mp_Progress->show();
    mp_CancelButton->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setBackupSuccessUI(int success, int failed)
{
    hideAll();
    // Icon Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/backup_96.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    QString successStr;
    if (failed > 0) {
        successStr = QObject::tr("%1 drivers backed up, %2 drivers failed").arg(success).arg(failed);
    } else if (success <= 0) {
        successStr = QObject::tr("Failed to backup drivers");
    } else {
        successStr = QObject::tr("%1 drivers backed up").arg(success);
    }

    mp_UpdateLabel->setText(successStr);

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(11);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
//    mp_VLayoutLabel->addSpacing(SPACE_9);
//    mp_VLayoutLabel->addWidget(mp_RebootLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);


//    mp_BackupSgButton->setEnabled(false);
    mp_HLayoutButton->addStretch();
    mp_HLayoutButton->addWidget(mp_ReDetectedIconButton);
    mp_HLayoutButton->addSpacing(SPACE_10);
    mp_HLayoutButton->addWidget(mp_BackupSgButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    // 显示控件
    mp_PicLabel->show();
    mp_UpdateLabel->show();
//    mp_RebootLabel->show();
    mp_ReDetectedIconButton->show();
    mp_BackupSgButton->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setRestoreDriverUI(int restorableSize)
{
    hideAll();

    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/restore_96.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    mp_UpdateLabel->setText(QObject::tr("You have %1 drivers that can be restored").arg(restorableSize));
    mp_ModelLabel->setText(QObject::tr("Please select a driver to restore"));

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(SPACE_15);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_VLayoutLabel->addWidget(mp_ModelLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    mp_HLayoutButton->addStretch();
    mp_HLayoutButton->addWidget(mp_ReDetectedSgButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_ModelLabel->show();
    mp_ReDetectedSgButton->show();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setRestoringUI(int progressValue, QString driverDescription)
{
    hideAll();

    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/restore_96.svg"));
    QPixmap pic = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_PicLabel->setPixmap(pic);

    mp_UpdateLabel->setText(QObject::tr("Driver is restoring..."));
    mp_ModelLabel->setText(QObject::tr("Restoring: %1").arg(driverDescription));
    mp_Progress->setValue(progressValue);

    mp_HLayoutTotal->addWidget(mp_PicLabel);
    mp_HLayoutTotal->addSpacing(SPACE_15);
    mp_VLayoutLabel->addStretch();
    mp_VLayoutLabel->addWidget(mp_UpdateLabel);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_VLayoutLabel->addWidget(mp_Progress);
    mp_VLayoutLabel->addSpacing(SPACE_5);
    mp_VLayoutLabel->addWidget(mp_ModelLabel);
    mp_VLayoutLabel->addStretch();
    mp_HLayoutTotal->addLayout(mp_VLayoutLabel);

    mp_HLayoutButton->addStretch();
    mp_HLayoutButton->addWidget(mp_ReDetectedSgButton);
    mp_HLayoutTotal->addLayout(mp_HLayoutButton);

    mp_PicLabel->show();
    mp_UpdateLabel->show();
    mp_ModelLabel->show();
    mp_Progress->show();
    mp_ReDetectedSgButton->hide();
    this->setLayout(mp_HLayoutTotal);
}

void DetectedStatusWidget::setReDetectEnable(bool enable)
{
    if (mp_ReDetectedSgButton)
        mp_ReDetectedSgButton->setEnabled(enable);
    if (mp_ReDetectedIconButton)
        mp_ReDetectedIconButton->setEnabled(enable);
}

void DetectedStatusWidget::slotReboot()
{
    // 调用DBus接口重启
    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    QDBusInterface *iface = new QDBusInterface(SERVICE_NAME, DEVICE_SERVICE_PATH, DEVICE_SERVICE_INTERFACE, QDBusConnection::sessionBus());
    iface->call("Restart");
}

void DetectedStatusWidget::slotFeedBack()
{
    CommonTools::feedback();
}

void DetectedStatusWidget::slotBackupPath()
{
    Dtk::Widget::DDesktopServices::showFolder(CommonTools::getBackupPath() + "driver");
}


void DetectedStatusWidget::slotInstall()
{
    emit installAll();
}

void DetectedStatusWidget::slotBackup()
{
    emit backupAll();
}

void DetectedStatusWidget::slotReDetectSlot()
{
    emit redetected();
}

void DetectedStatusWidget::slotCancel()
{
    emit cancelClicked();
}

void DetectedStatusWidget::onUpdateTheme()
{
    // 根据主题设置字体颜色
    DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    plt.setColor(DPalette::Text, plt.color(DPalette::TextTitle));
    mp_UpdateLabel->setPalette(plt);
    plt.setColor(DPalette::Text, plt.color(DPalette::TextTips));
    mp_ModelLabel->setPalette(plt);
}

void DetectedStatusWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();;
    QPainterPath path;
    path.addRoundedRect(rect, 8, 8);
    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid /* && wid == this*/)
        cg = DPalette::Active;
    else
        cg = DPalette::Inactive;

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    QBrush bgBrush(palette.color(cg, DPalette::ItemBackground));
    painter.fillPath(path, bgBrush);

    painter.restore();

    DFrame::paintEvent(event);
}

void DetectedStatusWidget::initUI()
{
    hideAll();

    // 无边框直角
    this->setFixedHeight(INIT_HEIGHT);
    this->setFrameRounded(true);
    this->setFrameShape(NoFrame);

    // 发现6个驱动可安装更新
    QFont font = mp_UpdateLabel->font();
    font.setWeight(FONT_WEIGHT);
    mp_UpdateLabel->setFont(font);
    mp_UpdateLabel->setWordWrap(true);
    DFontSizeManager::instance()->bind(mp_UpdateLabel, DFontSizeManager::T5);

    // 检测时间
    DFontSizeManager::instance()->bind(mp_TimeLabel, DFontSizeManager::T8);

    // 机器型号
    DFontSizeManager::instance()->bind(mp_ModelLabel, DFontSizeManager::T9);

    // 重启Label
    QString rebootStr = QString("<a style=\"text-decoration:none\" href=\"reboot\">") + QObject::tr("reboot") + "</a>";
    mp_RebootLabel->setText(QObject::tr("Please %1 for the installed drivers to take effect").arg(rebootStr));

    // 反馈Label
    QString feedbackStr = QString("<a style=\"text-decoration:none\" href=\"submit feedback\">") + QObject::tr("submit feedback") + "</a>";
    mp_FeedBackLabel->setText(QObject::tr("Please try again or %1 to us").arg(feedbackStr));

    // 备份路径
    DFontSizeManager::instance()->bind(mp_BackupPathLabel, DFontSizeManager::T9);
    mp_BackupPathLabel->setText(QString("<a style=\"text-decoration:none\" href=\"backup-path\">") + QObject::tr("View backup path") + "</a>");

    // 一键安装
    mp_InstallButton->setText(QObject::tr("Install All"));
    mp_InstallButton->setFixedWidth(BUTTON_WIDTH);
    mp_InstallButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 重新检测 DSuggestButton
    mp_ReDetectedSgButton->setText(QObject::tr("Scan Again"));
    mp_ReDetectedSgButton->setFixedWidth(BUTTON_WIDTH);
    mp_ReDetectedSgButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 一键备份
    mp_BackupSgButton->setText(QObject::tr("Backup All"));
    mp_BackupSgButton->setFixedWidth(BUTTON_WIDTH);
    mp_BackupSgButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 取消
    mp_CancelButton->setText(QObject::tr("Cancel"));
    mp_CancelButton->setMinimumWidth(BUTTON_WIDTH);

    // 重新检测 IconButton
    mp_ReDetectedIconButton->setIcon(QIcon::fromTheme("retest"));
    mp_ReDetectedIconButton->setIconSize(QSize(ICON_BT_SIZE, ICON_BT_SIZE));
    mp_ReDetectedIconButton->setFixedSize(ICON_BT_SIZE, ICON_BT_SIZE);

    // 进度条
    mp_Progress->setFixedHeight(PROGRESS_HEIGHT);
    mp_Progress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);   // 长度随窗口大小变化
}

void DetectedStatusWidget::initConnect()
{
    // 重启
    connect(mp_RebootLabel, &QLabel::linkActivated, this, &DetectedStatusWidget::slotReboot);

    // 反馈
    connect(mp_FeedBackLabel, &QLabel::linkActivated, this, &DetectedStatusWidget::slotFeedBack);

    // 备份路径
    connect(mp_BackupPathLabel, &QLabel::linkActivated, this, &DetectedStatusWidget::slotBackupPath);

    // 一键安装
    connect(mp_InstallButton, &DSuggestButton::clicked, this, &DetectedStatusWidget::slotInstall);

    // 一键备份
    connect(mp_BackupSgButton, &DSuggestButton::clicked, this, &DetectedStatusWidget::slotBackup);

    // redetected button
    connect(mp_ReDetectedIconButton, &DIconButton::clicked, this, &DetectedStatusWidget::slotReDetectSlot);
    connect(mp_ReDetectedSgButton, &DIconButton::clicked, this, &DetectedStatusWidget::slotReDetectSlot);

    // 取消
    connect(mp_CancelButton, &DPushButton::clicked, this, &DetectedStatusWidget::slotCancel);

    // 主题变化
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &DetectedStatusWidget::onUpdateTheme);
}

void DetectedStatusWidget::hideAll()
{
    mp_PicLabel->hide();
    mp_UpdateLabel->hide();
    mp_TimeLabel->hide();
    mp_ModelLabel->hide();
    mp_RebootLabel->hide();
    mp_FeedBackLabel->hide();
    mp_BackupPathLabel->hide();
    mp_InstallButton->hide();
    mp_BackupSgButton->hide();
    mp_ReDetectedSgButton->hide();
    mp_CancelButton->hide();
    mp_ReDetectedIconButton->hide();
    mp_Progress->hide();

    delete this->layout();
    mp_HLayoutTotal = new QHBoxLayout();
    mp_HLayoutButton = new QHBoxLayout();
    mp_VLayoutLabel = new QVBoxLayout();
    mp_HLayoutLabel = new QHBoxLayout();
    mp_HLayoutTotal->setContentsMargins(MARGIN_LR, MARGIN_TB, MARGIN_LR, MARGIN_TB);
    mp_HLayoutTotal->setSpacing(0);
    mp_HLayoutButton->setSpacing(0);
    mp_VLayoutLabel->setSpacing(0);
    mp_HLayoutLabel->setSpacing(0);
}
