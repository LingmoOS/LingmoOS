// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CNUMBERSLIDERWIDGET_H
#define CNUMBERSLIDERWIDGET_H


#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>

#include "globaldefine.h"
#include "adaptivedef.h"

#ifdef USE_DTK
#include <DLineEdit>
#include <DSlider>
DWIDGET_USE_NAMESPACE
#endif
class DRAWLIB_EXPORT NumberSlider : public QWidget
{
    Q_OBJECT
public:
    explicit NumberSlider(QWidget *parent = nullptr);
    explicit NumberSlider(const QString &text, QWidget *parent = nullptr);

    void setValue(int value);
    int  value() const;

    void setRange(int min, int max);
    int min()const;
    int max()const;

    void setText(const QString &str);
    QString text()const;

    void setTextVisiable(bool b);
    bool isTextVisiable()const;

    void setFlagDiameter(uint diameter);
    uint flagDiameter()const;

    void setTickInterval(int t);
    void setPix(QPixmap pix);

signals:
    void valueChanged(int value, EChangedPhase phase);

private:
    PRIVATECLASS(NumberSlider)
};


#endif // CNUMBERSLIDERWIDGET_H
