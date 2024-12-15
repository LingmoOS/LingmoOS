// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMPORTTIMELINEVIEW_H
#define IMPORTTIMELINEVIEW_H

#include "widgets/thumbnail/thumbnaillistview.h"
#include "widgets/widgtes/expansionmenu.h"
#include "qmlWidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGraphicsOpacityEffect>
#include <QQuickPaintedItem>

#include <DListWidget>
#include <DCommandLinkButton>
#include <DGuiApplicationHelper>

class BatchOperateWidget;
class NoResultWidget;
class ImportTimeLineView : public DWidget
{
    friend class QmlWidget;
    Q_OBJECT
public:
    explicit ImportTimeLineView(QmlWidget *parent = nullptr);
    ~ImportTimeLineView() override
    {
        void clearAndStop();
    }

public slots:
    //更新最上当悬浮标题时间与数量
    void slotTimeLineDataAndNum(QString data, QString num, QString text);

    void onShowCheckBox(bool bShow);
    void clearAndStartLayout();
protected:
    void resizeEvent(QResizeEvent *ev) override;
    QPaintEngine * paintEngine() const override;
    void paintEvent(QPaintEvent * event) override;
    void showEvent(QShowEvent *ev) override;

private:
    void initTimeLineViewWidget();
    void initConnections();
    void updateDateNumLabel();

    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;

public:
    void addTimelineLayout();
    void initDropDown();
    void themeChangeSlot(DGuiApplicationHelper::ColorType themeType);
    ThumbnailListView *getListView();
    //tab进入时清除其他所有选中
    void clearAllSelection();
#if 1
    QStringList selectPaths();
#endif
private slots:
    /**
     * @brief updateSize
     * 调整已导入界面的整体大小
     */
    void updateSize();
    void onCheckBoxClicked();
    //筛选显示，当先列表中内容为无结果
    void slotNoPicOrNoVideo(bool isNoResult);
    //筛选显示变化
    void sltCurrentFilterChanged(ExpansionPanel::FilteData &data);
signals:
    void sigUpdatePicNum();
    //筛选显示图片或者视频，无结果时
    void sigNoPicOrNoVideo(bool isNoResult);//1050
public:
    const static int title_HEIGHT = 87;
private:
    QLayout *m_mainLayout;
    QList<QDateTime> m_timelines;
    DWidget *m_importTitleItem;                               //title包裹窗口
    DWidget *m_timeLineViewWidget = nullptr;
    BatchOperateWidget *m_batchOperateWidget = nullptr;
    DLabel *m_importLabel = nullptr;                    //已导入悬浮标题
    DLabel *m_dateNumLabel = nullptr;                   //已导入悬浮日期数量
    DCheckBox *m_dateNumCheckBox = nullptr;
    ExpansionMenu *m_expansionMenu = nullptr;
    FilterWidget *m_ToolButton = nullptr;
    QString dateFullStr;
    QString numFullStr;
    ThumbnailListView *m_importTimeLineListView = nullptr;
    NoResultWidget *m_noResultWidget = nullptr;
    QGraphicsOpacityEffect *m_oe;
    QGraphicsOpacityEffect *m_oet;

    bool m_ctrlPress;

    QmlWidget *m_qquickContainer;
};

#endif // IMPORTTIMELINEVIEW_H
