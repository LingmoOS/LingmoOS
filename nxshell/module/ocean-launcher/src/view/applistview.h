// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLISTVIEW_H
#define APPLISTVIEW_H

#include <DWindowManagerHelper>
#include <DGuiApplicationHelper>
#include <DListView>

#include <QListView>
#include <QPropertyAnimation>
#include <QDBusInterface>

#define DRAG_SCROLL_THRESHOLD 25

DGUI_USE_NAMESPACE

class AppListView : public Dtk::Widget::DListView
{
    Q_OBJECT

public:
    explicit AppListView(QWidget *parent = nullptr);

    using DListView::indexAt;
    const QModelIndex indexAt(const int index) const;

signals:
    void popupMenuRequested(const QPoint &pos, const QModelIndex &index) const;
    void requestScrollUp() const;
    void requestScrollDown() const;
    void requestScrollStop() const;
    void requestSwitchToCategory(const QModelIndex &index) const;
    void requestEnter(bool enter) const;

public slots:
    void menuHide();

private slots:
    void onThemeChanged(DGuiApplicationHelper::ColorType);

protected:
    void wheelEvent(QWheelEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void startDrag(const QModelIndex &index);

private:
    void handleScrollValueChanged();
    void handleScrollFinished();
    void prepareDropSwap();
    void createFakeAnimation(const int pos, const bool moveNext, const bool isLastAni = false);
    void dropSwap();

private:
    int m_dropToRow;
    int m_dragStartRow;
    QPoint m_dragStartPos;
    QPoint m_fullscreenStartPos;
    QTimer *m_dropThresholdTimer;
    bool m_enableDropInside = false;                    // 小窗口模式标识
    bool m_touchMoveFlag;                               // 代表触摸屏移动操作

    QPropertyAnimation *m_lastFakeAni = nullptr;
    QPropertyAnimation *m_scrollAni;
    double m_speedTime = 1.0;

    QTimer *m_updateEnableSelectionByMouseTimer;        // 限制拖拽时间不能少于200ms
    QTimer *m_updateEnableShowSelectionByMouseTimer;    // 检测按压是否现实选择灰色背景
    QPoint m_lastTouchBeginPos;
};

#endif // APPLISTVIEW_H
