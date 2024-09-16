// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H

#include <QObject>
#include <QHBoxLayout>
#include <DWidget>
#include <DSplitter>

class PageListView;
class PageInfoWidget;
class DeviceBaseInfo;

using namespace Dtk::Widget;

/**
 * @brief The DeviceWidget class
 * 界面类，设备信息的主界面类
 */
class DeviceWidget : public DWidget
{
    Q_OBJECT
public:
    explicit DeviceWidget(QWidget *parent = nullptr);
    ~DeviceWidget();

    /**
     * @brief updateListView:更新ListView
     * @param lst:设备类别
     */
    void updateListView(const QList<QPair<QString, QString> > &lst);

    /**
     * @brief updateDevice:更新设备
     * @param itemStr:设备类别
     * @param lst:该设备类别的所有设备指针lst
     */
    void updateDevice(const QString &itemStr, const QList<DeviceBaseInfo *> &lst);

    /**
     * @brief updateOverview:更新概况界面
     * @param map:概况信息map
     */
    void updateOverview(const QMap<QString, QString> &map);

    /**
     * @brief currentIndex:当前设备类型
     * @return 设备类型
     */
    QString currentIndex() const;

    /**
     * @brief setFontChangeFlag:字体变化标志
     */
    void setFontChangeFlag();

    /**
     * @brief clear:清除数据
     */
    void clear();

signals:

    /**
     * @brief itemClicked:item点击信号
     * @param itemStr:item内容
     */
    void itemClicked(const QString &itemStr);

    /**
     * @brief refreshInfo:刷新信息
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息
     */
    void exportInfo();

private slots:

    /**
     * @brief slotListViewWidgetItemClicked:ListView item点击槽函数
     * @param itemStr:item显示字符串
     */
    void slotListViewWidgetItemClicked(const QString &itemStr);

    /**
     * @brief slotUpdateUI:更新UI界面
     */
    void slotUpdateUI();

protected:
    /**
     * @brief: 事件的重写
     */
    virtual void resizeEvent(QResizeEvent *event)override;

private:
    /**@brief:初始化界面布局*/
    void initWidgets();

private:
    PageListView              *mp_ListView;          //<! 左边的list
    PageInfoWidget            *mp_PageInfo;          //<! 右边的详细内容
    QString                   m_CurItemStr;          //<! 当前Item内容
    QHBoxLayout               *m_Layout;             //<! layout
};

#endif // DETAILWIDGET_H
