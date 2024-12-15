// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEDRIVERRESTOREINFO_H
#define PAGEDRIVERRESTOREINFO_H

#include <QObject>
#include <DWidget>
#include <DFrame>
#include <DLabel>
#include <DSuggestButton>

#include "MacroDefinition.h"
#include "driveritem.h"

class DetectedStatusWidget;
class PageDriverTableView;

using namespace Dtk::Widget;

class PageDriverRestoreInfo : public DFrame
{
     Q_OBJECT
public:
    explicit PageDriverRestoreInfo(QWidget *parent = nullptr);

    DetectedStatusWidget *headWidget() {return mp_HeadWidget; }

    /**
     * @brief addDriverInfoToTableView 添加驱动信息到表格中
     */
    void addDriverInfoToTableView(DriverInfo *info, int index);

    /**
     * @brief showTables 显示表格
     */
    void showTables(int);

    /**
     * @brief clearAllData 清除所有数据
     */
    void clearAllData();

    /**
     * @brief setItemOperationEnable
     */
    void setItemOperationEnable(int index, bool enable);

private slots:
    /**
     * @brief slotOperatorClicked 操作按钮点击槽函数
     */
    void slotOperatorClicked(int index, int itemIndex, DriverOperationItem::Mode mode);

signals:
    void operatorClicked(int index, int itemIndex, DriverOperationItem::Mode mode);
    void redetected();
    void gotoBackup();

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
    DStackedWidget          *mp_StackWidget;
    DetectedStatusWidget    *mp_HeadWidget;
    DWidget                 *mp_NotBackupWidget;
    PageDriverTableView     *mp_ViewBackable;
    DLabel                  *mp_BackableDriverLabel;

    // 无备份驱动时显示
    DFrame                  *mp_NoRestoreDriverFrame;
    DSuggestButton          *mp_GotoBackupSgButton;
};

#endif // PAGEDRIVERRESTOREINFO_H
