// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFYLISTVIEW_H
#define NOTIFYLISTVIEW_H

#include "notification/constants.h"
#include "notifymodel.h"

#include <DListView>

#include <QPoint>

class QPropertyAnimation;
class QScrollBar;
class QTimer;
class QBasicTimer;
class BubbleItem;

DWIDGET_USE_NAMESPACE

class NotifyListView : public DListView
{
    Q_OBJECT
public:
    explicit NotifyListView(QWidget *parent = nullptr);
    ~NotifyListView() override;

public:
    void createAddedAnimation(EntityPtr entity, const ListItemPtr appItem);
    void createRemoveAnimation(BubbleItem *item);
    void createExpandAnimation(int idx, const ListItemPtr appItem);
    bool aniState() { return m_aniState; }
    void setCurrentRow(int row);
    QWidget *lastItemView() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    virtual bool event(QEvent *event) override;
    virtual void timerEvent(QTimerEvent *e) override;

private:
    bool canShow(EntityPtr ptr); // 判断消息是否应该层叠[即超过4小时]
    void handleScrollValueChanged();
    void handleScrollFinished();
    void updateViewHeight();

signals:
    void removeAniFinished(EntityPtr ptr);
    void expandAniFinished(QString appName);
    void addedAniFinished(EntityPtr entity);
    void refreshItemTime();
    void lastItemCreated();

private:
    bool m_aniState = false;
    int m_currentIndex = 0;
    double m_speedTime = 2.0;
    QPropertyAnimation *m_scrollAni;
    QPointer<QWidget> m_prevElement = nullptr;
    QPointer<QWidget> m_currentElement = nullptr;
    QTimer *m_refreshTimer = nullptr;
    QBasicTimer *m_layoutRequestTimer = nullptr;
};

#endif // NOTIFYLISTVIEW_H
