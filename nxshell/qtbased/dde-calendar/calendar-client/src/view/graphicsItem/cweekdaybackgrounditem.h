// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CWEEKDAYBACKGROUNDITEM_H
#define CWEEKDAYBACKGROUNDITEM_H

#include "cscenebackgrounditem.h"

#include <QDate>
/**
 * @brief The CWeekDayBackgroundItem class
 * 周和天的全天/非全天背景绘制
 */
class CWeekDayBackgroundItem : public CSceneBackgroundItem
{
    Q_OBJECT
public:
    explicit CWeekDayBackgroundItem(QGraphicsItem *parent = nullptr);
    //设置主题
    void setTheMe(int type = 0);
    //是否绘制分割线
    bool drawDividingLine() const;
    //设置是否绘制分割线
    void setDrawDividingLine(bool drawDividingLine);
    //获取改背景是否焦点显示
    bool showFocus() const;
    //设置该背景是否焦点显示
    void setShowFocus(bool showFocus);
    //设置item是否获取focus
    void setItemFocus(bool isFocus) override;
    //在该背景上是否还有下一个需要焦点切换的item
    bool hasNextSubItem();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void updateCurrentItemShow() override;
signals:
    void signalPosOnView(const qreal y);
public slots:
private:
    bool m_drawDividingLine; //是否绘制分割线
    bool m_showFocus; //背景是否显示焦点效果
    QColor m_weekColor; //周六周日背景色
};

#endif // CWEEKDAYBACKGROUNDITEM_H
