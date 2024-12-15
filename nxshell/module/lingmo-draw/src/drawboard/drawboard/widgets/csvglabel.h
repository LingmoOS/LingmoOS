// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CSVGLABEL_H
#define CSVGLABEL_H

#include <QLabel>
#include <QSvgRenderer>
//#ifdef USE_DTK
//DWIDGET_USE_NAMESPACE
//#endif
class QWidget;

class CSvgLabel : public QLabel
{
    Q_OBJECT
public:
    CSvgLabel(const QString &str = QString(), QWidget *parent = nullptr);

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    QSvgRenderer m_svgRender;
};

#endif // CSVGLABEL_H
