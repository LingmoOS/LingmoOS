// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEDRIVERBACKUPINFO_H
#define PAGEDRIVERBACKUPINFO_H

#include <QObject>
#include <DWidget>
#include <DFrame>
#include <DLabel>

#include "MacroDefinition.h"
#include "driveritem.h"

class DetectedStatusWidget;
class PageDriverTableView;

using namespace Dtk::Widget;

class PageDriverBackupInfo : public DFrame
{
     Q_OBJECT
public:
    explicit PageDriverBackupInfo(QWidget *parent = nullptr);

    DetectedStatusWidget *headWidget() {return mp_HeadWidget; }

    /**
     * @brief addDriverInfoToTableView 添加驱动信息到表格中
     */
    void addDriverInfoToTableView(DriverInfo *info, int index);

    /**
     * @brief showTables 显示表格
     */
    void showTables(int, int);

    /**
     * @brief getCheckedDriverIndex 获取选中驱动索引
     */
    void getCheckedDriverIndex(QList<int> &lstIndex);

    /**
     * @brief clearAllData 清除所有数据
     */
    void clearAllData();

    /**
     * @brief updateItemStatus 更新状态
     */
    void updateItemStatus(int index, Status status);

    /**
     * @brief setCheckedCBDisnable 置灰checkbox
     */
    void setCheckedCBDisnable();

    /**
     * @brief setHeaderCbEnable 设置表头checkbox状态
     */
    void setHeaderCbEnable(bool enable);

signals:
    void operatorClicked(int index, int itemIndex, DriverOperationItem::Mode mode);
    void itemChecked(int index, bool checked);
    void redetected();
    void backupAll();
    void undoBackup();

private:
    /**
     * @brief initWidget 初始化界面
     */
    void initUI();

    /**
     * @brief initTable 初始化表格信息
     */
    void initTable();


private:
    DetectedStatusWidget *mp_HeadWidget;
    DWidget              *mp_NotBackupWidget;
    PageDriverTableView  *mp_ViewBackable; //可备份驱动列表
    DLabel               *mp_BackableDriverLabel;

    DWidget              *mp_BackedUpWidget;
    PageDriverTableView  *mp_ViewBackedUp; //已备份驱动列表
    DLabel               *mp_BackedUpDriverLabel;
};

#endif // PAGEDRIVERBACKUPINFO_H
