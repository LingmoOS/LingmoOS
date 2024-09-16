// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageDriverControl.h"
#include "GetDriverPathWidget.h"
#include "GetDriverNameWidget.h"
#include "DriverWaitingWidget.h"
#include "DBusDriverInterface.h"
#include "DBusAnythingInterface.h"
#include "DBusInterface.h"
#include "drivericonwidget.h"
#include "HttpDriverInterface.h"

#include <DBlurEffectWidget>
#include <DWidget>
#include <DApplicationHelper>
#include <DPalette>
#include <DTitlebar>
#include <DIconButton>

#include <QVBoxLayout>
#include <QDBusConnection>
#include <QWindow>
#include <polkit-qt5-1/PolkitQt1/Authority>

#define EUNKNOW         0
#define E_FILE_NOT_EXISTED 100 // file_not_existed 文件不存在，主要比如点击下一步时拔掉优盘
#define E_NOT_DRIVER      101 // not driver 非驱动文件
#define E_NOT_SIGNED      102 // not signed 没有数字签名

using namespace PolkitQt1;

PageDriverControl::PageDriverControl(QWidget *parent, QString operation, bool install, QString deviceName, QString driverName, QString printerVendor, QString printerModel)
    : DDialog(parent)
    , mp_stackWidget(new DStackedWidget)
    , m_Install(install)
    , m_DriverName(driverName)
    , m_printerVendor(printerVendor)
    , m_printerModel(printerModel)
    , m_deviceName(deviceName)
{
    setObjectName("PageDriverControl");
    setFixedSize(480, 335);
    setOnButtonClickedClose(false);
    setWindowTitle(QString("%1-%2").arg(operation).arg(deviceName));
    setWindowFlag(Qt::WindowMinimizeButtonHint, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setIcon(QIcon::fromTheme("deepin-devicemanager"));
    initErrMsg();

    DBlurEffectWidget *widget = findChild<DBlurEffectWidget *>();
    if (nullptr != widget) {
        widget->lower();
        widget->setBlurEnabled(false);
    }
    setAttribute(Qt::WA_NoSystemBackground, false);

    DWidget *cenWidget = new DWidget;
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(mp_stackWidget);
    cenWidget->setLayout(vLayout);
    if (m_Install) {
        initInstallWidget();
    } else {
        initUninstallWidget();
    }
    addContent(cenWidget);
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processChange, this, &PageDriverControl::slotProcessChange);
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processEnd, this, &PageDriverControl::slotProcessEnd);
}

bool PageDriverControl::isRunning()
{
    for (auto w : qApp->allWindows()) {
        if ("PageDriverControlWindow" == w->objectName()) {
            if (w->isVisible())
                return true;
        }
    }
    return false;
}

void PageDriverControl::initInstallWidget()
{
    // 先添加用户选择驱动文件所在目录路径的界面
    mp_PathDialog = new GetDriverPathWidget(this);
    mp_stackWidget->addWidget(mp_PathDialog);
    // 添加用户选择驱动文件名称的界面
    mp_NameDialog = new GetDriverNameWidget(this);
    mp_stackWidget->addWidget(mp_NameDialog);
    // 添加安装等待界面
    mp_WaitDialog = new DriverWaitingWidget(tr("Updating"), this);
    mp_stackWidget->addWidget(mp_WaitDialog);

    mp_stackWidget->setCurrentIndex(0);
    addContent(mp_stackWidget);
    this->addButton(tr("Cancel", "button"), true);
    this->addButton(tr("Next"), true, DDialog::ButtonRecommend);
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBtnCancel);
    connect(this->getButton(1), &QPushButton::clicked, this, &PageDriverControl::slotBtnNext);
    connect(mp_PathDialog, &GetDriverPathWidget::signalNotLocalFolder, this, [ = ](bool isLocal) {
        if (!isLocal)
            getButton(1)->setDisabled(true);
        else
            getButton(1)->setDisabled(false);
    });
    connect(mp_NameDialog, &GetDriverNameWidget::signalDriversCount, this, [ = ] {getButton(1)->setDisabled(true);});
    connect(mp_NameDialog, &GetDriverNameWidget::signalItemClicked, this, [ = ] {getButton(1)->setDisabled(false);});
}

void PageDriverControl::initUninstallWidget()
{
    // 先添加警告界面
    QIcon icon(QIcon::fromTheme("cautious"));
    QPixmap pic = icon.pixmap(80, 80);
    DriverIconWidget *widget = new DriverIconWidget(pic, tr("Warning"), tr("The device will be unavailable after the driver uninstallation"), this);
    mp_stackWidget->addWidget(widget);
    mp_stackWidget->setCurrentIndex(0);
    this->addButton(tr("Cancel", "button"), true);
    this->addButton(tr("Uninstall", "button"), true, DDialog::ButtonWarning);
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBtnCancel);
    connect(this->getButton(1), &QPushButton::clicked, this, &PageDriverControl::slotBtnNext);

    mp_WaitDialog = new DriverWaitingWidget(tr("Uninstalling"), this);
    mp_stackWidget->addWidget(mp_WaitDialog);
}

void PageDriverControl::slotBtnCancel()
{
    this->close();
}

void PageDriverControl::slotBtnNext()
{
    if (m_Install) {
        installDriverLogical();
    } else {
        uninstallDriverLogical();
    }
}

void PageDriverControl::slotProcessChange(qint32 value, QString detail)
{
    Q_UNUSED(detail)
    mp_WaitDialog->setValue(value);
}

void PageDriverControl::slotProcessEnd(bool sucess, QString errCode)
{
    QString successStr = m_Install ? tr("Update successful") : tr("Uninstallation successful");
    QString failedStr = m_Install ? tr("Update failed") : tr("Uninstallation failed");
    QString status = sucess ? successStr : failedStr;
    QString iconPath = sucess ? "success" : "fail";
    QIcon icon(QIcon::fromTheme(iconPath));
    QPixmap pic = icon.pixmap(80, 80);
    DriverIconWidget *widget = new DriverIconWidget(pic, status, sucess ? "" : errMsg(errCode), this);
    mp_stackWidget->addWidget(widget);
    this->addButton(tr("OK", "button"), true);
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotClose);
    mp_stackWidget->setCurrentIndex(mp_stackWidget->currentIndex() + 1);
    disconnect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processChange, this, &PageDriverControl::slotProcessChange);
    disconnect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processEnd, this, &PageDriverControl::slotProcessEnd);
    setProperty("DriverProcessStatus", "Done"); //卸载或者加载程序已完成
    enableCloseBtn(true);
}

void PageDriverControl::slotClose()
{
    emit refreshInfo();
    this->close();
}

void PageDriverControl::slotBackPathPage()
{
    mp_NameDialog->stopLoadingDrivers();
    mp_stackWidget->setCurrentIndex(0);
    this->setButtonText(1, tr("Next", "button"));
    this->setButtonText(0, tr("Cancel", "button"));
    this->getButton(0)->disconnect();
    getButton(1)->setEnabled(true);
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBtnCancel);
}

void PageDriverControl::removeBtn()
{
    clearButtons();
}

void PageDriverControl::installDriverLogical()
{
    int curIndex = mp_stackWidget->currentIndex();
    if (0 == curIndex) {
        QString path = mp_PathDialog->path();
        QFile file(path);
        bool includeSubdir = mp_PathDialog->includeSubdir();
        mp_PathDialog->updateTipLabelText("");
        if (path.isEmpty() || !file.exists()) {
            mp_PathDialog->updateTipLabelText(tr("The selected folder does not exist, please select again"));
            return;
        }
        mp_stackWidget->setCurrentIndex(1);
        mp_NameDialog->loadAllDrivers(includeSubdir, path);
        this->setButtonText(1, tr("Update", "button"));
        this->setButtonText(0, tr("Previous", "button"));
        getButton(1)->setDisabled(true);          //默认置灰
        this->getButton(0)->disconnect();
        connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBackPathPage);
    } else if (1 == curIndex) {
        // 驱动安装之前需要先提权
        Authority::Result result = Authority::instance()->checkAuthorizationSync("com.deepin.deepin-devicemanager.checkAuthentication",
                                                                                 UnixProcessSubject(getpid()),
                                                                                 Authority::AllowUserInteraction);
        if (result != Authority::Yes) {
            return;
        }

        QString driveName = mp_NameDialog->selectName();
        //先判断是否是驱动文件，如果不是，再判断是否存在。
        //因为后台isDriverPackage返回false的情况有2种：1.文件不存在 2.不是驱动文件
        mp_NameDialog->updateTipLabelText("");
        if (!installErrorTips(driveName))
            return;
        removeBtn();
        mp_WaitDialog->setValue(0);
        mp_WaitDialog->setText(tr("Updating"));
        mp_stackWidget->setCurrentIndex(2);
        DBusDriverInterface::getInstance()->installDriver(driveName);
        setProperty("DriverProcessStatus", "Doing");//卸载或者加载程序进行中
        enableCloseBtn(false);
    }
}

bool PageDriverControl::installErrorTips(const QString &driveName)
{
    QFile file(driveName);
    if (!DBusDriverInterface::getInstance()->isDebValid(driveName)) {
        mp_NameDialog->updateTipLabelText(tr("Broken package"));
        return false;
    }
    if (!DBusDriverInterface::getInstance()->isArchMatched(driveName)) {
        mp_NameDialog->updateTipLabelText(tr("Unmatched package architecture"));
        return false;
    }

    if (driveName.isEmpty() || !file.exists()) {
        mp_NameDialog->updateTipLabelText(tr("The selected file does not exist, please select again"));
        return false;
    }
    return true;
}

void PageDriverControl::uninstallDriverLogical()
{
    // 点击卸载之后进入正在卸载界面
    removeBtn();
    mp_stackWidget->setCurrentIndex(mp_stackWidget->currentIndex() + 1);
    if (m_printerVendor.count() > 0) {
        DBusDriverInterface::getInstance()->uninstallPrinter(m_printerVendor, m_printerModel);
    } else {
        DBusDriverInterface::getInstance()->uninstallDriver(m_DriverName);
    }
    setProperty("DriverProcessStatus", "Doing");//卸载或者加载程序进行中
    enableCloseBtn(false);
}

void PageDriverControl::keyPressEvent(QKeyEvent *event)
{
    // Esc、Alt+F4：如果后台更新时，直接返回。如果后台更新结束，触发close事件。如果是开始状态，则不处理
    bool blIsClose = false;
    if (event->key() == Qt::Key_F4) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        if (modifiers != Qt::NoModifier) {
            if (modifiers.testFlag(Qt::AltModifier)) {
                blIsClose = true;
            }
        }
    }
    if (Qt::Key_Escape == event->key()) {
        blIsClose = true;
    }
    if (!blIsClose) {
        return;
    }
    if ("Doing" == property("DriverProcessStatus").toString()) {
        return;
    } else if ("Done" == property("DriverProcessStatus").toString()) {
        slotClose();//卸载或者更新后，关闭时刷新
    } else {
        this->close();
    }
}
void PageDriverControl::closeEvent(QCloseEvent *event)
{
    // 如果后台更新时，直接返回，否则不处理
    if ("Doing" == property("DriverProcessStatus").toString()) {
        event->ignore();
        return;
    } else if ("Done" == property("DriverProcessStatus").toString()) {
        slotClose();//卸载或者更新后，关闭时刷新
    }
}

void PageDriverControl::enableCloseBtn(bool enable)
{
    // 获取titlebar
    DTitlebar *titlebar = findChild<DTitlebar *>();
    if (!titlebar) {
        return;
    }

    // 获取安装按钮
    DIconButton *closeBtn = titlebar->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    if (!closeBtn) {
        return;
    }

    // 禁用按钮
    closeBtn->setAttribute(Qt::WA_TransparentForMouseEvents, !enable);
    closeBtn->setEnabled(enable);
}

void PageDriverControl::initErrMsg()
{
    // 初始化错误消息
    m_MapErrMsg.insert(EUNKNOW,tr("Unknown error"));
    m_MapErrMsg.insert(ENOENT,tr("The driver module was not found")); // 2	未发现该驱动模块 /* No such file or directory */
    m_MapErrMsg.insert(ENOEXEC,tr("Invalid module format"));          // 8   模块格式无效 当该驱动已经编译到内核实，安装相同驱动的.ko文件会出现这个错误
    m_MapErrMsg.insert(EAGAIN,tr("The driver module has dependencies"));  // 11	驱动模块被依赖
    m_MapErrMsg.insert(E_FILE_NOT_EXISTED,tr("The selected file does not exist, please select again"));
    m_MapErrMsg.insert(E_NOT_DRIVER,tr("It is not a driver"));
    m_MapErrMsg.insert(E_NOT_SIGNED,tr("Unable to install - no digital signature"));
//    m_MapErrMsg.insert(EBADF,tr(""));                // EBADF		9	/* Bad file number */
//    m_MapErrMsg.insert(EEXIST,tr(""));               // EEXIST		17	/* File exists */
//    m_MapErrMsg.insert(ENODEV,tr(""));               // ENODEV		19	/* No such device */
//    m_MapErrMsg.insert(EROFS,tr(""));               // EROFS		30	/* Read-only file system */
}

const QString &PageDriverControl::errMsg(const QString &errCode)
{
    // 将错误码转换为错误信息
    if (m_MapErrMsg.find(errCode.toInt()) != m_MapErrMsg.end()) {
        return m_MapErrMsg[errCode.toInt()];
    } else {
        return m_MapErrMsg[EUNKNOW];
    }
}
