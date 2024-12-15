// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include "widgets/thumbnail/thumbnaillistview.h"
#include "widgets/widgtes/expansionmenu.h"
#include "qmlWidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <DListWidget>
#include <DCommandLinkButton>
#include <DGuiApplicationHelper>
#include <QGraphicsOpacityEffect>

class NoResultWidget;
class TimeLineView : public DWidget
{
    friend class QmlWidget;
    Q_OBJECT

public:
    TimeLineView(QmlWidget *parent = nullptr);
    ~TimeLineView() override
    {
    }

    void themeChangeSlot(DGuiApplicationHelper::ColorType themeType);
    ThumbnailListView *getThumbnailListView();
    //tab进入时清除其他所有选中
    void clearAllSelection();

public slots:
    void on_AddLabel(QString date, QString num);
    void onCheckBoxClicked();
    void clearAndStartLayout();
    //更新最上当悬浮标题时间与数量
    void slotTimeLineDataAndNum(QString data, QString num, QString text);
    //筛选显示变化
    void sltCurrentFilterChanged(ExpansionPanel::FilteData &data);
protected:
    void resizeEvent(QResizeEvent *ev) override;
    QPaintEngine * paintEngine() const override;
    void paintEvent(QPaintEvent * event) override;
private:
    void initTimeLineViewWidget();
    void initConnections();
    void addTimelineLayout();
    void initDropDown();
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

public slots:
    //筛选显示，当先列表中内容为无结果
    void slotNoPicOrNoVideo(bool isNoResult);

    void onShowCheckBox(bool bShow);
private:
    QLayout *m_mainLayout;
    QList<QString> m_timelines;
    //悬浮时间栏控件
    DWidget *m_dateNumItemWidget = nullptr;
    BatchOperateWidget *m_batchOperateWidget = nullptr;
    DLabel *m_dateLabel = nullptr;
    DLabel *m_numLabel = nullptr;
    DCheckBox *m_numCheckBox = nullptr;
    ExpansionMenu *m_expansionMenu = nullptr;
    FilterWidget *m_ToolButton = nullptr;

    int allnum;
    ThumbnailListView *m_timeLineThumbnailListView = nullptr;//时间线缩略图列表，含时间项
    QGraphicsOpacityEffect *m_oe;
    QGraphicsOpacityEffect *m_oet;

public:
    NoResultWidget *m_noResultWidget = nullptr;
    DWidget *m_timeLineViewWidget;
    int m_selPicNum;

    QmlWidget *m_qquickContainer;
};

#endif // TIMELINEVIEW_H
