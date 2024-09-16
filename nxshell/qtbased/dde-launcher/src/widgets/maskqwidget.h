// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MASKQWIDGET_H
#define MASKQWIDGET_H

#include <QWidget>

class MaskQWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MaskQWidget(QWidget *parent = nullptr);

    void setColor(QColor color);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QColor m_color;

signals:

public slots:
};

#endif // MASKQWIDGET_H
