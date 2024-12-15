// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QFrame>

class BackgroundWidget : public QFrame
{
    Q_OBJECT
public:
    enum RoundRole {
        NoRole,
        Top,
        Bottom,
        TopAndBottom
    };

    enum ColorType {
        NoType,
        ItemBackground   //列表项的背景色
    };

    explicit BackgroundWidget(QWidget *parent = nullptr);

    void setBackground(int radius, ColorType colorType, RoundRole role);

protected:
    void paintEvent(QPaintEvent *event) override;
    QColor backgroundColor();

private:
    int radius { 0 };
    ColorType colorType { NoType };
    RoundRole roundRole { NoRole };
};

#endif   // BACKGROUNDWIDGET_H
