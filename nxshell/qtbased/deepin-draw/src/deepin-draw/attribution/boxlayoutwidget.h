// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HBOXLAYOUTWIDGET_H
#define HBOXLAYOUTWIDGET_H

#include <QWidget>

class QBoxLayout;
class BoxLayoutWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoxLayoutWidget(QWidget *parent = nullptr);
    void addWidget(QWidget *w, int stretch = 0);
    void resetLayout(QBoxLayout *l);
    void setContentsMargins(int left, int top, int right, int bottom);
    void setMargins(int value);
    void setFrameRect(bool roundedrect);
    void setBackgroundRect(bool roundedrect);
    void setColor(QColor c);
    void setAlphaF(qreal alphaf);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
private:
    void init();
signals:

private:
    QColor  m_color;//背景色
    QBoxLayout  *m_layout = nullptr;
    bool m_framerect = true;//绘制边框标志
    bool m_backgroundrect = false;//绘制背景标志
};

#endif // HBOXLAYOUTWIDGET_H
