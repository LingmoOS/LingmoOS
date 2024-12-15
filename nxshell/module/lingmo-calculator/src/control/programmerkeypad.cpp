// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "programmerkeypad.h"

const int KEYPAD_HEIGHT = 279; //键盘界面高度
const int KEYPAD_SPACING = 3; //键盘按键间距,按钮比ui大2pix,此处小2pix
const int LEFT_MARGIN = 10; //键盘左边距,按钮比ui大2pix,此处小2pix
const int RIGHT_MARGIN = 10; //键盘右边距,按钮比ui大2pix,此处小2pix
const int BOTTOM_MARGIN = 10; //键盘下边距,按钮比ui大2pix,此处小2pix
const QSize STANDARD_TEXTBTNSIZE = QSize(69, 42); //程序员模式按钮大小，为画边框比ui大2pix

const ProgrammerKeypad::KeyDescription ProgrammerKeypad::keyDescriptions[] = {
    {"AND", Key_AND, 0, 0},         {"A", Key_A, 0, 1},             {"<<", Key_moveL, 0, 2},
    {">>", Key_moveR, 0, 3},        {"C", Key_Clear, 0, 4},         {"", Key_Backspace, 0, 5},

    {"OR", Key_OR, 1, 0},           {"B", Key_B, 1, 1},             {"(", Key_leftBracket, 1, 2},
    {")", Key_rightBracket, 1, 3},  {"%", Key_percent, 1, 4},       {"", Key_Div, 1, 5},

    {"NOT", Key_NOT, 2, 0},         {"C", Key_C, 2, 1},             {"7", Key_7, 2, 2},
    {"8", Key_8, 2, 3},             {"9", Key_9, 2, 4},             {"", Key_Mult, 2, 5},

    {"NAND", Key_NAND, 3, 0},       {"D", Key_D, 3, 1},             {"4", Key_4, 3, 2},
    {"5", Key_5, 3, 3},             {"6", Key_6, 3, 4},             {"", Key_Min, 3, 5},

    {"NOR", Key_NOR, 4, 0},         {"E", Key_E, 4, 1},             {"1", Key_1, 4, 2},
    {"2", Key_2, 4, 3},             {"3", Key_3, 4, 4},             {"", Key_Plus, 4, 5},

    {"XOR", Key_XOR, 5, 0},         {"F", Key_F, 5, 1},             {"+/-", Key_opposite, 5, 2},
    {"0", Key_0, 5, 3},             {".", Key_point, 5, 4},         {"=", Key_equal, 5, 5}
};

ProgrammerKeypad::ProgrammerKeypad(QWidget *parent)
    : DWidget(parent),
      m_layout(new QGridLayout(this)),
      m_mapper(new QSignalMapper(this)),
      m_leftBracket(new DLabel(this)),
      m_rightBracket(new DLabel(this))
{
    this->setFixedHeight(KEYPAD_HEIGHT);
    m_layout->setMargin(0);
    m_layout->setSpacing(KEYPAD_SPACING);
    m_layout->setContentsMargins(0, 0, 0, 0);
//    setFocusPolicy(Qt::StrongFocus);
    m_leftBracket->setFixedSize(24, 14);
    m_rightBracket->setFixedSize(24, 14);

    installEventFilter(this);

    initButtons();
    initUI();

    connect(m_mapper, SIGNAL(mapped(int)), SIGNAL(buttonPressed(int)));
}

ProgrammerKeypad::~ProgrammerKeypad()
{

}

/**
 * @brief 防止鼠标点击拖拽可拖动窗口
 */
void ProgrammerKeypad::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

/**
 * @brief 根据枚举值返回相应button
 * @param key 按钮名
 */
DPushButton *ProgrammerKeypad::button(ProgrammerKeypad::Buttons key)
{
    return m_keys.value(key).first;
}

/**
 * @brief 按钮点击动画效果
 */
void ProgrammerKeypad::animate(ProgrammerKeypad::Buttons key, bool isspace)
{
    if (button(key)->text().isEmpty()) {
        IconButton *btn = static_cast<IconButton *>(button(key));
        btn->animate(isspace);
    } else {
        if (button(key)->text() == "=") {
            EqualButton *btn = dynamic_cast<EqualButton *>(button(key));
            btn->animate(isspace);
        } else {
            TextButton *btn = static_cast<TextButton *>(button(key));
            btn->animate(isspace);
        }
    }
}

/**
 * @brief 焦点是否在该button上
 */
bool ProgrammerKeypad::buttonHasFocus()
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
void ProgrammerKeypad::initButtons()
{
    const int count = sizeof(keyDescriptions) / sizeof(keyDescriptions[0]);
    for (int i = 0; i < count; ++i) {
        const KeyDescription *desc = keyDescriptions + i;
        DPushButton *button;

        if (desc->text.isEmpty()) {
            button = new IconButton(this);
        } else {
            if (desc->text == "=") {
                button = new EqualButton(desc->text, this);
                connect(static_cast<EqualButton *>(button), &EqualButton::focus, this, &ProgrammerKeypad::getFocus); //获取上下左右键
                connect(static_cast<EqualButton *>(button), &EqualButton::space, this, [ = ]() {
                    emit buttonPressedbySpace(Key_equal);
                });
            } else {
                if (i % 6 == 0)
                    button = new TextButton(desc->text, true, this);
                else
                    button = new TextButton(desc->text, false, this);
            }
        }
        if (i % 6 == 1) {
            m_decdisable << static_cast<TextButton *>(button);
            m_octdisable << static_cast<TextButton *>(button);
            m_bindisable << static_cast<TextButton *>(button);
        }
        if (i == 15 || i == 16) {
            m_octdisable << static_cast<TextButton *>(button);
            m_bindisable << static_cast<TextButton *>(button);
        }
        if (i == 14 || i == 20 || i == 21 || i == 22 || i == 27 || i == 28) {
            m_bindisable << static_cast<TextButton *>(button);
        }

        if (desc->text == "=")
            button->setFixedSize(69, 43);
        else
            button->setFixedSize(STANDARD_TEXTBTNSIZE);
        m_layout->addWidget(button, desc->row, desc->column, Qt::AlignHCenter | Qt::AlignVCenter);
        const QPair<DPushButton *, const KeyDescription *> hashValue(button, desc);
        m_keys.insert(desc->button, hashValue); //key为枚举值，value.first为DPushButton *, value.second为const KeyDescription *

        if (desc->text != "=") {
            connect(static_cast<TextButton *>(button), &TextButton::focus, this, &ProgrammerKeypad::getFocus); //获取上下左右键
            connect(static_cast<TextButton *>(button), &TextButton::space, this, [ = ]() {
                Buttons spacekey = m_keys.key(hashValue);
                emit buttonPressedbySpace(spacekey);
            });
        }
        connect(button, &DPushButton::clicked, m_mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        m_mapper->setMapping(button, desc->button); //多个按钮绑定到一个mapper上
    }
    static_cast<TextButton *>(this->button(Key_point))->setButtonGray(true);
    radixChanged(1);
}

/**
 * @brief 切换相应主题切图
 */
void ProgrammerKeypad::buttonThemeChanged(int type)
{
    m_themetype = type;
    QString path;
    if (type == 2)
        path = QString(":/assets/images/%1/").arg("dark");
    else
        path = QString(":/assets/images/%1/").arg("light");

    IconButton *btn = static_cast<IconButton *>(button(Key_Div));
    btn->setIconUrl(path + "divide_normal.svg", path + "divide_hover.svg", path + "divide_press.svg");
    btn = static_cast<IconButton *>(button(Key_Mult));
    btn->setIconUrl(path + "x_normal.svg", path + "x_hover.svg", path + "x_press.svg");
    btn = static_cast<IconButton *>(button(Key_Min));
    btn->setIconUrl(path + "-_normal.svg", path + "-_hover.svg", path + "-_press.svg");
    btn = static_cast<IconButton *>(button(Key_Plus));
    btn->setIconUrl(path + "+_normal.svg", path + "+_hover.svg", path + "+_press.svg");
    btn = static_cast<IconButton *>(button(Key_Backspace));
    btn->setIconUrl(path + "clear_normal.svg", path + "clear_hover.svg", path + "clear_press.svg", 1);
}

/**
 * @brief 获取button上下左右键信号setfocus
 * @param direction 0-上　1-下　2-左　3-右
 */
void ProgrammerKeypad::getFocus(int direction)
{
    QHashIterator<Buttons, QPair<DPushButton *, const KeyDescription *>> i(m_keys);
    while (i.hasNext()) {
        i.next();
        if (i.value().first->hasFocus()) {
            break; //获取焦点按钮
        }
    }
    switch (direction) {
    case 0:
        if (i.key() / 6 > 0) {
            for (int j = 1; i.key() >= 6 * j; j++) {
                if (button(static_cast<Buttons>(i.key() - 6 * j))->isEnabled()) {
                    button(static_cast<Buttons>(i.key() - 6 * j))->setFocus(); //根据上下左右信号重置焦点
                    break;
                }
            }
        }
        break;
    case 1:
        if (i.key() / 6 < 5) {
            for (int j = 1; i.key() + 6 * j < 36; j++) {
                if (button(static_cast<Buttons>(i.key() + 6 * j))->isEnabled()) {
                    button(static_cast<Buttons>(i.key() + 6 * j))->setFocus(); //根据上下左右信号重置焦点
                    break;
                }
            }
        }
        break;
    case 2:
        if (i.key() % 6 > 0) {
            for (int j = 1; (i.key() - j + 1) % 6 > 0; j++) {
                if (button(static_cast<Buttons>(i.key() - j))->isEnabled()) {
                    button(static_cast<Buttons>(i.key() - j))->setFocus(); //根据上下左右信号重置焦点
                    break;
                }
            }
        }
        break;
    case 3:
        if (i.key() % 6 < 5) {
            for (int j = 1; (i.key() + j - 1) % 6 < 5; j++) {
                if (button(static_cast<Buttons>(i.key() + j))->isEnabled()) {
                    button(static_cast<Buttons>(i.key() + j))->setFocus(); //根据上下左右信号重置焦点
                    break;
                }
            }
        }
        break;
    default:
        break;
    }
}

/**
 * @brief ProgrammerKeypad::radixChanged
 * @param row:进制列表的行数 0-16 1-10 2-8 3-2
 * 根据当前进制，屏蔽或开放数字键盘的对应按键
 */
void ProgrammerKeypad::radixChanged(int row)
{
    switch (row) {
    case 0:
        foreach (TextButton *button, m_bindisable) {
            if (!button->isEnabled())
                button->setEnabled(true);
        }
        m_currentBase = 16;
        break;
    case 1:
        foreach (TextButton *button, m_bindisable) {
            if (!button->isEnabled())
                button->setEnabled(true);
        }
        foreach (TextButton *button, m_decdisable) {
            if (button->isEnabled())
                button->setEnabled(false);
        }
        m_currentBase = 10;
        break;
    case 2:
        foreach (TextButton *button, m_bindisable) {
            if (!button->isEnabled())
                button->setEnabled(true);
        }
        foreach (TextButton *button, m_octdisable) {
            if (button->isEnabled())
                button->setEnabled(false);
        }
        m_currentBase = 8;
        break;
    case 3:
        foreach (TextButton *button, m_bindisable) {
            button->setEnabled(false);
        }
        m_currentBase = 2;
        break;
    default:
        foreach (TextButton *button, m_bindisable) {
            if (!button->isEnabled())
                button->setEnabled(true);
        }
        foreach (TextButton *button, m_decdisable) {
            if (button->isEnabled())
                button->setEnabled(false);
        }
        m_currentBase = 10;
        break;
    }
}

void ProgrammerKeypad::bracketsNum(int direction, QString num)
{
    if (num == "0")
        num = "";

    if (direction == 0)
        m_leftBracket->setText(num);
    else if (direction == 1)
        m_rightBracket->setText(num);
}

void ProgrammerKeypad::initUI()
{
    QHashIterator<Buttons, QPair<DPushButton *, const KeyDescription *>> i(m_keys);

    while (i.hasNext()) {
        i.next();
        if (i.key() == Key_leftBracket) {
            m_leftBracket->setParent(i.value().first);
            m_leftBracket->move(i.value().first->rect().x() + 37, i.value().first->rect().y() + 22);
            connect(i.value().first, &DPushButton::pressed, [ = ]() {
                m_leftBracket->setStyleSheet(QString("font-family:Noto Sans;color:%1;font-size:14px;")
                                             .arg(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name()));
            });
            connect(static_cast<TextButton *>(i.value().first), &TextButton::mouseRelease, [ = ]() {
                //考虑focus状态，在鼠标松开时再更改label字体颜色
                if (m_themetype == 1)
                    m_leftBracket->setStyleSheet(QString("font-family:Noto Sans;color:black;font-size:14px;"));
                else
                    m_leftBracket->setStyleSheet(QString("font-family:Noto Sans;color:white;font-size:14px;"));
            });
        }
        if (i.key() == Key_rightBracket) {
            m_rightBracket->setParent(i.value().first);
            m_rightBracket->move(i.value().first->rect().x() + 37, i.value().first->rect().y() + 22);
            connect(i.value().first, &DPushButton::pressed, [ = ]() {
                m_rightBracket->setStyleSheet(QString("font-family:Noto Sans;color:%1;font-size:14px;")
                                              .arg(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name()));
            });
            connect(static_cast<TextButton *>(i.value().first), &TextButton::mouseRelease, [ = ]() {
                if (m_themetype == 1)
                    m_rightBracket->setStyleSheet(QString("font-family:Noto Sans;color:black;font-size:14px;"));
                else
                    m_rightBracket->setStyleSheet(QString("font-family:Noto Sans;color:white;font-size:14px;"));
            });
        }
    }

    button(Key_Div)->setObjectName("SymbolButton");
    button(Key_Mult)->setObjectName("SymbolButton");
    button(Key_Min)->setObjectName("SymbolButton");
    button(Key_Plus)->setObjectName("SymbolButton");

    this->setContentsMargins(LEFT_MARGIN, 0, RIGHT_MARGIN, BOTTOM_MARGIN);
}
