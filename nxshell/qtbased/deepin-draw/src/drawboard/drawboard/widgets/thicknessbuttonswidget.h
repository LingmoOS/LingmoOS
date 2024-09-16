// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THICKNESSBUTTONSWIDGET_H
#define THICKNESSBUTTONSWIDGET_H

#include "globaldefine.h"

#include <QWidget>
#include <QPushButton>


class circleIconPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit circleIconPushButton(int penWidth, QWidget *parent = nullptr);
    int penWidth();
    virtual QSize sizeHint() const override;
protected:
    void paintEvent(QPaintEvent *);
private:
    int m_penWidth;
};

class DRAWLIB_EXPORT ThicknessButtonsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThicknessButtonsWidget(QWidget *parent = nullptr);
    void setText(const QString &str);
    void add(int lineWidth);
    //void addDefaultButtons();
    void removeAllButtons();
    void addButtons(int *arr, int count);
    void setTextVisible(bool bVisible);

    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;
private:
    void InitLayout();
signals:
    void valueChanged(int value, EChangedPhase phase);
public slots:
    void setValue(int value);

private:
    PRIVATECLASS(ThicknessButtonsWidget)

};

#endif // THICKNESSBUTTONSWIDGET_H
