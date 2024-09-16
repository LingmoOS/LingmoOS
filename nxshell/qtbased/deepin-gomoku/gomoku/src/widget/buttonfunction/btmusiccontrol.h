// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BTMUSICCONTROL_H
#define BTMUSICCONTROL_H

#include "buttonitem.h"

class BTMusicControl : public ButtonItem
{
    Q_OBJECT
public:
    explicit BTMusicControl(QGraphicsItem *parent = nullptr);
    ~BTMusicControl() override;
    void setNotFirstGame() override;

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void buttonFunction() override;

    /**
     * @brief setElidedText 字符串太长时截断并且显示tipTool
     * @param text 需要显示的字符串
     * @param fontMetrics 字体度量器
     */
//    void setElidedText(QString &text, QFontMetrics &fontMetrics, const int textWidth) override;

signals:
    void signalMusic(bool musicControl);

private:
    QPixmap voicePixmap; //声音开启图片
    QPixmap voiceOffPixmap; //声音关闭图片
    //以下位置参数,根据UI图得来
//    const qreal musicTextPosWidth = 0.2941; //按钮文字位置占整个scene宽度比例
    const qreal musicTextPosWidth = 0.3625; //按钮文字位置占整个scene宽度比例
    const qreal buttonMusicControlPosHeight = 0.765; //音乐功能按钮位置占整个scene高度比例
};

#endif // BTMUSICCONTROL_H
