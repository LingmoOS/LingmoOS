// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "memhiskeypad.h"

const MemHisKeypad::KeyDescription MemHisKeypad::keyDescriptions[] = {
    {"F-E", Key_FE, 1, 0, 1, 1},
    {"MC", Key_MC, 1, 1, 1, 1},       {"MR", Key_MR, 1, 2, 1, 1},
    {"M+", Key_Mplus, 1, 3, 1, 1},    {"M-", Key_Mmin, 1, 4, 1, 1},
    {"MS", Key_MS, 1, 5, 1, 1},       {"MH˅", Key_MHlist, 1, 6, 1, 1},
};

MemHisKeypad::MemHisKeypad(QWidget *parent)
    : DWidget(parent),
      m_layout(new QGridLayout(this)),
      m_mapper(new QSignalMapper(this))
{
    this->setFixedHeight(41);
    m_layout->setMargin(0);
    m_layout->setSpacing(3);  //按钮比ui大2pix,此处比ui小2pix
    m_layout->setContentsMargins(0, 0, 0, 0);

    initButtons();
    this->setContentsMargins(12, 0, 13, 0);

    connect(m_mapper, SIGNAL(mapped(int)), SIGNAL(buttonPressed(int)));
}

MemHisKeypad::~MemHisKeypad()
{
}

void MemHisKeypad::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

/**
 * @brief 根据枚举值返回相应button
 * @param key 按钮名
 */
DPushButton *MemHisKeypad::button(Buttons key)
{
    return m_keys.value(key).first;
}

/**
 * @brief 按钮点击动画效果
 */
void MemHisKeypad::animate(Buttons key, bool isspace)
{
    MemoryButton *btn = static_cast<MemoryButton *>(button(key));
    btn->animate(isspace);
}

bool MemHisKeypad::buttonHasFocus()
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
void MemHisKeypad::initButtons()
{
    const int count = sizeof(keyDescriptions) / sizeof(keyDescriptions[0]);
    for (int i = 0; i < count; ++i) {
        const KeyDescription *desc = keyDescriptions + i;
        DPushButton *button;
        if (desc->text == "F-E")
            button = new TextButton(desc->text, false, this);
        else
            button = new MemoryButton(desc->text, false, this);
        QFont font = button->font();
        font.setFamily("Noto Sans");
        button->setFont(font);

        if (desc->text == "MH˅")
            button->setFixedSize(69, 38);
        else
            button->setFixedSize(57, 38);
        m_layout->addWidget(button, desc->row, desc->column, desc->rowcount, desc->columncount,
                            Qt::AlignTop);
        const QPair<DPushButton *, const KeyDescription *> hashValue(button, desc);
        m_keys.insert(desc->button, hashValue); //key为枚举值，value.first为DPushButton *, value.second为const KeyDescription *

        connect(static_cast<TextButton *>(button), &TextButton::focus, this, &MemHisKeypad::getFocus); //获取上下左右键
        connect(button, &DPushButton::clicked, m_mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(static_cast<TextButton *>(button), &TextButton::space, this, [ = ]() {
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
void MemHisKeypad::getFocus(int direction)
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
        if ((i.key() - 52) % 7 > 0) {
            int enablekey = i.key() - 1;
            while (!button(static_cast<Buttons>(enablekey))->isEnabled() && enablekey > 52) {
                enablekey -= 1;
            }
            button(static_cast<Buttons>(enablekey))->setFocus();
        }
        break;
    case 3:
        if ((i.key() - 52) % 7 < 6) {
            int enablekey = i.key() + 1;
            while (!button(static_cast<Buttons>(enablekey))->isEnabled() && enablekey < 58) {
                enablekey += 1;
            }
            button(static_cast<Buttons>(enablekey))->setFocus();
        }
        break;
    default:
        break;
    }
}
