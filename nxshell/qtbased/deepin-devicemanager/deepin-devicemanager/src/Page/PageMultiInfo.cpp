// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PageMultiInfo.h"
#include "PageTableHeader.h"
#include "PageDetail.h"
#include "MacroDefinition.h"
#include "DeviceInfo.h"
#include "PageDriverControl.h"
#include "DevicePrint.h"
#include "DeviceInput.h"
#include "DeviceNetwork.h"
#include "DDLog.h"

// Dtk头文件
#include <DFontSizeManager>
#include <DMessageBox>
#include <DMenu>
#include <DMessageManager>

// Qt库文件
#include <QVBoxLayout>
#include <QAction>
#include <QIcon>
#include <QLoggingCategory>
#include <QProcess>

DWIDGET_USE_NAMESPACE

using namespace DDLog;

#define LEAST_PAGE_HEIGHT 315  // PageMultiInfo最小高度 当小于这个高度时，上方的表格就要变小

PageMultiInfo::PageMultiInfo(QWidget *parent)
    : PageInfo(parent)
    , mp_Label(new DLabel(this))
    , mp_Table(new PageTableHeader(this))
    , mp_Detail(new PageDetail(this))
{
    m_deviceList.clear();
    m_menuControlList.clear();
    // 初始化界面布局
    initWidgets();

    // 连接槽函数
    connect(mp_Table, &PageTableHeader::itemClicked, this, &PageMultiInfo::slotItemClicked);
    connect(mp_Table, &PageTableHeader::refreshInfo, this, &PageMultiInfo::refreshInfo);
    connect(mp_Table, &PageTableHeader::exportInfo, this, &PageMultiInfo::exportInfo);
    connect(mp_Detail, &PageDetail::refreshInfo, this, &PageMultiInfo::refreshInfo);
    connect(mp_Detail, &PageDetail::exportInfo, this, &PageMultiInfo::exportInfo);
    connect(mp_Table, &PageTableHeader::enableDevice, this, &PageMultiInfo::slotEnableDevice);
    connect(mp_Table, &PageTableHeader::installDriver, this, &PageMultiInfo::slotActionUpdateDriver);
    connect(mp_Table, &PageTableHeader::uninstallDriver, this, &PageMultiInfo::slotActionRemoveDriver);
    connect(mp_Table, &PageTableHeader::wakeupMachine, this, &PageMultiInfo::slotWakeupMachine);
    connect(mp_Table, &PageTableHeader::signalCheckPrinterStatus, this, &PageMultiInfo::slotCheckPrinterStatus);
    emit refreshInfo();
}

PageMultiInfo::~PageMultiInfo()
{
    // 清空指针
    if (mp_Table) {
        delete mp_Table;
        mp_Table = nullptr;
    }
    if (mp_Detail) {
        delete mp_Detail;
        mp_Detail = nullptr;
    }
    m_deviceList.clear();
    m_menuControlList.clear();
}

void PageMultiInfo::updateInfo(const QList<DeviceBaseInfo *> &lst)
{
    m_lstDevice.clear();
    m_lstDevice = lst;

    if (lst.size() < 1)
        return;
    m_deviceList.clear();
    m_menuControlList.clear();

    //  获取多个设备界面表格信息
    getTableListInfo(lst, m_deviceList, m_menuControlList);

    // 更新表格
    mp_Table->updateTable(m_deviceList, m_menuControlList);

    // 更新详细信息
    mp_Detail->showDeviceInfo(lst);
}

void PageMultiInfo::setLabel(const QString &itemstr)
{
    if (mp_Label) {
        mp_Label->setText(itemstr);

        // 设备类型加粗
        QFont font = mp_Label->font();
        font.setWeight(63);
        mp_Label->setFont(font);

        DFontSizeManager::instance()->bind(mp_Label, DFontSizeManager::T5);
    }
}

void PageMultiInfo::clearWidgets()
{
    m_lstDevice.clear();
    mp_Detail->clearWidget();
}

void PageMultiInfo::resizeEvent(QResizeEvent *e)
{
    if (m_lstDevice.size() < 1)
        return PageInfo::resizeEvent(e);

    // 先获取当前窗口大小
    int curHeight = this->height();
    if (curHeight < LEAST_PAGE_HEIGHT) {
        //  获取多个设备界面表格信息
        mp_Table->updateTable(m_deviceList, m_menuControlList, true, (LEAST_PAGE_HEIGHT - curHeight) / TREE_ROW_HEIGHT + 1);
    } else {
        //  获取多个设备界面表格信息
        mp_Table->updateTable(m_deviceList, m_menuControlList, true, 0);
    }

    return PageInfo::resizeEvent(e);
}

void PageMultiInfo::slotItemClicked(int row)
{
    // 显示表格中选择设备的详细信息
    if (mp_Detail)
        mp_Detail->showInfoOfNum(row);
}

void PageMultiInfo::slotEnableDevice(int row, bool enable)
{
    if (!mp_Detail)
        return;

    // 禁用/启用设备
    EnableDeviceStatus res = mp_Detail->enableDevice(row, enable);

    // 除设置成功的情况，其他情况需要提示设置失败
    if (res == EDS_Success) {
        // 设置成功,更新界面
        emit updateUI();
    } else if (res == EDS_Faild) {
        // 设置失败
        QString con;
        if (enable)
            // 无法启用设备
            con = tr("Failed to enable the device");
        else
            // 无法禁用设备
            con = tr("Failed to disable the device");

        // 禁用、启用失败提示
        DMessageManager::instance()->sendMessage(this->window(), QIcon::fromTheme("warning"), con);
    } else if (res == EDS_NoSerial) {
        QString con = tr("Failed to disable it: unable to get the device SN");
        DMessageManager::instance()->sendMessage(this->window(), QIcon::fromTheme("warning"), con);
    }
}

void PageMultiInfo::slotWakeupMachine(int row, bool wakeup)
{
    if (!mp_Detail)
        return;
    mp_Detail->setWakeupMachine(row, wakeup);
}

void PageMultiInfo::slotActionUpdateDriver(int row)
{
    DeviceBaseInfo *device = m_lstDevice[row];
    //打印设备卸载驱动时，通过dde-printer来操作
    if (nullptr != device && device->hardwareClass() == "printer") {
        if (!QProcess::startDetached("dde-printer"))
            qCInfo(appLog) << "dde-printer startDetached error";
        return;
    }

    PageDriverControl *installDriver = new PageDriverControl(this, tr("Update Drivers"), true, device->name(), "");
    installDriver->show();
    connect(installDriver, &PageDriverControl::refreshInfo, this, &PageMultiInfo::refreshInfo);
}

void PageMultiInfo::slotActionRemoveDriver(int row)
{
    DeviceBaseInfo *device = m_lstDevice[row];
    if (nullptr == device) {
        return;
    }
    QString printerVendor;
    QString printerModel;
    DevicePrint *printer = qobject_cast<DevicePrint *>(device);
    if (printer) {
        printerVendor = printer->getVendor();
        printerModel = printer->getModel();
    }
    PageDriverControl *rmDriver = new PageDriverControl(this, tr("Uninstall Drivers"), false,
                                                        device->name(), device->driver(), printerVendor, printerModel);
    rmDriver->show();
    connect(rmDriver, &PageDriverControl::refreshInfo, this, &PageMultiInfo::refreshInfo);
}

void PageMultiInfo::slotCheckPrinterStatus(int row, bool &isPrinter, bool &isInstalled)
{
    DeviceBaseInfo *device = m_lstDevice.value(row, nullptr);
    if (!device)
        return;
    DevicePrint *printer = qobject_cast<DevicePrint *>(device);
    if (printer) {
        isPrinter = true;
        isInstalled = PageInfo::packageHasInstalled("dde-printer");
    }
}

void PageMultiInfo::initWidgets()
{
    // 初始化界面布局
    QVBoxLayout *hLayout = new QVBoxLayout();
    QHBoxLayout *labelLayout = new QHBoxLayout();
    labelLayout->addSpacing(10);
    labelLayout->addWidget(mp_Label);

    // Label 距离上下控件的距离LABEL_MARGIN
    hLayout->addSpacing(LABEL_MARGIN);
    hLayout->addLayout(labelLayout);
    hLayout->addSpacing(LABEL_MARGIN);

    mp_Table->setFixedHeight(TABLE_HEIGHT);

    hLayout->addWidget(mp_Table);
    hLayout->addWidget(mp_Detail);
    hLayout->setContentsMargins(10, 10, 10, 0);

    setLayout(hLayout);
}

void PageMultiInfo::getTableListInfo(const QList<DeviceBaseInfo *> &lst, QList<QStringList> &deviceList, QList<QStringList> &menuControlList)
{
    //  获取多个设备界面表格信息
     if(lst[0] == nullptr) return;
    deviceList.append(lst[0]->getTableHeader());
    foreach (DeviceBaseInfo *info, lst) {
        if(info == nullptr) continue;
        QStringList lstDeviceInfo = info->getTableData();
        deviceList.append(lstDeviceInfo);

        QStringList menuControl;
        menuControl.append(info->canUninstall() ? "true" : "false");
        menuControl.append(info->canEnable() ? "true" : "false");
        DeviceInput *input = dynamic_cast<DeviceInput *>(info);
        if (input) {
            menuControl.append(input->canWakeupMachine() ? "true" : "false");
            menuControl.append(input->wakeupPath());
        }

        DeviceNetwork *network = dynamic_cast<DeviceNetwork *>(info);
        if (network) {
            menuControl.append(network->logicalName());
        }

        menuControlList.append(menuControl);
    }
}
