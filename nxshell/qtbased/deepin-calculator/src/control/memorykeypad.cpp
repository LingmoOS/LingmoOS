// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "memorykeypad.h"

const QSize MEMORYBUTTON_SIZE = QSize(52, 32); //标准模式大小，为画边框比ui大2pix

const MemoryKeypad::KeyDescription MemoryKeypad::keyDescriptions[] = {
    {"MC", Key_MC, 1, 0, 1, 1},       {"MR", Key_MR, 1, 1, 1, 1},
    {"M+", Key_Mplus, 1, 2, 1, 1},    {"M-", Key_Mminus, 1, 3, 1, 1},
    {"MS", Key_MS, 1, 4, 1, 1},       {"M˅", Key_Mlist, 1, 5, 1, 1},
};

MemoryKeypad::MemoryKeypad(QWidget *parent)
    : DWidget(parent),
      m_layout(new QGridLayout(this)),
      m_mapper(new QSignalMapper(this))
{
    this->setFixedHeight(35);
    m_layout->setMargin(0);
    m_layout->setSpacing(2);  //按钮比ui大2pix,此处比ui小2pix
    m_layout->setContentsMargins(0, 0, 0, 0);

    initButtons();
    this->setContentsMargins(12, 0, 13, 0);

    connect(m_mapper, SIGNAL(mapped(int)), SIGNAL(buttonPressed(int)));
}

MemoryKeypad::~MemoryKeypad()
{
}

void MemoryKeypad::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

/**
 * @brief 根据枚举值返回相应button
 * @param key 按钮名
 */
DPushButton *MemoryKeypad::button(Buttons key)
{
    return m_keys.value(key).first;
}

DPushButton *MemoryKeypad::button(int key)
{
    return m_keys.value(Buttons(key)).first;
}

/**
 * @brief 按钮点击动画效果
 */
void MemoryKeypad::animate(Buttons key, bool isspace)
{
    MemoryButton *btn = static_cast<MemoryButton *>(button(key));
    btn->animate(isspace);
}

/**
 * @brief 焦点是否在该button上
 */
bool MemoryKeypad::buttonHasFocus()
{
    QHashIterator<Buttons, QPair<DPushButton *, const KeyDescription *>> i(m_keys);
    while (i.hasNext()) {
        i.next();
        if (i.value().first->hasFocus()) {
            return true;
        }
    }
    return false;
}

/**
 * @brief 初始化button
 */
void MemoryKeypad::initButtons()
{
    const int count = sizeof(keyDescriptions) / sizeof(keyDescriptions[0]);
    for (int i = 0; i < count; ++i) {
        const KeyDescription *desc = keyDescriptions + i;
        DPushButton *button;
        button = new MemoryButton(desc->text, false, this);
        QFont font = button->font();
        font.setFamily("Noto Sans");
        button->setFont(font);

        button->setFixedSize(MEMORYBUTTON_SIZE);
        m_layout->addWidget(button, desc->row, desc->column, desc->rowcount, desc->columncount,
                            Qt::AlignTop);
        const QPair<DPushButton *, const KeyDescription *> hashValue(button, desc);
        m_keys.insert(desc->button, hashValue); //key为枚举值，value.first为DPushButton *, value.second为const KeyDescription *

        connect(static_cast<MemoryButton *>(button), &MemoryButton::focus, this, &MemoryKeypad::getFocus); //获取上下左右键
        connect(button, &DPushButton::clicked, m_mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(static_cast<MemoryButton *>(button), &MemoryButton::space, this, [ = ]() {
            Buttons spacekey = m_keys.key(hashValue);
            emit buttonPressedbySpace(spacekey);
        });
        m_mapper->setMapping(button, desc->button); //多个按钮绑定到一个mapper上
    }
}

/**
 * @brief 获取button上下左右键信号setfocus
 * @param direction 0-上　1-下　2-左　3-右
 */
void MemoryKeypad::getFocus(int direction)
{
    QHashIterator<Buttons, QPair<DPushButton *, const KeyDescription *>> i(m_keys);
    while (i.hasNext()) {
        i.next();
        if (i.value().first->hasFocus()) {
            break; //获取焦点按钮
        }
    }
    switch (direction) {
    case 2:
        if ((i.key() - 20) % 6 > 0)
            button(static_cast<Buttons>(i.key() - 1))->setFocus();
        break;
    case 3:
        if ((i.key() - 20) % 6 < 5)
            button(static_cast<Buttons>(i.key() + 1))->setFocus();
        break;
    default:
        break;
    }
}
