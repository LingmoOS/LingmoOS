// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/namespace.h"
#include "dtkwidget_global.h"
#include "../src/widgets/titlebuttonitem.h"
#include "../src/widgets/titledslideritem.h"
#include "../src/widgets/dccslider.h"
#include "../src/window/modules/common/compixmap.h"
#include "usbconnectionwork.h"

#include <DTableView>
#include <DCommandLinkButton>
#include <DLabel>

#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QSettings>
#include <QItemDelegate>

DEF_NAMESPACE_BEGIN
DWIDGET_USE_NAMESPACE
using namespace def::widgets;

// usb管控表格类型
enum UsbTableType {
    Record = 0,
    Whitelist
};

class UsbTableNameItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    UsbTableNameItemDelegate(UsbTableType tableType = UsbTableType::Record, QObject *parent = nullptr);
    virtual ~UsbTableNameItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

private:
    UsbTableType m_tableType;
    int m_nameItemCol;
    int m_dataItemCol;
};

/*
USB管控页面
*/
class UsbConnectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UsbConnectionWidget(UsbConnectionWork *work, QWidget *parent = nullptr);
    ~UsbConnectionWidget();

private:
    void refreshUsbConnectionTable();

    void removeItemFromWhiteLstTable(UsbConnectionInfo info);

    QHBoxLayout *initTwoTable();

    QWidget *initConnectionRecordTableWgt();
    QWidget *initWhiteLstTableWgt();
    // 确认改变连接记录开关
    void confirmChangeIsbSaveRecord();
    void showConnectionRecordTableWgt();
    void showWhiteLstTableWgt();
    void appendUsbConnectionLog(UsbConnectionInfo info);
    void confirmClearUsbConnectionLog();
    void clearUsbConnectionLog();
    void appendUsbConnectionWhiteList(UsbConnectionInfo info);
    //根据状态更新usb限制模式项的按钮菜单
    void updateUsbLimitModeItem();
    // 清理界面中超过30天的日志
    void cleanUsbConnectionLog();
    // 更新白名单中名称
    void updateNameinWhiteLst(QString sName, QString sSerial, QString sVendorID, QString sProductID);
    // 通过usb连接信息获得数据项
    QStandardItem *createDataItemFromInfo(UsbConnectionInfo info);
    // 通过数据项获得usb连接信息
    UsbConnectionInfo getInfoFromDataItem(QStandardItem *item) const;
    // 使能所有控件
    void enableAllWidgets(bool enable);
Q_SIGNALS:

public Q_SLOTS:
    // 根据实际开关值刷新界面中开关状态
    void changeIsbSaveRecord();
    void selectLimitationModel(QAction *action);
    void confirmChangeUsbConnectionWhiteLst(UsbTableType type, int dataPosX, int dataPosY);
    void changeUsbConnectionWhiteLst(int opStatus, UsbConnectionInfo info);
    void addUsbConnectionWhiteLst(const UsbConnectionInfo &info);
    void openUsbConnectionDetails(QStandardItem *dateItem);
    void onReceiveUsbInfo(UsbConnectionInfo info);

private:
    UsbConnectionWork *m_usbConnectionWork;

    //初始配置
    bool m_isbSaveRecord;
    QString m_limitationModel;

    TitleButtonItem *m_usbLimitModeItem;
    DLabel *m_usbLimitModeItemButtonLabel;

    QWidget *m_connectionRecordTableWgt;
    QWidget *m_whiteLstTableWgt;

    DTableView *m_connectionRecordTable;
    QStandardItemModel *m_connectionRecordTableModel;
    // 清除记录按钮
    DCommandLinkButton *m_clearLogBtn;
    DTableView *m_whiteLstTable;
    QStandardItemModel *m_whiteLstTableModel;
};

DEF_NAMESPACE_END
