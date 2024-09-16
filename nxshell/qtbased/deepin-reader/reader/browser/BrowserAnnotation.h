// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BrowserAnnotation_H
#define BrowserAnnotation_H

#include "Model.h"

#include <QGraphicsItem>
#include <QRectF>

class BrowserPage;

/**
 * @brief The BrowserAnnotation class
 * 用于跟随视图框架变化的注释项
 */
class BrowserAnnotation : public QGraphicsItem
{
    friend class SheetBrowser;
public:
    explicit BrowserAnnotation(QGraphicsItem *parent, QRectF rect, deepin_reader::Annotation *annotation, qreal scalefactor);

    ~BrowserAnnotation() override;

    /**
     * @brief 为了让注释占比固定,视图改变时需要设置缩放因子
     * @param scaleFactor 缩放因子
     */
    void setScaleFactor(qreal scaleFactor);

    /**
     * @brief annotationType
     * 注释类型
     * @return
     */
    int annotationType();

    /**
     * @brief annotationText
     * 注释内容
     * @return
     */
    QString annotationText();

    /**
     * @brief boundingRect
     * 注释大小范围(无缩放和旋转时的范围)
     * @return
     */
    QRectF boundingRect()const override;

    /**
     * @brief annotation
     * 注释指针
     * @return
     */
    deepin_reader::Annotation *annotation();

    /**
     * @brief isSame
     * 操作注释是否与该注释相同
     * @param annotation
     * @return
     */
    bool isSame(deepin_reader::Annotation *annotation);

    /**
     * @brief setDrawSelectRect
     * 设置选择框
     * @param draw
     */
    void setDrawSelectRect(const bool draw);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    deepin_reader::Annotation *m_annotation;

    QRectF m_rect;

    QString m_text;

    QGraphicsItem *m_parent = nullptr;

    bool m_drawSelectRect = false;

    double m_scaleFactor = 1.0;
};

#endif // BrowserAnnotation_H
