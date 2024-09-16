// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scientifickeypad.h"

#include <QTimer>
#include <DPalette>
#include <DImageButton>
#include <DGuiApplicationHelper>

#include "dthememanager.h"

const int KEYPADHEIGHT = 350; //整体键盘的高度
const QSize BUTTON_SIZE = QSize(69, 46); //科学模式的固定大小

const ScientificKeyPad::KeyDescription ScientificKeyPad::keyDescriptions[] = {
    {"", Key_deg, 2, 0, 1, 1}, {"2ⁿᵈ", Key_page, 2, 1, 1, 1}, {"π", Key_PI, 2, 2, 1, 1},
    {"e", Key_e, 2, 3, 1, 1},     {"Mod", Key_Mod, 2, 4, 1, 1},   {"", Key_Backspace, 2, 5, 1, 1},

    {"sin", Key_sin, 3, 0, 1, 1},       {"x²", Key_x2, 3, 1, 1, 1},   {"1/x", Key_Derivative, 3, 2, 1, 1},
    {"x!", Key_Factorials, 3, 3, 1, 1}, {"exp", Key_exp, 3, 4, 1, 1}, {"C", Key_Clear, 3, 5, 1, 1},

    {"cos", Key_cos, 4, 0, 1, 1}, {"x³", Key_x3, 4, 1, 1, 1},     {"(", Key_left, 4, 2, 1, 1},
    {")", Key_right, 4, 3, 1, 1}, {"%", Key_Percent, 4, 4, 1, 1}, {"", Key_Div, 4, 5, 1, 1},

    {"tan", Key_tan, 5, 0, 1, 1},       {"xʸ", Key_xy, 5, 1, 1, 1}, {"7", Key_7, 5, 2, 1, 1},
    {"8", Key_8, 5, 3, 1, 1},         {"9", Key_9, 5, 4, 1, 1}, {"", Key_Mult, 5, 5, 1, 1},

    {"cot", Key_cot, 6, 0, 1, 1}, {"10ˣ", Key_10x, 6, 1, 1, 1}, {"4", Key_4, 6, 2, 1, 1},
    {"5", Key_5, 6, 3, 1, 1},     {"6", Key_6, 6, 4, 1, 1},     {"", Key_Min, 6, 5, 1, 1},

    {"|x|", Key_Modulus, 7, 0, 1, 1}, {"log", Key_log, 7, 1, 1, 1}, {"1", Key_1, 7, 2, 1, 1},
    {"2", Key_2, 7, 3, 1, 1},         {"3", Key_3, 7, 4, 1, 1},     {"", Key_Plus, 7, 5, 1, 1},

    {"Rand", Key_Rand, 8, 0, 1, 1}, {"ln", Key_ln, 8, 1, 1, 1},   {"+/-", Key_opposite, 8, 2, 1, 1},
    {"0", Key_0, 8, 3, 1, 1},       {".", Key_Point, 8, 4, 1, 1}, {"=", Key_Equals, 8, 5, 1, 1},
};

const ScientificKeyPad::KeyDescription1 ScientificKeyPad::keyDescriptions1[] = {

    {"", Key_arcsin, 3, 0, 1, 1},       {"", Key_sqrt2, 3, 1, 1, 1},
    {"", Key_arccos, 4, 0, 1, 1}, {"", Key_sqrt3, 4, 1, 1, 1},

    {"", Key_arctan, 5, 0, 1, 1},       {"", Key_ysqrtx, 5, 1, 1, 1},

    {"", Key_arccot, 6, 0, 1, 1}, {"2ˣ", Key_2x, 6, 1, 1, 1},

    {"logᵧx", Key_logyx, 7, 1, 1, 1},

    {"eˣ", Key_ex, 8, 1, 1, 1},
};

/**
 * @brief 初始化并设置iconbutton
 */
//static DPushButton *createSpecialKeyButton(ScientificKeyPad::Buttons key, bool page)
//{
//    IconButton *button;
//    if (page)
//        button = new IconButton(nullptr, 0, true);
//    else
//        button = new IconButton();

//    QString path;
//    if (DGuiApplicationHelper::instance()->themeType() == 2)
//        path = QString(":/assets/images/%1/").arg("dark");
//    else
//        path = QString(":/assets/images/%1/").arg("light");

//    if (key == ScientificKeyPad::Key_Div) {
//        button->setIconUrl(path + "divide_normal.svg", path + "divide_hover.svg", path + "divide_press.svg");
//    } else if (key == ScientificKeyPad::Key_Mult) {
//        button->setIconUrl(path + "x_normal.svg", path + "x_hover.svg", path + "x_press.svg");
//    } else if (key == ScientificKeyPad::Key_Min) {
//        button->setIconUrl(path + "-_normal.svg", path + "-_hover.svg", path + "-_press.svg");
//    } else if (key == ScientificKeyPad::Key_Plus) {
//        button->setIconUrl(path + "+_normal.svg", path + "+_hover.svg", path + "+_press.svg");
//    } else if (key == ScientificKeyPad::Key_Backspace) {
//        button->setIconUrl(path + "clear_normal.svg", path + "clear_hover.svg", path + "clear_press.svg", 1);
//    }  else if (key == ScientificKeyPad::Key_sqrt2) {
//        button->setIconUrl(path + "squareroot_normal.svg", path + "squareroot_hover.svg", path + "squareroot_press.svg", 3);
//    } else if (key == ScientificKeyPad::Key_sqrt3) {
//        button->setIconUrl(path + "cuberoot_normal.svg", path + "cuberoot_hover.svg", path + "cuberoot_press.svg", 3);
//    } else if (key == ScientificKeyPad::Key_ysqrtx) {
//        button->setIconUrl(path + "yroot_normal.svg", path + "yroot_hover.svg", path + "yroot_press.svg", 3);
//    } else if (key == ScientificKeyPad::Key_deg) {
//        button->setIconUrl(path + "deg_normal.svg", path + "deg_hover.svg", path + "deg_press.svg", 5);
//    }
//    //connect(button, &IconButton::updateInterface, this, &ScientificKeyPad::updateInterface);
//    return button;
//}

ScientificKeyPad::ScientificKeyPad(QWidget *parent)
    : DWidget(parent)
    , m_gridlayout1(new QGridLayout(this))
    , m_mapper(new QSignalMapper(this))
    , m_leftBracket(new DLabel(this))
    , m_rightBracket(new DLabel(this))
    , m_arcsinwidget(new QStackedWidget(this))
    , m_arccoswidget(new QStackedWidget(this))
    , m_arctanwidget(new QStackedWidget(this))
    , m_arccotwidget(new QStackedWidget(this))
    , m_sqrtwidget(new QStackedWidget(this))
    , m_cbrtwidget(new QStackedWidget(this))
    , m_yrootwidget(new QStackedWidget(this))
    , m_2xwidget(new QStackedWidget(this))
    , m_logyxwidget(new QStackedWidget(this))
    , m_exwidget(new QStackedWidget(this))
{
    this->setFixedHeight(KEYPADHEIGHT);
    m_leftBracket->setFixedSize(24, 14);
    m_rightBracket->setFixedSize(24, 14);

    initButtons();
    initUI();
    m_gridlayout1->setMargin(0);
    m_gridlayout1->setSpacing(3); //按钮比ui大2pix,此处小2pix
    m_gridlayout1->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_gridlayout1);

    connect(m_mapper, SIGNAL(mapped(int)), SIGNAL(buttonPressed(int)));
    connect(this, &ScientificKeyPad::buttonPressed, this,
            &ScientificKeyPad::turnPage); //按下2nd事件
    connect(this, &ScientificKeyPad::buttonPressedbySpace, this,
            &ScientificKeyPad::turnPage);
}

ScientificKeyPad::~ScientificKeyPad()
{
}

void ScientificKeyPad::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

/**
 * @brief 根据枚举值返回相应button
 * @param key 按钮名
 */
DPushButton *ScientificKeyPad::button(Buttons key)
{
    if (key == Key_arcsin || key == Key_arccos || key == Key_arccot ||
            key == Key_arctan || key == Key_sqrt2 || key == Key_sqrt3 ||
            key == Key_ysqrtx || key == Key_2x || key == Key_logyx || key == Key_ex) {
        return m_keys1.value(key).first;
    } else {
        return m_keys.value(key).first;
    }
}

DPushButton *ScientificKeyPad::button(int key)
{
    if (key == Key_arcsin || key == Key_arccos || key == Key_arccot ||
            key == Key_arctan || key == Key_sqrt2 || key == Key_sqrt3 ||
            key == Key_ysqrtx || key == Key_2x || key == Key_logyx || key == Key_ex) {
        return m_keys1.value(Buttons(key)).first;
    } else {
        return m_keys.value(Buttons(key)).first;
    }
}

/**
 * @brief 按钮点击动画效果
 */
void ScientificKeyPad::animate(Buttons key, bool isspace)
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

bool ScientificKeyPad::buttonHasFocus()
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
void ScientificKeyPad::initButtons()
{
    const int count = sizeof(keyDescriptions) / sizeof(keyDescriptions[0]);

    for (int i = 0; i < count; ++i) {
        const KeyDescription *desc = keyDescriptions + i;
        DPushButton *button;

        if (desc->text.isEmpty()) {
            if (i % 6 == 0 || i % 6 == 1)
                button = new IconButton(this, 0, true);
            else
                button = new IconButton(this, 0, false);
        } else {
            if (desc->text == "=") {
                button = new EqualButton(desc->text, this);
                connect(static_cast<EqualButton *>(button), &EqualButton::focus, this, &ScientificKeyPad::getFocus); //获取上下左右键
                connect(static_cast<EqualButton *>(button), &EqualButton::space, this, [ = ]() {
                    Buttons spacekey = Key_Equals;
                    emit buttonPressedbySpace(spacekey);
                });
            } else if (desc->text == "MC" || desc->text == "MR" || desc->text == "M+" || desc->text == "M-" || desc->text == "MS") {
                button = new MemoryButton(desc->text, false, this);
            } else {
                if (i % 6 == 0 || i % 6 == 1)
                    button = new TextButton(desc->text, true, this);
                else {
                    button = new TextButton(desc->text, false, this);
                }
            }
        }

        //stackwidget中按钮初始化
        if ((i / 6 > 0) && (i % 6 == 0 || i % 6 == 1) && i != 30 && i != 36) {
            DPushButton *pagebutton;
            //翻页按键初始化
            if (desc->button == Key_sin) {
                const KeyDescription1 *desc1 = keyDescriptions1;
                pagebutton = new TextButton("arcsin", true, this);
                initStackWidget(m_arcsinwidget, button, pagebutton, desc1);
            } else if (desc->button == Key_x2) {
                const KeyDescription1 *desc1 = keyDescriptions1 + 1;
                pagebutton = new IconButton(this, 0, true);
                initStackWidget(m_arccoswidget, button, pagebutton, desc1);
            } else if (desc->button == Key_cos) {
                const KeyDescription1 *desc1 = keyDescriptions1 + 2;
                pagebutton = new TextButton("arccos", true, this);
                initStackWidget(m_arctanwidget, button, pagebutton, desc1);
            } else if (desc->button == Key_x3) {
                const KeyDescription1 *desc1 = keyDescriptions1 + 3;
                pagebutton = new IconButton(this, 0, true);
                initStackWidget(m_arccotwidget, button, pagebutton, desc1);
            } else if (desc->button == Key_tan) {
                const KeyDescription1 *desc1 = keyDescriptions1 + 4;
                pagebutton = new TextButton("arctan", true, this);
                initStackWidget(m_sqrtwidget, button, pagebutton, desc1);
            } else if (desc->button == Key_xy) {
                const KeyDescription1 *desc1 = keyDescriptions1 + 5;
                pagebutton = new IconButton(this, 0, true);
                initStackWidget(m_cbrtwidget, button, pagebutton, desc1);
            } else if (desc->button == Key_cot) {
                const KeyDescription1 *desc1 = keyDescriptions1 + 6;
                pagebutton = new TextButton("arccot", true, this);
                initStackWidget(m_yrootwidget, button, pagebutton, desc1);
            } else if (desc->button == Key_10x) {
                const KeyDescription1 *desc1 = keyDescriptions1 + 7;
                pagebutton = new TextButton("2ˣ", true, this);
                initStackWidget(m_2xwidget, button, pagebutton, desc1);
            } else if (desc->button == Key_log) {
                const KeyDescription1 *desc1 = keyDescriptions1 + 8;
                pagebutton = new TextButton("logᵧx", true, this);
                initStackWidget(m_logyxwidget, button, pagebutton, desc1);
            } else {
                const KeyDescription1 *desc1 = keyDescriptions1 + 9;
                pagebutton = new TextButton("eˣ", true, this);
                initStackWidget(m_exwidget, button, pagebutton, desc1);
            }

            pagebutton->setFixedSize(BUTTON_SIZE);
            connect(static_cast<TextButton *>(pagebutton), &TextButton::focus, this, &ScientificKeyPad::getFocus); //获取上下左右键
        } else {
            m_gridlayout1->addWidget(button, desc->row, desc->column, desc->rowcount, desc->columncount,
                                     Qt::AlignCenter/* | Qt::AlignTop*/);
        }

        button->setFixedSize(BUTTON_SIZE);
        const QPair<DPushButton *, const KeyDescription *> hashValue(button, desc);
        m_keys.insert(desc->button, hashValue); //key为枚举值，value.first为DPushButton *, value.second为const KeyDescription *

        if (desc->text != "=") {
            connect(static_cast<TextButton *>(button), &TextButton::focus, this, &ScientificKeyPad::getFocus); //获取上下左右键
            connect(static_cast<TextButton *>(button), &TextButton::space, this, [ = ]() {
                Buttons spacekey = m_keys.key(hashValue);
                emit buttonPressedbySpace(spacekey);
            });
        }
        connect(button, &DPushButton::clicked, m_mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        m_mapper->setMapping(button, desc->button); //多个按钮绑定到一个mapper上

    }
}

/**
 * @brief 连接窗口缩放与按钮大小变化的信号槽，设置左右括号不匹配数label的状态颜色
 */
void ScientificKeyPad::initUI()
{
    QHashIterator<Buttons, QPair<DPushButton *, const KeyDescription *>> i(m_keys);

    while (i.hasNext()) {
        i.next();
        if (i.key() == Key_left) {
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
        if (i.key() == Key_right) {
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

    QHashIterator<Buttons, QPair<DPushButton *, const KeyDescription1 *>> i1(m_keys1);

    button(Key_Div)->setObjectName("SymbolButton");
    button(Key_Mult)->setObjectName("SymbolButton");
    button(Key_Min)->setObjectName("SymbolButton");
    button(Key_Plus)->setObjectName("SymbolButton");

    this->setContentsMargins(12, 0, 12, 12);
}

/**
 * @brief ScientificKeyPad::initStackWidget
 * @param widget
 * @param button index(0)的按键
 * @param pagebutton index(1)的按键
 * @param desc1 index(1)按键信息
 * page2按键初始化设置
 */
void ScientificKeyPad::initStackWidget(QStackedWidget *widget, DPushButton *button, DPushButton *pagebutton, const KeyDescription1 *desc1)
{
    widget->addWidget(button);
    widget->addWidget(pagebutton);
    widget->setCurrentIndex(0);
    widget->setFixedSize(BUTTON_SIZE.width(), BUTTON_SIZE.height() + 4); //预留４pix给阴影
    m_gridlayout1->addWidget(widget, desc1->row, desc1->column, desc1->rowcount, desc1->columncount,
                             Qt::AlignCenter/* | Qt::AlignTop*/);
    const QPair<DPushButton *, const KeyDescription1 *> hashValue1(pagebutton, desc1);
    m_keys1.insert(desc1->button, hashValue1); //key为枚举值，value.first为DPushButton *, value.second为const KeyDescription1 *
    connect(pagebutton, &DPushButton::clicked, m_mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(static_cast<TextButton *>(pagebutton), &TextButton::space, this, [ = ]() {
        Buttons spacekey = m_keys1.key(hashValue1);
        emit buttonPressedbySpace(spacekey);
    });
    m_mapper->setMapping(pagebutton, desc1->button); //多个按钮绑定到一个mapper上
}

/**
 * @brief 切换相应主题切图
 */
void ScientificKeyPad::buttonThemeChanged(int type)
{
    m_themetype = type;
    QString path;
    if (type == 2) {
        path = QString(":/assets/images/%1/").arg("dark");
        m_leftBracket->setStyleSheet(QString("font-family:Noto Sans;color:white;font-size:14px;"));
        m_rightBracket->setStyleSheet(QString("font-family:Noto Sans;color:white;font-size:14px;"));
    } else {
        path = QString(":/assets/images/%1/").arg("light");
        m_leftBracket->setStyleSheet(QString("font-family:Noto Sans;color:black;font-size:14px;"));
        m_rightBracket->setStyleSheet(QString("font-family:Noto Sans;color:black;font-size:14px;"));
    }

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
    btn = static_cast<IconButton *>(button(Key_sqrt2));
    btn->setIconUrl(path + "squareroot_normal.svg", path + "squareroot_hover.svg", path + "squareroot_press.svg", 3);
    btn = static_cast<IconButton *>(button(Key_sqrt3));
    btn->setIconUrl(path + "cuberoot_normal.svg", path + "cuberoot_hover.svg", path + "cuberoot_press.svg", 3);
    btn = static_cast<IconButton *>(button(Key_ysqrtx));
    btn->setIconUrl(path + "yroot_normal.svg", path + "yroot_hover.svg", path + "yroot_press.svg", 3);
    if (m_deg == 1) {
        btn = static_cast<IconButton *>(button(Key_deg));
        btn->setIconUrl(path + "rad_normal.svg", path + "rad_hover.svg", path + "rad_press.svg", 5);
    } else if (m_deg == 2) {
        btn = static_cast<IconButton *>(button(Key_deg));
        btn->setIconUrl(path + "grad_normal.svg", path + "grad_hover.svg", path + "grad_press.svg", 7);
    } else if (m_deg == 3) {
        btn = static_cast<IconButton *>(button(Key_deg));
        btn->setIconUrl(path + "deg_normal.svg", path + "deg_hover.svg", path + "deg_press.svg", 5);
    }
}

/**
 * @brief 点击2nd时切换stackwidget
 */
void ScientificKeyPad::turnPage(int key)
{
    if (key == Key_page) {
        if (m_arcsinwidget->currentIndex() == 0) {
            m_arcsinwidget->setCurrentIndex(1);
            m_arccoswidget->setCurrentIndex(1);
            m_arctanwidget->setCurrentIndex(1);
            m_arccotwidget->setCurrentIndex(1);
            m_sqrtwidget->setCurrentIndex(1);
            m_cbrtwidget->setCurrentIndex(1);
            m_yrootwidget->setCurrentIndex(1);
            m_2xwidget->setCurrentIndex(1);
            m_logyxwidget->setCurrentIndex(1);
            m_exwidget->setCurrentIndex(1);
        } else {
            m_arcsinwidget->setCurrentIndex(0);
            m_arccoswidget->setCurrentIndex(0);
            m_arctanwidget->setCurrentIndex(0);
            m_arccotwidget->setCurrentIndex(0);
            m_sqrtwidget->setCurrentIndex(0);
            m_cbrtwidget->setCurrentIndex(0);
            m_yrootwidget->setCurrentIndex(0);
            m_2xwidget->setCurrentIndex(0);
            m_logyxwidget->setCurrentIndex(0);
            m_exwidget->setCurrentIndex(0);
        }
    }
}

/**
 * @brief ScientificKeyPad::getdeg
 * @param deg 当前为deg/rad/grad
 * 切换deg按键图标
 */
void ScientificKeyPad::getdeg(int deg)
{
    m_deg = deg;
    QString path;
    if (m_themetype == 2)
        path = QString(":/assets/images/%1/").arg("dark");
    else
        path = QString(":/assets/images/%1/").arg("light");
    IconButton *btn;
    //m_deg == 1，点击后为rad
    if (m_deg == 1) {
        btn = static_cast<IconButton *>(button(Key_deg));
        QTimer::singleShot(100, this, [ = ] { //延迟出focus按下动画效果
            btn->setIconUrl(path + "rad_normal.svg", path + "rad_hover.svg", path + "rad_press.svg", 5);
            btn->update();
        });
    } else if (m_deg == 2) {
        btn = static_cast<IconButton *>(button(Key_deg));
        QTimer::singleShot(100, this, [ = ] {
            btn->setIconUrl(path + "grad_normal.svg", path + "grad_hover.svg", path + "grad_press.svg", 7);
            btn->update();
        });
    } else if (m_deg == 3) {
        btn = static_cast<IconButton *>(button(Key_deg));
        QTimer::singleShot(100, this, [ = ] {
            btn->setIconUrl(path + "deg_normal.svg", path + "deg_hover.svg", path + "deg_press.svg", 5);
            btn->update();
        });
    }
}

/**
 * @brief ScientificKeyPad::bracketsNum
 * @param direction 0-左括号， 1-右括号
 * @param num 不匹配括号数
 * 显示左右括号不匹配数
 */
void ScientificKeyPad::bracketsNum(int direction, QString num)
{
    if (num == "0")
        num = "";

    if (direction == 0)
        m_leftBracket->setText(num);
    else if (direction == 1)
        m_rightBracket->setText(num);
}

/**
 * @brief 获取button上下左右键信号setfocus
 * @param direction 0-上　1-下　2-左　3-右
 */
void ScientificKeyPad::getFocus(int direction)
{
    QHashIterator<Buttons, QPair<DPushButton *, const KeyDescription *>> i(m_keys);
    bool ihasfocus = false; //i是否有focus按钮
    int number = -1; //记录focus的Buttons
    while (i.hasNext()) {
        i.next();
        if (i.value().first->hasFocus()) {
            ihasfocus = true;
            number = i.key();
            break; //获取焦点按钮
        }
    }
    if (ihasfocus == false) {
        QHashIterator<Buttons, QPair<DPushButton *, const KeyDescription1 *>> i1(m_keys1);
        while (i1.hasNext()) {
            i1.next();
            if (i1.value().first->hasFocus()) {
                ihasfocus = true;
                number = i1.key();
                break; //获取焦点按钮
            }
        }
    }
    if (number == -1)
        return;
    if (m_arcsinwidget->currentIndex() == 0) {
        switch (direction) {
        case 0:
            if (number / 6 > 0)
                button(static_cast<Buttons>(number - 6))->setFocus(); //根据上下左右信号重置焦点
            break;
        case 1:
            if (number / 6 < 6)
                button(static_cast<Buttons>(number + 6))->setFocus();
            break;
        case 2:
            if (number % 6 > 0)
                button(static_cast<Buttons>(number - 1))->setFocus();
            break;
        case 3:
            if (number % 6 < 5)
                button(static_cast<Buttons>(number + 1))->setFocus();
            break;
        default:
            break;
        }
    } else {
        switch (direction) {
        case 0:
            if (number == Key_arcsin) //focus在arcsin
                button(Key_deg)->setFocus();
            else if (number == Key_sqrt2)
                button(Key_page)->setFocus();
            else if (number == Key_Modulus)
                button(Key_arccot)->setFocus();
            else if (number >= 42 && number <= 51) //focus在剩余第二页界面
                button(static_cast<Buttons>(number - 1))->setFocus();
            else if (number / 6 > 0)
                button(static_cast<Buttons>(number - 6))->setFocus(); //根据上下左右信号重置焦点
            break;
        case 1:
            if (number == Key_deg) //focus在deg
                button(Key_arcsin)->setFocus();
            else if (number == Key_page)
                button(Key_sqrt2)->setFocus();
            else if (number == Key_arccot)
                button(Key_Modulus)->setFocus();
            else if (number >= 42 && number < 51) //focus在剩余第二页界面
                button(static_cast<Buttons>(number + 1))->setFocus();
            else if (number / 6 < 6)
                button(static_cast<Buttons>(number + 6))->setFocus();
            break;
        case 2:
            if (number == Key_logyx) //focus在logyx
                button(Key_Modulus)->setFocus();
            else if (number == Key_ex)
                button(Key_Rand)->setFocus();
            else if (number >= 46 && number <= 51) //二行以下第二列
                button(static_cast<Buttons>(number - 4))->setFocus();
            else if (number < 42 && number / 6 > 0 && number % 6 == 2) //二行以下第三列
                button(static_cast<Buttons>(number + 13 + 5 * (6 - number / 6)))->setFocus();
            else if (number < 42 && number % 6 > 0)
                button(static_cast<Buttons>(number - 1))->setFocus();
            break;
        case 3:
            if (number == Key_Modulus) //focus在logyx
                button(Key_logyx)->setFocus();
            else if (number == Key_Rand)
                button(Key_ex)->setFocus();
            else if (number >= 42 && number <= 45) //二行以下第一列
                button(static_cast<Buttons>(number + 4))->setFocus();
            else if (number >= 46 && number <= 51) //二行以下第二列
                button(static_cast<Buttons>(number - 13 - 5 * (51 - number)))->setFocus();
            else if (number < 42 && number % 6 < 5)
                button(static_cast<Buttons>(number + 1))->setFocus();
            break;
        default:
            break;
        }
    }
}
