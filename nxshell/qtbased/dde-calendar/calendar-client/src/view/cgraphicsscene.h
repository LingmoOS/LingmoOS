// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CGRAPHICSSCENE_H
#define CGRAPHICSSCENE_H

#include "ckeypressprxy.h"

#include <QGraphicsScene>

/**
 * @brief The CGraphicsScene class
 *
 */

class CFocusItem;
class CGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CGraphicsScene(QObject *parent = nullptr);
    ~CGraphicsScene() override;
    //设置Scene的第一个focus item
    void setFirstFocusItem(QGraphicsItem *item);
    //获取Scene的第一个focus item
    QGraphicsItem *getFirstFocusItem() const;
    //设置Scene当前focus item
    void setCurrentFocusItem(QGraphicsItem *item);
    //获取Scene当前focus item
    QGraphicsItem *getCurrentFocusItem() const;
    //设置keypress处理
    void setKeyPressPrxy(CKeyPressPrxy *keyPrxy);
    //更新当前item焦点效果显示
    void currentFocusItemUpdate();
    //设置上一页时间
    void setPrePage(const QDate &focusDate, bool isSwitchView = false);
    //设置下一页时间
    void setNextPage(const QDate &focusDate, bool isSwitchView = false);
    //获取是否为主动切换
    bool getActiveSwitching() const;
    //设置是否为主动切换
    void setActiveSwitching(bool activeSwitching);
    void currentItemInit();

    void setIsShowCurrentItem(bool isShowCurrentItem);

    void setIsContextMenu(bool isContextMenu);
protected:
    bool event(QEvent *event) override;
private:
    bool focusInDeal(QEvent *event);
    bool focusOutDeal(QEvent *event);
signals:
    void signalSwitchPrePage(const QDate &focusDate, bool isSwitchView = false);
    void signalSwitchNextPage(const QDate &focusDate, bool isSwitchView = false);
    void signalSwitchView(const QDate &focusDate, bool setItemFocus = false);
    //通知另一个视图初始化
    void signalViewFocusInit();
    //切换到日视图
    void signalGotoDayView(const QDate &date);
    //快捷键触发右击
    void signalContextMenu(CFocusItem *item);
    //切换焦点到下一个widget
    void signalsetNextFocus();

private:
    QGraphicsItem *firstfocusItem;
    QGraphicsItem *currentFocusItem;
    CKeyPressPrxy *m_keyPrxy;
    bool m_activeSwitching; //是否为主动切换焦点
    bool m_isContextMenu; //是否为右击菜单切换焦点
    bool m_isShowCurrentItem; //true 当前item获取焦点  false 当前item的下一个item获取焦点
};

#endif // CGRAPHICSSCENE_H
