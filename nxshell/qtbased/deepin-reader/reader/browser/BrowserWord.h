// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BrowserWord_H
#define BrowserWord_H

#include "Model.h"

#include <QGraphicsItem>
#include <QRectF>

class BrowserPage;

/**
 * @brief 用于表示一页文档中的某个文字
 */
class BrowserWord : public QGraphicsItem
{
public:
    explicit BrowserWord(QGraphicsItem *parent, deepin_reader::Word word);

    QRectF boundingRect()const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 获取文字文本
     * @return
     */
    QString text();

    /**
     * @brief 设置文字无法选中
     * @param enable 是否可以被选中
     */
    void setSelectable(bool enable);

    /**
     * @brief 为了让文字占比固定,视图改变时需要设置缩放因子
     * @param scaleFactor 缩放因子
     */
    void setScaleFactor(qreal scaleFactor);

    /**
     * @brief BrowserWord::boundingBox
     * 文字自身的区域
     * @return
     */
    QRectF boundingBox()const;

private:
    deepin_reader::Word m_word;

    qreal m_scaleFactor = 1;

    bool m_selectable = true;
};

#endif // BrowserWord_H
