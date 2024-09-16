// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef BUTTONLABEL_H
#define BUTTONLABEL_H

#include <QWidget>
#include <QLabel>
#include <QVariant>

/**
 * @class ButtonLabel
 * @brief label按钮类
 */

class ButtonLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ButtonLabel(QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    /**
     * @brief 鼠标点击事件
     */
    virtual void mousePressEvent(QMouseEvent *event) override;

};

#endif // BUTTONLABEL_H
