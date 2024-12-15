// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CRADIOBUTTON_H
#define CRADIOBUTTON_H

#include <QRadioButton>

//自定义颜色单选按钮
class CRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    explicit CRadioButton(QWidget *parent = nullptr);

    void setColor(const QColor&);
    QColor getColor();

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_color;
};

#endif // CRADIOBUTTON_H
