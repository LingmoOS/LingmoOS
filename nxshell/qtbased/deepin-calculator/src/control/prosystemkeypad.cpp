// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "prosystemkeypad.h"

#include <QDebug>
#include "../../3rdparty/core/settings.h"

const QSize SYSTEMKEYPADSIZE = QSize(451, 279);
const int LEFT_MARGIN = 10; //键盘左边距
const int BOTTOM_MARGIN = 14; //键盘下边距

ProSystemKeypad::ProSystemKeypad(QWidget *parent)
    : DWidget(parent),
      m_layout(new QGridLayout(this))
{
    this->setFocusPolicy(Qt::ClickFocus);
    this->setFixedSize(SYSTEMKEYPADSIZE);
    m_layout->setContentsMargins(LEFT_MARGIN, 0, 0, BOTTOM_MARGIN);
    initUI();
    m_binaryValue.fill('0', 64);
}

ProSystemKeypad::~ProSystemKeypad()
{

}

/**
 * @brief 设置当前显示位数
 * @param system 64, 32, 16, 8
 */
void ProSystemKeypad::setSystem(int system, int oldsystem)
{
    QHashIterator<int, ProBitWidget *> i(m_keys);
    if (system == 64) {
        while (i.hasNext()) {
            i.next();
            i.value()->setButtonEnabled(true);
        }
        if (oldsystem < system && m_binaryValue.at(64 - oldsystem) == '1') {
            for (int j = 63; j >= oldsystem; j--) {
                m_buttons.value(j)->setButtonState(true);
                m_binaryValue.replace(63 - j, 1, "1");
            }
            emit valuechanged(m_binaryValue);
        }
    } else if (system == 32) {
        while (i.hasNext()) {
            i.next();
            if (i.key() >= 32)
                i.value()->setButtonEnabled(false);
            else
                i.value()->setButtonEnabled(true);
        }
        if (oldsystem < system && m_binaryValue.at(64 - oldsystem) == '1') {
            for (int j = 31; j >= oldsystem; j--) {
                m_buttons.value(j)->setButtonState(true);
                m_binaryValue.replace(63 - j, 1, "1");
            }
            emit valuechanged(m_binaryValue);
        }
        if (oldsystem > system) {
            bool isdiff = m_binaryValue.contains("1");
            for (int j = oldsystem - 1; j > 31; j--) {
                m_buttons.value(j)->setButtonState(false);
                m_binaryValue.replace(63 - j, 1, "0");
            }
            if (isdiff)
                emit valuechanged(m_binaryValue);
        }
    } else if (system == 16) {
        while (i.hasNext()) {
            i.next();
            if (i.key() >= 16)
                i.value()->setButtonEnabled(false);
            else
                i.value()->setButtonEnabled(true);
        }
        if (oldsystem < system && m_binaryValue.at(64 - oldsystem) == '1') {
            for (int j = 15; j >= oldsystem; j--) {
                m_buttons.value(j)->setButtonState(true);
                m_binaryValue.replace(63 - j, 1, "1");
            }
            emit valuechanged(m_binaryValue);
        }
        if (oldsystem > system) {
            bool isdiff = m_binaryValue.contains("1");
            for (int j = oldsystem - 1; j > 15; j--) {
                m_buttons.value(j)->setButtonState(false);
                m_binaryValue.replace(63 - j, 1, "0");
            }
            if (isdiff)
                emit valuechanged(m_binaryValue);
        }
    } else {
        while (i.hasNext()) {
            i.next();
            if (i.key() < 8)
                i.value()->setButtonEnabled(true);
            else
                i.value()->setButtonEnabled(false);
        }
        if (oldsystem > system) {
            bool isdiff = m_binaryValue.contains("1");
            for (int j = oldsystem - 1; j > 7; j--) {
                m_buttons.value(j)->setButtonState(false);
                m_binaryValue.replace(63 - j, 1, "0");
            }
            if (isdiff)
                emit valuechanged(m_binaryValue);
        }
    }
    //    qDebug() << m_binaryValue;
}

bool ProSystemKeypad::buttonHasFocus()
{
    QHashIterator<int, BitButton *> i(m_buttons);
    while (i.hasNext()) {
        i.next();
        if (i.value()->hasFocus()) {
            return true;
        }
    }
    return false;
}

void ProSystemKeypad::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

/**
 * @brief ProSystemKeypad::getFocus
 * @param dir:键盘方向键方向
 * @param key:按键的序号
 * 处理bitbutton的focus事件s
 */
void ProSystemKeypad::getFocus(int dir, int key)
{
    switch (dir) {
    case 0:
        if (key < 48 && m_buttons.value(key + 16)->isEnabled())
            m_buttons.value(key + 16)->setFocus();
        break;
    case 1:
        if (key > 15 && m_buttons.value(key - 16)->isEnabled())
            m_buttons.value(key - 16)->setFocus();
        break;
    case 2:
        if (key < 63 && m_buttons.value(key + 1)->isEnabled())
            m_buttons.value(key + 1)->setFocus();
        break;
    case 3:
        if (key > 0 && m_buttons.value(key - 1)->isEnabled())
            m_buttons.value(key - 1)->setFocus();
        break;
    default:
        break;
    }
}

/**
 * @brief ProSystemKeypad::setvalue
 * @param num:当前输入栏中光标所在位置对应的数字
 * 当输入栏中输入或修改数字时，修改对应的bitbutton的状态
 */
void ProSystemKeypad::setvalue(QString num)
{
    num.remove(" ").remove(",");
    int length = num.length();
    if (length > Settings::instance()->proBitLength) {
        num = num.right(Settings::instance()->proBitLength);
        length = num.length();
        m_binaryValue.fill('0', 64).replace(64 - length, length, num);
        emit valuechanged(m_binaryValue);
        longBitCut(num);
    } else
        m_binaryValue.fill('0', 64).replace(64 - length, length, num);
//    qDebug() << m_binaryValue;
    for (int i = 0; i < Settings::instance()->proBitLength; i++) {
        if (m_binaryValue.at(63 - i) == '1') {
            m_buttons.value(i)->setButtonState(true);
        } else
            m_buttons.value(i)->setButtonState(false);
    }
}

void ProSystemKeypad::longBitCut(const QString &num)
{
    QString number = "0b" + num;
    Quantity ans(HNumber(number.toLatin1().data()));
    if (!ans.isNan())
        emit longbitcut(ans);
}

void ProSystemKeypad::initUI()
{
    for (int i = 0; i < 16; i++) {
        ProBitWidget *probitwidget = new ProBitWidget(this);
        probitwidget->findChild<DLabel *>()->setText(QString::number(60 - i * 4, 10));
        m_layout->addWidget(probitwidget, i / 4, i % 4, Qt::AlignHCenter | Qt::AlignVCenter);

        //每块ProBitWidget对应一个key
        m_keys.insert(60 - i * 4, probitwidget);
        for (int j = 0; j < 4; j++) {
            m_buttons.insert(63 - i * 4 - j, probitwidget->getbutton(j));
        }
    }
    initconnects();
}

void ProSystemKeypad::initconnects()
{
    for (int i = 0; i < 64; i++) {
        connect(m_buttons.value(i), &DPushButton::clicked, this, [ = ]() {
            emit bitbuttonclicked();
            changeBinaryValue(i);
        });
        connect(m_buttons.value(i), &BitButton::focus, this, [ = ](int direction) {
            getFocus(direction, i);
        }); //获取上下左右键
        connect(m_buttons.value(i), &BitButton::space, this, [ = ]() {
            if (m_buttons.value(i)->text() == QString("0"))
                m_buttons.value(i)->setButtonState(true);
            else
                m_buttons.value(i)->setButtonState(false);
            changeBinaryValue(i);
        });
    }
}

/**
 * @brief ProSystemKeypad::changeBinaryValue
 * @param i:按键序号
 * 修改位键盘对应的2进制的值并传给输入框
 */
void ProSystemKeypad::changeBinaryValue(int i)
{
    if (m_binaryValue.at(63 - i) == '0')
        m_binaryValue.replace(63 - i, 1, '1');
    else
        m_binaryValue.replace(63 - i, 1, '0');
//    qDebug() << m_binaryValue;
    emit valuechanged(m_binaryValue);
}
