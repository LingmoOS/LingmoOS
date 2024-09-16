// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CSPINBOX_H
#define CSPINBOX_H

#include <QWidget>
#include "globaldefine.h"

#ifdef USE_DTK
#include <DSpinBox>
#define CSPINBOXFATHERCLASS DSpinBox
DWIDGET_USE_NAMESPACE
#else
#include <QSpinBox>
#define CSPINBOXFATHERCLASS QSpinBox
#endif

class QTimer;

class CSpinBox : public CSPINBOXFATHERCLASS
{
    Q_OBJECT
public:
    explicit CSpinBox(QWidget *parent = nullptr);

    bool isTimerRunning();

    bool isChangedByWheelEnd();

    void setValueChangedKeepFocus(bool b);

    void setSpinRange(int min, int max);

    void setSpecialText(QString sp = "— —");

    void setSpinPhaseValue(int value, EChangedPhase phase);

    void setSpinValue(int value);

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void showEvent(QShowEvent *event) override;

    void keyPressEvent(QKeyEvent *event)override;
    void keyReleaseEvent(QKeyEvent *event)override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    bool eventFilter(QObject *o, QEvent *e) override;

    Q_SLOT void timerStart();
    Q_SLOT void timerEnd();

signals:
    void focusChanged(bool isFocus);

    void valueChanged(int value, EChangedPhase phase);

private:
    QTimer *getTimer();

    void updateMaxSize();

public:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QTimer *_wheelTimer = nullptr;
    bool    _wheelbegin = false;
    bool    _wheelEnd   = false;
    bool    _keepFocus = false;

    int _s_value = -100;
    int _s_phase = -100;

    int m_min = -1;
    int m_max = INT_MAX;



    //QSize _sizeHit;
};

#endif // CSPINBOX_H
