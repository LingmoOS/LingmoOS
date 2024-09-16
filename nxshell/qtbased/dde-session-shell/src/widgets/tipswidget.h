// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIPSWIDGET_H
#define TIPSWIDGET_H

#include <DBlurEffectWidget>
#include <QPointer>

class TipsWidget : public Dtk::Widget::DBlurEffectWidget
{
public:
    explicit TipsWidget(QWidget *parent = nullptr);
    void setContent(QWidget *content);
    inline QWidget *getContent() const { return m_content; }
    void resizeFromContent();

public slots:
    /*!
     * @brief slot to show the tips widget, x and y is for bottom center point of the widget
     */
    void show(int x, int y);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

protected:
    QPoint m_lastPos;
    QPointer<QWidget> m_content;
};

#endif // TIPSWIDGET_H
