// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEINFOWIDGET_H
#define PAGEINFOWIDGET_H

#include <QObject>
#include <DWidget>
//#include <QHBoxLayout>

class DeviceBaseInfo;
class PageInfo;
class PageMultiInfo;
class PageSingleInfo;
class PageOverview;
class PageBoardInfo;

using namespace Dtk::Widget;

/**
 * @brief The PageInfoWidget class
 * UI 设备信息的展示类
 */

class PageInfoWidget : public DWidget
{
    Q_OBJECT
public:
    explicit PageInfoWidget(QWidget *parent = nullptr);

    /**
     * @brief updateTable: 更新表格
     * @param itemStr:设备类型
     * @param lst:设备类型对应的所有设备列表
     */
    void updateTable(const QString &itemStr, const QList<DeviceBaseInfo *> &lst);

    /**
     * @brief updateTable: 更新表格
     * @param map:概况信息map
     */
    void updateTable(const QMap<QString, QString> &map);

    /**
     * @brief setFontChangeFlag: 设置字体变化标志
     */
    void setFontChangeFlag();

    /**
     * @brief clear:清除数据
     */
    void clear();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息信号
     */
    void exportInfo();
    void updateUI();

private:
    /**
     * @brief initWidgets
     */
    void initWidgets();

private:
    PageInfo       *mp_PageInfo;
    PageSingleInfo *mp_PageSignalInfo;
    PageMultiInfo  *mp_PageMutilInfo;
    PageOverview   *mp_PageOverviewInfo;
    PageBoardInfo  *mp_PageBoardInfo;
};

#endif // PAGEINFOWIDGET_H
