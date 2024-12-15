// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cspinbox.h"

#include <QLineEdit>
#include <QTimer>
#include <QKeyEvent>
#include <QDebug>


//#include "frame/cgraphicsview.h"
//#include "application.h"

CSpinBox::CSpinBox(QWidget *parent)
    : CSPINBOXFATHERCLASS(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    /*    if (Application::isTabletSystemEnvir()) {
            lineEdit()->setReadOnly(true);
            setEnabledEmbedStyle(false);
            setButtonSymbols(PlusMinus);
            setMaximumHeight(36);
        } else */{
        //setEnabledEmbedStyle(true);
        setButtonSymbols(UpDownArrows);
        setMaximumSize(100, 36);
    }

    connect(this, QOverload<int>::of(&QSpinBox::valueChanged), this, [ = ](int value) {
        setSpinPhaseValue(value, isTimerRunning() ? EChangedUpdate : EChanged);
        updateMaxSize();
    });

    //setValueChangedKeepFocus(true);

    setKeyboardTracking(false);

    setRange(-INT_MAX, INT_MAX);

    lineEdit()->setAlignment(Qt::AlignCenter);
    lineEdit()->installEventFilter(this);
}

bool CSpinBox::isTimerRunning()
{
    if (_wheelTimer != nullptr) {
        return _wheelTimer->isActive();
    }
    return false;
}

bool CSpinBox::isChangedByWheelEnd()
{
    return _wheelEnd;
}

void CSpinBox::setValueChangedKeepFocus(bool b)
{
    _keepFocus = b;
}

void CSpinBox::setSpinRange(int min, int max)
{
    m_min = min;
    m_max = max;
}

void CSpinBox::setSpecialText(QString sp)
{
    this->blockSignals(true);
    this->setValue(this->minimum());
    this->setSpecialValueText(sp);
    this->blockSignals(false);
}

void CSpinBox::focusInEvent(QFocusEvent *event)
{
//    if (CManageViewSigleton::GetInstance()->getCurView() != nullptr)
//        CManageViewSigleton::GetInstance()->getCurView()->disableCutShortcut(true);
    QSpinBox::focusInEvent(event);
}

void CSpinBox::focusOutEvent(QFocusEvent *event)
{
//    if (CManageViewSigleton::GetInstance()->getCurView() != nullptr)
//        CManageViewSigleton::GetInstance()->getCurView()->disableCutShortcut(false);
    QSpinBox::focusOutEvent(event);
}

void CSpinBox::wheelEvent(QWheelEvent *event)
{
    if (hasFocus()) {
        //启动定时器
        timerStart();

        QSpinBox::wheelEvent(event);
    }
}

void CSpinBox::showEvent(QShowEvent *event)
{
    setValueChangedKeepFocus(true);

    updateMaxSize();

    QSpinBox::showEvent(event);
}

void CSpinBox::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) {
        //启动定时器
        timerStart();
    } else if (event->key() == Qt::Key_Minus) {
        //不允许负数值设置
        return;
    }
    QSpinBox::keyPressEvent(event);
    event->accept();
    setFocus();
}

void CSpinBox::keyReleaseEvent(QKeyEvent *event)
{
    //timerEnd();
    QSpinBox::keyReleaseEvent(event);
    event->accept();
    setFocus();
}

void CSpinBox::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event)
    // [41555] spinbox 触摸长按会弹出菜单导致焦点丢失，此处截获弹出菜单事件
}

void CSpinBox::mouseDoubleClickEvent(QMouseEvent *event)
{
    // [BUG:45463] 选中两个不同锚点的星形，点击锚点减小按钮，能切换窗口大小
    QSpinBox::mouseDoubleClickEvent(event);
    event->accept();
}

bool CSpinBox::eventFilter(QObject *o, QEvent *e)
{
    if (lineEdit() == o) {
        if (e->type() == QEvent::FontChange) {
            updateMaxSize();
        }
    }
    return QSpinBox::eventFilter(o, e);
}

void CSpinBox::timerEnd()
{
    //先结束timer(标志着isTimerRunning返回false)
    if (_wheelTimer != nullptr) {
        _wheelTimer->deleteLater();
        _wheelTimer = nullptr;
    }

    //wheel结束时的值变化标记
    _wheelEnd = true;
    //emit this->valueChanged(value(), EChangedFinished);
    setSpinPhaseValue(value(), EChangedFinished);
    _wheelEnd = false;
    setFocus();
}

QTimer *CSpinBox::getTimer()
{
    if (_wheelTimer == nullptr) {
        _wheelTimer = new QTimer(this);
        _wheelTimer->setSingleShot(true);
        connect(_wheelTimer, &QTimer::timeout, this, &CSpinBox::timerEnd);
    }
    return _wheelTimer;
}

void CSpinBox::setSpinPhaseValue(int value, EChangedPhase phase)
{
    if (_s_value != value || _s_phase != phase) {
        _s_value = value;
        _s_phase = phase;

        if (_s_value < m_min) {
            _s_value = m_min;
        } else if (value > m_max) {
            _s_value = m_max;
        }
        this->blockSignals(true);
        setValue(_s_value);
        this->blockSignals(false);
        emit valueChanged(_s_value, EChangedPhase(_s_phase));
    }
}

void CSpinBox::setSpinValue(int value)
{
    if (_s_value != value) {
        _s_value = value;

        if (_s_value < m_min) {
            _s_value = m_min;
        } else if (value > m_max) {
            _s_value = m_max;
        }
    }
    setValue(value);
}

void CSpinBox::updateMaxSize()
{
//    if (Application::isTabletSystemEnvir()) {
//        const int c_MinWidth = 120;
//        int w = lineEdit()->fontMetrics().width(lineEdit()->text());
//        setMaximumWidth(w + c_MinWidth);
//    }
}

QSize CSpinBox::sizeHint() const
{
    return QSpinBox::sizeHint();
}

QSize CSpinBox::minimumSizeHint() const
{
    return QSpinBox::minimumSizeHint();
}

void CSpinBox::timerStart()
{
    if (!isTimerRunning()) {
        //emit this->valueChanged(value(), EChangedBegin);
        setSpinPhaseValue(value(), EChangedBegin);
    }
    getTimer()->start(300);
}
