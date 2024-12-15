// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "procheckbtnkeypad.h"

const QSize BUTTON_SIZE = QSize(69, 42); //程序员模式大小，为画边框比ui大2pix

const ProCheckBtnKeypad::KeyDescription ProCheckBtnKeypad::keyDescriptions[] = {
    {"", Key_GeneralKeypad, 0, 0},      {"", Key_BinaryKeypad, 0, 1},
    {"QWORD", Key_System, 0, 2},         {"", Key_Option, 0, 3},
    {"MS", Key_MS, 0, 4},               {"M˅", Key_Mlist, 0, 5}
};

ProCheckBtnKeypad::ProCheckBtnKeypad(QWidget *parent)
    : DWidget(parent),
      m_layout(new QGridLayout(this)),
      m_mapper(new QSignalMapper(this))
{
    this->setFixedHeight(45);
    m_layout->setMargin(0);
    m_layout->setSpacing(2);  //按钮比ui大2pix,此处比ui小2pix
    m_layout->setContentsMargins(0, 0, 0, 0);

    initButtons();
    this->setContentsMargins(10, 0, 10, 0);

    connect(m_mapper, SIGNAL(mapped(int)), SIGNAL(buttonPressed(int)));
}

ProCheckBtnKeypad::~ProCheckBtnKeypad()
{

}

void ProCheckBtnKeypad::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

/**
 * @brief 根据枚举值返回相应button
 * @param key 按钮名
 */
DPushButton *ProCheckBtnKeypad::button(ProCheckBtnKeypad::Buttons key)
{
    return m_keys.value(key).first;
}

/**
 * @brief 按钮点击动画效果
 */
void ProCheckBtnKeypad::animate(ProCheckBtnKeypad::Buttons key, bool isspace)
{
    MemoryButton *btn = static_cast<MemoryButton *>(button(key));
    btn->animate(isspace);
}

/**
 * @brief 焦点是否在该button上
 */
bool ProCheckBtnKeypad::buttonHasFocus()
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
void ProCheckBtnKeypad::initButtons()
{
    const int count = sizeof(keyDescriptions) / sizeof(keyDescriptions[0]);
    for (int i = 0; i < count; ++i) {
        const KeyDescription *desc = keyDescriptions + i;
        DPushButton *button;
        if (desc->text.isEmpty()) {
            button = new IconButton(this);
        } else if (desc->text == "QWORD") {
            button = new TextButton(desc->text, false, this);
        } else {
            button = new MemoryButton(desc->text, false, this);
        }

        button->setFixedSize(BUTTON_SIZE);
        m_layout->addWidget(button, desc->row, desc->column, Qt::AlignTop);
        const QPair<DPushButton *, const KeyDescription *> hashValue(button, desc);
        m_keys.insert(desc->button, hashValue); //key为枚举值，value.first为DPushButton *, value.second为const KeyDescription *

        connect(static_cast<TextButton *>(button), &TextButton::focus, this, &ProCheckBtnKeypad::getFocus); //获取上下左右键
        connect(button, &DPushButton::clicked, m_mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        connect(static_cast<TextButton *>(button), &TextButton::space, this, [ = ]() {
            emit buttonPressedbySpace(m_keys.key(hashValue));
        });
        m_mapper->setMapping(button, desc->button); //多个按钮绑定到一个mapper上
    }
    static_cast<IconButton *>(button(Key_GeneralKeypad))->setBtnHighlight(true);
}

/**
 * @brief ProCheckBtnKeypad::buttonThemeChanged
 * @param type：主题类型
 * @param index：当前需要高亮的图片 0-Key_GeneralKeypad 1-Key_BinaryKeypad
 * 切换主题时切换相应的svg图片
 */
void ProCheckBtnKeypad::buttonThemeChanged(int type)
{
    QString path;
    if (type == 2)
        path = QString(":/assets/images/%1/").arg("dark");
    else
        path = QString(":/assets/images/%1/").arg("light");

    IconButton *btn;
    btn = static_cast<IconButton *>(button(Key_GeneralKeypad));
    btn->setIconUrl(path + "icon_generalkeyboard_normal.svg", path + "icon_generalkeyboard_hover.svg", path + "icon_generalkeyboard_press.svg", 3);
    btn = static_cast<IconButton *>(button(Key_BinaryKeypad));
    btn->setIconUrl(path + "icon_binarysystem_normal.svg", path + "icon_binarysystem_hover.svg", path + "icon_binarysystem_press.svg", 3);
    btn = static_cast<IconButton *>(button(Key_Option));
    if (btn->getIconUrl().at(0) != "icon_as_iconnormal.svg" && btn->getIconUrl().at(0) != QString()) {
        btn->setIconUrl(path + btn->getIconUrl().at(0),
                        path + btn->getIconUrl().at(1),
                        path + btn->getIconUrl().at(2), 5);
    } else {
        btn->setIconUrl(path + "icon_as_iconnormal.svg", path + "icon_as_iconhover.svg", path + "icon_as_iconhover.svg", 5);
    }
}

/**
 * @brief 获取button上下左右键信号setfocus
 * @param direction 0-上　1-下　2-左　3-右
 */
void ProCheckBtnKeypad::getFocus(int direction)
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
        if (i.key() % 6 > 0)
            button(static_cast<Buttons>(i.key() - 1))->setFocus();
        break;
    case 3:
        if (i.key() % 6 < 5)
            button(static_cast<Buttons>(i.key() + 1))->setFocus();
        break;
    default:
        break;
    }
}
