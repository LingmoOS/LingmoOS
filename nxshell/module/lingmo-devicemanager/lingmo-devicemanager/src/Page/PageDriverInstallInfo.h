// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEDRIVERINSTALLINFO_H
#define PAGEDRIVERINSTALLINFO_H

#include <QObject>
#include <DWidget>
#include <DFrame>
#include <DLabel>

#include "MacroDefinition.h"
#include "driveritem.h"

class DetectedStatusWidget;
class PageDriverTableView;

using namespace Dtk::Widget;

class PageDriverInstallInfo : public DFrame
{
    Q_OBJECT
public:
    explicit PageDriverInstallInfo(QWidget *parent = nullptr);

    DetectedStatusWidget *headWidget() {return mp_HeadWidget; }

    /**
     * @brief addDriverInfoToTableView 添加驱动信息到表格中
     */
    void addDriverInfoToTableView(DriverInfo *info, int index);

    /**
     * @brief addCurDriverInfo 当驱动为最新时，不更新
     * @param info
     */
    void addCurDriverInfo(DriverInfo *info);

    /**
     * @brief showTables 显示表格
     */
    void showTables(int, int, int);

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
    void updateItemStatus(int index, Status status, QString errS = "");

    /**
     * @brief setCheckedCBDisnable 置灰checkbox
     */
    void setCheckedCBDisnable();

    /**
     * @brief setHeaderCbEnable 设置表头checkbox状态
     */
    void setHeaderCbEnable(bool enable);

public slots:
    /**
     * @brief slotDownloadProgressChanged 下载进度刷新
     */
    void slotDownloadProgressChanged(DriverType type, QString size, QStringList msg);

    /**
     * @brief slotDownloadProgressChanged 下载完成
     */
    void slotDownloadFinished(int, Status);

signals:
    void operatorClicked(int index, int itemIndex, DriverOperationItem::Mode mode);
    void itemChecked(int index, bool checked);
    void redetected();
    void installAll();
    void undoInstall();

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

    PageDriverTableView  *mp_ViewNotInstall; //没有安装驱动的列表
    PageDriverTableView  *mp_ViewCanUpdate;  //可以更新的驱动列表
    PageDriverTableView  *mp_AllDriverIsNew; //当所有驱动为最新时，显示此page

    DLabel               *mp_InstallLabel;
    DLabel               *mp_UpdateLabel;
    DLabel               *mp_LabelIsNew;
    DWidget              *mp_InstallWidget;
    DWidget              *mp_UpdateWidget;
};

#endif // PAGEDRIVERINSTALLINFO_H
