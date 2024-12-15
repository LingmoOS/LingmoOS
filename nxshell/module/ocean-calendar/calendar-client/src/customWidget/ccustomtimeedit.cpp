// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ccustomtimeedit.h"

#include <QMouseEvent>
#include <QLineEdit>
#include <QKeyEvent>

CCustomTimeEdit::CCustomTimeEdit(QWidget *parent)
    : QTimeEdit(parent)
{
    //设置edit最大宽度，不影响其他控件使用
    setMaximumWidth(80);
    setButtonSymbols(QTimeEdit::NoButtons);
}

/**
 * @brief CCustomTimeEdit::getLineEdit      获取编辑框
 * @return
 */
QLineEdit *CCustomTimeEdit::getLineEdit()
{
    return lineEdit();
}

void CCustomTimeEdit::focusInEvent(QFocusEvent *event)
{
    QTimeEdit::focusInEvent(event);
    emit signalUpdateFocus(true);
}

void CCustomTimeEdit::focusOutEvent(QFocusEvent *event)
{
    QTimeEdit::focusOutEvent(event);
    emit signalUpdateFocus(false);
}

void CCustomTimeEdit::mousePressEvent(QMouseEvent *event)
{
    //设置父类widget焦点
    if (parentWidget() != nullptr) {
        parentWidget()->setFocus(Qt::TabFocusReason);
    }
    //设置点击位置的光标
    lineEdit()->setCursorPosition(lineEdit()->cursorPositionAt(event->pos()));
    QAbstractSpinBox::mousePressEvent(event);
}

void CCustomTimeEdit::keyPressEvent(QKeyEvent *event)
{
    QTimeEdit::keyPressEvent(event);
    //鼠标左右键,切换光标位置
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
        if (parentWidget() != nullptr) {
            parentWidget()->setFocus(Qt::TabFocusReason);
        }
        lineEdit()->setCursorPosition(currentSectionIndex());
    }
}

void CCustomTimeEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
    QTimeEdit::mouseDoubleClickEvent(event);
    //鼠标双击,选中section
    setSelectedSection(currentSection());
}
