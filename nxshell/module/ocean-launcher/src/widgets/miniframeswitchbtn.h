// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MINIFRAMESWITCHBTN_H
#define MINIFRAMESWITCHBTN_H

#include "roundedbutton.h"
#include <QWidget>
#include <QLabel>

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

class MiniFrameSwitchBtn : public QWidget
{
    Q_OBJECT

public:
    explicit MiniFrameSwitchBtn(QWidget *parent = nullptr);

    void updateStatus(int status);
    void click();

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
    void updateIcon();

private:
    QLabel *m_textLabel;
    QLabel *m_enterIcon;
    QLabel *m_allIconLabel;
    bool m_hover = false;
    QColor m_color;
};

#endif
