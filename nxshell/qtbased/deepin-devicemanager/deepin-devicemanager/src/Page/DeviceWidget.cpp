// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 其它头文件
#include "DeviceWidget.h"
#include "PageListView.h"
#include "PageInfoWidget.h"
#include "DeviceInfo.h"
#include "MacroDefinition.h"
#include "DeviceManager.h"
#include "DeviceBios.h"

// Qt库文件
#include <QHBoxLayout>
#include <QLoggingCategory>


DeviceWidget::DeviceWidget(QWidget *parent)
    : DWidget(parent)
    , mp_ListView(new PageListView(this))
    , mp_PageInfo(new PageInfoWidget(this))
    , m_CurItemStr("")
    , m_Layout(nullptr)
{
    // 初始化界面布局
    initWidgets();

    // 连接槽函数
    connect(mp_ListView, &PageListView::itemClicked, this, &DeviceWidget::slotListViewWidgetItemClicked);
    connect(mp_PageInfo, &PageInfoWidget::refreshInfo, this, &DeviceWidget::refreshInfo);
    connect(mp_PageInfo, &PageInfoWidget::exportInfo, this, &DeviceWidget::exportInfo);
    connect(mp_PageInfo, &PageInfoWidget::updateUI, this, &DeviceWidget::refreshInfo);

    connect(mp_ListView, &PageListView::refreshActionTrigger, this, &DeviceWidget::refreshInfo);
    connect(mp_ListView, &PageListView::exportActionTrigger, this, &DeviceWidget::exportInfo);
}

DeviceWidget::~DeviceWidget()
{
    if (mp_ListView) {
        delete mp_ListView;
        mp_ListView = nullptr;
    }
    if (mp_PageInfo) {
        delete mp_PageInfo;
        mp_PageInfo = nullptr;
    }
    if (m_Layout) {
        delete m_Layout;
        m_Layout = nullptr;
    }
}

void DeviceWidget::updateListView(const QList<QPair<QString, QString> > &lst)
{
    // 更新左边的列表
    if (mp_ListView)
        mp_ListView->updateListItems(lst);
}

void DeviceWidget::updateDevice(const QString &itemStr, const QList<DeviceBaseInfo *> &lst)
{
    if (lst.size() == 0)
        return;

    // 更新右边的详细内容
    if (mp_PageInfo)
        mp_PageInfo->updateTable(itemStr, lst);
}

void DeviceWidget::updateOverview(const QMap<QString, QString> &map)
{
    if (map.size() == 0)
        return;

    // 更新概况
    if (mp_PageInfo)
        mp_PageInfo->updateTable(map);
}

QString DeviceWidget::currentIndex() const
{
    // 当前设备类型
    return mp_ListView->currentType();
}

void DeviceWidget::setFontChangeFlag()
{
    // 设置字体变化标志
    mp_PageInfo->setFontChangeFlag();
}

void DeviceWidget::clear()
{
    mp_ListView->clear();
    mp_PageInfo->clear();
}

void DeviceWidget::slotListViewWidgetItemClicked(const QString &itemStr)
{
    // ListView Item 点击
    m_CurItemStr = itemStr;
    emit itemClicked(itemStr);
}

void DeviceWidget::slotUpdateUI()
{
    // 更新当前UI界面
    emit itemClicked(m_CurItemStr);
}

void DeviceWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
    // 获取所有设备类别
    const QList<QPair<QString, QString>> &types = DeviceManager::instance()->getDeviceTypes();

    // 根据右侧Listview当前Index获取当前设备类别的
    QString userStr = mp_ListView->currentIndex();

    QString deviceType = "";
    foreach (auto iter, types) {
        if (iter.second.contains(userStr)) {
            deviceType = iter.first;
            break;
        }
    }

    // 根据设备类别获取设备指针
    QList<DeviceBaseInfo *> lst;
    bool ret = DeviceManager::instance()->getDeviceList(deviceType, lst);
    if (! ret) {
        // 更新Overview界面
        QMap<QString, QString> overviewMap = DeviceManager::instance()->getDeviceOverview();
        mp_PageInfo->updateTable(overviewMap);
    }

    // 更新设备信息界面
    if (lst.size() == 1) {
        mp_PageInfo->updateTable(deviceType, lst);
    } else if (lst.size() > 1) {
        // 判断是否是BIOS界面
        DeviceBios *bios = dynamic_cast<DeviceBios *>(lst[0]);
        if (bios)
            mp_PageInfo->updateTable(deviceType, lst);
    }
}

void DeviceWidget::initWidgets()
{
    // 初始化页面布局
    m_Layout = new QHBoxLayout();
    m_Layout->setContentsMargins(0, 0, 0, 0);
    m_Layout->setSpacing(0);
    m_Layout->addWidget(mp_ListView);
    m_Layout->addWidget(mp_PageInfo);
    setLayout(m_Layout);
}
