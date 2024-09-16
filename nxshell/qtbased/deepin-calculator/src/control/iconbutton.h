// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include "textbutton.h"

#include <DLabel>
#include <DGuiApplicationHelper>

#include <QPaintEvent>
#include <QDomElement>
#include <QSvgRenderer>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class IconButton : public TextButton
{
    Q_OBJECT

public:
    /**
     * @brief IconButton
     * @param parent
     * @param b 1-垃圾桶 2-科学计算器titlebar历史区按钮
     * @param page 科学模式第二页的IconButton
     */
    IconButton(QWidget *parent = nullptr, int b = 0, bool page = false);
    ~IconButton() override;

    void setIconUrl(const QString &normalFileName, const QString &hoverFileName, const QString &pressFileName, int mode = 0);
    QStringList getIconUrl() const;
    void animate(bool isspace = false, int msec = 100);
    void showtooltip(bool ismemory); //ismemory-true-内存区垃圾桶 false-历史区垃圾桶
    void updateWhenBtnDisable();
    void setBtnPressing(bool press);
    void setBtnHighlight(bool light);

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    //void setIconSize(const int &size);

//    DLabel *m_iconWidget;
    QGraphicsDropShadowEffect *m_effect;

//    DSvgRenderer *m_iconRenderer;
    QSvgRenderer *m_iconRenderer; //改用QSvgRenderer

    bool m_isHover;
    bool m_isPress;
    QPixmap m_pixmap;
    QString m_normalUrl = QString();
    QString m_hoverUrl;
    QString m_pressUrl;
    int m_mode = 0; //0-四则运算符 1-退格符 2-退格符press 3-根号 4-根号press 5-deg 6-degpress
    QString m_currentUrl;
    int m_buttonStatus = 0; //0-normal 1-hover 2-press
    bool m_isEmptyBtn; //内存中的清空按钮
//    int m_themetype;
    bool m_page;
    bool m_isHistorybtn = false; //打开历史记录的按钮，暂时保存
    QString m_cleartooltip = QString();
    bool m_isacting = false; //规避鼠标在button内release时出现foucus状态的框
    bool m_isPressing = false; //对于两个rectangle的特殊按钮，点击后需要持续显示press状态
    bool m_highlight = false; //用于设置程序员模式下全键盘/位键盘切换持续高亮
private:
    void SetAttrRecur(QDomElement elem, QString strtagname, QString strattr, QString strattrval);
    void drawCenterPixMap(QPainter &painter);
signals:
    void isclicked();
};

#endif
