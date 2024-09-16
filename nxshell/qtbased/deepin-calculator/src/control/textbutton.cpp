// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textbutton.h"

const qreal BLURRADIUS = 12; //阴影模糊半径
const qreal ROUND_XRADIUS = 8; //按钮圆角x轴半径
const qreal ROUND_YRADIUS = 8; //按钮圆角y轴半径

TextButton::TextButton(const QString &text, bool page, QWidget *parent)
    : DPushButton(text, parent)
    , m_effect(new QGraphicsDropShadowEffect(this))
{
    setFocusPolicy(Qt::TabFocus);
    setObjectName("TextButton");

    init();
    m_isHover = m_isPress = false;
    m_page = page;

    m_effect->setOffset(0, 4); //阴影偏移
    m_effect->setBlurRadius(BLURRADIUS); //阴影模糊半径
}

TextButton::~TextButton()
{
    // delete m_effect;
}

void TextButton::init()
{
    //按钮初始化时确认当前是否是上标按钮
    if (text() == "2ⁿᵈ") {
        m_btn = 1;
    } else if (text() == "xʸ") {
        m_btn = 2;
    } else if (text() == "10ˣ") {
        m_btn = 3;
    } else if (text() == "2ˣ") {
        m_btn = 4;
    } else if (text() == "eˣ") {
        m_btn = 5;
    } else if (text() == "arcsin") {
        m_btn = 6;
    } else if (text() == "arccos") {
        m_btn = 7;
    } else if (text() == "arctan") {
        m_btn = 8;
    } else if (text() == "arccot") {
        m_btn = 9;
    }
    if (text() == "Rand")
        m_font.setPixelSize(15);
    else if (text() == "logᵧx") {
        m_font.setPixelSize(14);
    } else if (text() == "log") {
        m_font.setPixelSize(16);
    } else if (text() == "|x|") {
        m_font.setPixelSize(14);
    } else if (text() == "Mod") {
        m_font.setPixelSize(14);
    } else if (text() == "exp") {
        m_font.setPixelSize(16);
    } else if (text() == "sin") {
        m_font.setPixelSize(16);
    } else if (text() == "cos") {
        m_font.setPixelSize(16);
    } else if (text() == "tan") {
        m_font.setPixelSize(16);
    } else if (text() == "cot") {
        m_font.setPixelSize(16);
    } else if (text() == "+/-") {
        m_font.setPixelSize(20);
    } else if (text() == "QWORD" || text() == "DWORD" || text() == "WORD" || text() == "BYTE"
               || text() == "AND" || text() == "OR" || text() == "NOT" || text() == "NAND" || text() == "NOR" || text() == "XOR") {
        m_font.setPixelSize(14);
    } else if (text() == "A" || text() == "B" || text() == "C" || text() == "D"
               || text() == "E" || text() == "F") {
        m_font.setPixelSize(17);
    } else
        m_font.setPixelSize(18);
    m_font.setFamily("Noto Sans");
    m_font.setStyleName("Light");
    m_font.setWeight(2);
}

/**
 * @brief iconbutton物理键盘点击动画效果
 * @param msec 100ms
 */
void TextButton::animate(bool isspace, int msec)
{
    if (m_isPress == false) { //edit for bug-20492 20200416
        m_isHover = false;  //edit for bug-20508 20200414
        if (!isspace)
            setDown(true);
        m_isPress = true;

        QTimer::singleShot(msec, this, [ = ] {
            if (!isspace)
                setDown(false);
            m_isPress = false;
            update();
        });
    }
}

/**
 * @brief 判断FE与2nd按键是否被按下
 */
void TextButton::setButtonDown(bool down)
{
    m_Btnisdown = down;
    update();
}

/**
 * @brief TextButton::setBtnPressing
 * 用于数据长度和移位类型的按键，点开列表后，保持press的状态
 */
void TextButton::setBtnPressing(bool press)
{
    if (press) {
        m_palette = this->palette();
        m_font.setPixelSize(14);
        m_isPress = true;
        m_isHover = false; //20200722删除foucus状态
    } else {
        m_isPress = false;
    }
}

void TextButton::setButtonGray(bool gray)
{
//    m_isgray = gray;
    this->setEnabled(!gray);
    update();
}

/**
 * @brief 鼠标按下
 */
void TextButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
        return;
//    setFocus();
    m_palette = this->palette();
    m_isPress = true;
    m_isHover = false; //20200722删除foucus状态
    DPushButton::mousePressEvent(e);
}

/**
 * @brief 鼠标松开
 */
void TextButton::mouseReleaseEvent(QMouseEvent *e)
{
//    clearFocus();
    if (e->button() == Qt::RightButton)
        return;
    this->setPalette(m_palette);
    m_isPress = false;
    if (this->rect().contains(e->pos())) {
        m_isacting = true;
        m_isHover = true;
    } else
        m_isacting = false;
    emit mouseRelease();
    DPushButton::mouseReleaseEvent(e);
}

void TextButton::enterEvent(QEvent *e)
{
    if (text() == "Rand")
        m_font.setPixelSize(17);
    else if (text() == "logᵧx") {
        m_font.setPixelSize(16);
    } else if (text() == "log") {
        m_font.setPixelSize(18);
    } else if (text() == "|x|") {
        m_font.setPixelSize(16);
    } else if (text() == "Mod") {
        m_font.setPixelSize(16);
    } else if (text() == "exp") {
        m_font.setPixelSize(18);
    } else if (text() == "sin") {
        m_font.setPixelSize(18);
    } else if (text() == "cos") {
        m_font.setPixelSize(18);
    } else if (text() == "tan") {
        m_font.setPixelSize(18);
    } else if (text() == "cot") {
        m_font.setPixelSize(18);
    } else if (text() == "+/-") {
        m_font.setPixelSize(22);
    } else if (text() == "QWORD" || text() == "DWORD" || text() == "WORD" || text() == "BYTE"
               || text() == "AND" || text() == "OR" || text() == "NOT" || text() == "NAND" || text() == "NOR" || text() == "XOR") {
        m_font.setPixelSize(16);
    } else if (text() == "A" || text() == "B" || text() == "C" || text() == "D"
               || text() == "E" || text() == "F") {
        m_font.setPixelSize(19);
    } else if (text() == "arcsin" || text() == "arccos" || text() == "arctan" || text() == "arccot") {
        m_font.setPixelSize(18);
    } else
        m_font.setPixelSize(20);
    m_font.setStyleName("");
    m_isHover = true;
    DPushButton::enterEvent(e);
}

void TextButton::leaveEvent(QEvent *e)
{
    if (text() == "Rand")
        m_font.setPixelSize(15);
    else if (text() == "logᵧx") {
        m_font.setPixelSize(14);
    } else if (text() == "log") {
        m_font.setPixelSize(16);
    } else if (text() == "|x|") {
        m_font.setPixelSize(14);
    } else if (text() == "Mod") {
        m_font.setPixelSize(14);
    } else if (text() == "exp") {
        m_font.setPixelSize(16);
    } else if (text() == "sin") {
        m_font.setPixelSize(16);
    } else if (text() == "cos") {
        m_font.setPixelSize(16);
    } else if (text() == "tan") {
        m_font.setPixelSize(16);
    } else if (text() == "cot") {
        m_font.setPixelSize(16);
    } else if (text() == "+/-") {
        m_font.setPixelSize(20);
    } else if (text() == "QWORD" || text() == "DWORD" || text() == "WORD" || text() == "BYTE"
               || text() == "AND" || text() == "OR" || text() == "NOT" || text() == "NAND" || text() == "NOR" || text() == "XOR") {
        m_font.setPixelSize(14);
    } else if (text() == "A" || text() == "B" || text() == "C" || text() == "D"
               || text() == "E" || text() == "F") {
        m_font.setPixelSize(17);
    } else if (text() == "arcsin" || text() == "arccos" || text() == "arctan" || text() == "arccot") {
        m_font.setPixelSize(16);
    } else
        m_font.setPixelSize(18);
    m_font.setStyleName("Light");
    m_isHover = false;
    m_isacting = false;
    DPushButton::leaveEvent(e);
}

void TextButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if (isEnabled() == false) {
        m_font.setPixelSize(18);
        m_font.setStyleName("Light");
        m_isHover = false;
        m_isgray = true;
    } else {
        m_isgray = false;
    }
    QRectF rect = this->rect();
    QRectF normal(rect.left() + 1, rect.top() + 1, rect.width() - 2, rect.height() - 2);
    QRectF hover(rect.left() + 1, rect.top() + 1, rect.width() - 2, rect.height() - 2);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); //反锯齿
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true); //光滑的象素映射变换算法
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setFont(m_font);
    QRectF textRect = painter.fontMetrics().boundingRect(0, 0, int(rect.width()), int(rect.height()),
                                                         Qt::AlignCenter, this->text());
    QColor actcolor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name();//活动色
    QColor pressBrush, focus, hoverFrame, base, text, hoverbrush;
    QColor hoverShadow, focusShadow;
    QColor pressText = actcolor;
    hoverShadow = actcolor;
    hoverShadow.setAlphaF(0.1);
    focusShadow = QColor(0, 0, 0);
    focusShadow.setAlphaF(0.05);
    int type = DGuiApplicationHelper::instance()->themeType();
    if (type == 1) { //浅色主题设置
        pressBrush = QColor(0, 0, 0);
        pressBrush.setAlphaF(0.1);
        focus = actcolor;
        hoverFrame = actcolor;
        hoverFrame.setAlphaF(0.2);
        if (m_page) {
            base = QColor(238, 238, 238);
            hoverbrush = QColor(238, 238, 238);
        } else {
            base = QColor("#FFFFFF");
            hoverbrush = QColor("#FFFFFF");
        }
        text = QColor("#303030");
        if (m_isgray == true) {
            base = QColor("#FFFFFF");
            hoverbrush = QColor("#FFFFFF");
            text = QColor("#303030");
            text.setAlphaF(0.4);
            pressBrush = QColor("#FFFFFF");
        }
    } else { //深色主题设置
        pressBrush = QColor(0, 0, 0);
        pressBrush.setAlphaF(0.5);
        focus = actcolor;
        hoverFrame = actcolor;
        hoverFrame.setAlphaF(0.2);
        if (m_page) {
            base = QColor(60, 60, 60);
            hoverbrush = QColor(60, 60, 60);
        } else {
            base = QColor("#303030");
            hoverbrush = QColor(60, 60, 60);
        }
        if (m_isHover)
            text = Qt::white;
        else
            text = QColor(224, 224, 224);
        if (m_isgray == true) {
            base = QColor(48, 48, 48);
            base.setAlphaF(0.4);
            text = QColor(224, 224, 224);
            text.setAlphaF(0.4);
            pressText = Qt::black;
            pressBrush = QColor("#FFFFFF");
            hoverbrush = QColor(48, 48, 48);
            hoverbrush.setAlphaF(0.4);
        }
    }
    if (m_Btnisdown)
        text = actcolor; //FE,2nd设置
    if (hasFocus()) {
        painter.setPen(Qt::NoPen);
        if (m_isPress) {
            painter.setBrush(QBrush(pressBrush));
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            QPen pen;
            pen.setColor(pressText);
            painter.setPen(pen);
            pen.setColor(focus);
            pen.setWidth(2);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //focus边框
            paintspecialbtn(painter, rect, textRect);
        } else {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(base));
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            QPen pen;
//            if (m_isacting) {
//                painter.setPen(Qt::NoPen);
//            } else {
            pen.setColor(focus);
            pen.setWidth(2);
            painter.setPen(pen);
//            }
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素

            pen.setColor(text);
            painter.setPen(pen);
            painter.setFont(m_font);
            paintspecialbtn(painter, rect, textRect);
            m_effect->setColor(focusShadow);
            this->setGraphicsEffect(m_effect);
        }
    } else {
        painter.setPen(Qt::NoPen);
        if (m_isHover) { //hover状态
            painter.setPen(QPen(hoverFrame));
            painter.setBrush(QBrush(hoverbrush));
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素

            QPen pen;
            pen.setColor(text);
            painter.setPen(pen);
            paintspecialbtn(painter, rect, textRect);
            m_effect->setColor(hoverShadow);
            this->setGraphicsEffect(m_effect);
        } else if (m_isPress) { //press状态
            painter.setBrush(QBrush(pressBrush));
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            QPen pen;
            pen.setColor(pressText);
            painter.setPen(pen);
            paintspecialbtn(painter, rect, textRect);
        } else { //normal状态
            painter.setBrush(QBrush(base));
            painter.drawRoundedRect(normal, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            QPen pen;
            pen.setColor(text);
            painter.setPen(pen);
            paintspecialbtn(painter, rect, textRect);
            m_effect->setColor(QColor(0, 0, 0, 0));
            this->setGraphicsEffect(m_effect);
        }
    }
}

void TextButton::focusOutEvent(QFocusEvent *e)
{
    QPushButton::focusOutEvent(e);
}

void TextButton::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Up:
        emit focus(0);
        break;
    case Qt::Key_Down:
        emit focus(1);
        break;
    case Qt::Key_Left:
        emit focus(2);
        break;
    case Qt::Key_Right:
        emit focus(3);
        break;
    case Qt::Key_Space:
        emit space();
        break;
    default:
        DPushButton::keyPressEvent(e);
        break;
    }
}

/**
 * @brief 绘制有上标的按钮
 * @param painter
 * @param rect 按钮rect
 * @param textRect 字体rect
 */
void TextButton::paintspecialbtn(QPainter &painter, QRectF rect, QRectF textRect)
{
    painter.save();
    int powersize = 10; //次方字体大小
    int trisize = 16; //三角函数字体大小
    int vpadding = 0; //纵向微调距
    if (!hasFocus() && (m_isHover || m_isPress)) {
        powersize = 12;
        trisize = 18;
    }

    if (m_btn == 1) {
        vpadding = -1;
        QRectF textRect1 = painter.fontMetrics().boundingRect(0, 0, int(rect.width()), int(rect.height()),
                                                              Qt::AlignCenter, "2ⁿ");
        painter.setFont(m_font);
        painter.drawText(textRect1, "2");
        QFont a;
        a.setPixelSize(powersize);
        if (!hasFocus() && (m_isHover || m_isPress)) {
            a.setPixelSize(11);
            vpadding = 0;
        }
        QRectF textRect2 = painter.fontMetrics().boundingRect(int(rect.width()) / 2, int(rect.height() / 4 - vpadding),
                                                              int(rect.width()) / 2, int(rect.height() / 2),
                                                              Qt::AlignLeft, "nd");
        a.setFamily("Noto Sans");
        painter.setFont(a);
        painter.drawText(textRect2, "nd");
    } else if (m_btn == 2) {
        vpadding = 2;
        if (!hasFocus() && (m_isHover || m_isPress)) {
            vpadding = 4;
        }

        QRectF textRect1 = painter.fontMetrics().boundingRect(0, 0, int(rect.width()), int(rect.height()),
                                                              Qt::AlignCenter, "xʸ");
        painter.setFont(m_font);
        painter.drawText(textRect1, "x");
        QRectF textRect2 = painter.fontMetrics().boundingRect(int(rect.width()) / 2 + 2, int(rect.height() / 4 - vpadding),
                                                              int(rect.width()) / 2, int(rect.height() / 2),
                                                              Qt::AlignLeft, "y");
        QFont a;
        a.setPixelSize(powersize);
        a.setFamily("Noto Sans");
        painter.setFont(a);
        painter.drawText(textRect2, "y");
    } else if (m_btn == 3) {
        vpadding = 2;
        if (!hasFocus() && (m_isHover || m_isPress)) {
            vpadding = 4;
        }
        QRectF textRect1 = painter.fontMetrics().boundingRect(0, 0, int(rect.width()), int(rect.height()),
                                                              Qt::AlignCenter, "10ˣ");
        painter.setFont(m_font);
        painter.drawText(textRect1, "10");
        QRectF textRect2 = painter.fontMetrics().boundingRect(int(rect.width()) / 2 + 8, int(rect.height() / 4 - vpadding),
                                                              int(rect.width()) / 2, int(rect.height() / 2),
                                                              Qt::AlignLeft, "x");
        QFont a;
        a.setPixelSize(powersize);
        a.setFamily("Noto Sans");
        painter.setFont(a);
        painter.drawText(textRect2, "x");
    } else if (m_btn == 4) {
        vpadding = 2;
        if (!hasFocus() && (m_isHover || m_isPress)) {
            vpadding = 4;
        }
        QRectF textRect1 = painter.fontMetrics().boundingRect(0, 0, int(rect.width()), int(rect.height()),
                                                              Qt::AlignCenter, "2ˣ");
        painter.setFont(m_font);
        painter.drawText(textRect1, "2");
        QRectF textRect2 = painter.fontMetrics().boundingRect(int(rect.width()) / 2 + 2, int(rect.height() / 4 - vpadding),
                                                              int(rect.width()) / 2, int(rect.height() / 2),
                                                              Qt::AlignLeft, "x");
        QFont a;
        a.setPixelSize(powersize);
        a.setFamily("Noto Sans");
        painter.setFont(a);
        painter.drawText(textRect2, "x");
    } else if (m_btn == 5) {
        vpadding = 2;
        if (!hasFocus() && (m_isHover || m_isPress)) {
            vpadding = 4;
        }
        QRectF textRect1 = painter.fontMetrics().boundingRect(0, 0, int(rect.width()), int(rect.height()),
                                                              Qt::AlignCenter, "eˣ");
        painter.setFont(m_font);
        painter.drawText(textRect1, "e");
        QRectF textRect2 = painter.fontMetrics().boundingRect(int(rect.width()) / 2 + 2, int(rect.height() / 4 - vpadding),
                                                              int(rect.width()) / 2, int(rect.height() / 2),
                                                              Qt::AlignLeft, "x");
        QFont a;
        a.setPixelSize(powersize);
        a.setFamily("Noto Sans");
        painter.setFont(a);
        painter.drawText(textRect2, "x");
    } else if (m_btn == 6) {
        vpadding = 2;
        if (!hasFocus() && (m_isHover || m_isPress)) {
            vpadding = 4;
        }
        QRectF textRect1 = painter.fontMetrics().boundingRect(0, 0, int(rect.width()), int(rect.height()),
                                                              Qt::AlignCenter, "sinˣ");
        m_font.setPixelSize(trisize);
        painter.setFont(m_font);
        painter.drawText(textRect1, "sin");
        QRectF textRect2 = painter.fontMetrics().boundingRect(int(rect.width()) / 2 + 8, int(rect.height() / 4 - vpadding),
                                                              int(rect.width()) / 2, int(rect.height() / 2),
                                                              Qt::AlignLeft, "-1");
        QFont a;
        a.setPixelSize(powersize);
        a.setFamily("Noto Sans");
        painter.setFont(a);
        painter.drawText(textRect2, "-1");
    } else if (m_btn == 7) {
        vpadding = 2;
        if (!hasFocus() && (m_isHover || m_isPress)) {
            vpadding = 4;
        }
        QRectF textRect1 = painter.fontMetrics().boundingRect(0, 0, int(rect.width()), int(rect.height()),
                                                              Qt::AlignCenter, "cosˣ");
        m_font.setPixelSize(trisize);
        painter.setFont(m_font);
        painter.drawText(textRect1, "cos");
        QRectF textRect2 = painter.fontMetrics().boundingRect(int(rect.width()) / 2 + 8, int(rect.height() / 4 - vpadding),
                                                              int(rect.width()) / 2, int(rect.height() / 2),
                                                              Qt::AlignLeft, "-1");
        QFont a;
        a.setPixelSize(powersize);
        a.setFamily("Noto Sans");
        painter.setFont(a);
        painter.drawText(textRect2, "-1");
    } else if (m_btn == 8) {
        vpadding = 2;
        if (!hasFocus() && (m_isHover || m_isPress)) {
            vpadding = 4;
        }
        QRectF textRect1 = painter.fontMetrics().boundingRect(0, 0, int(rect.width()), int(rect.height()),
                                                              Qt::AlignCenter, "tanˣ");
        m_font.setPixelSize(trisize);
        painter.setFont(m_font);
        painter.drawText(textRect1, "tan");
        QRectF textRect2 = painter.fontMetrics().boundingRect(int(rect.width()) / 2 + 8, int(rect.height() / 4 - vpadding),
                                                              int(rect.width()) / 2, int(rect.height() / 2),
                                                              Qt::AlignLeft, "-1");
        QFont a;
        a.setPixelSize(powersize);
        a.setFamily("Noto Sans");
        painter.setFont(a);
        painter.drawText(textRect2, "-1");
    } else if (m_btn == 9) {
        vpadding = 2;
        if (!hasFocus() && (m_isHover || m_isPress)) {
            vpadding = 4;
        }
        QRectF textRect1 = painter.fontMetrics().boundingRect(0, 0, int(rect.width()), int(rect.height()),
                                                              Qt::AlignCenter, "cotˣ");
        m_font.setPixelSize(trisize);
        painter.setFont(m_font);
        painter.drawText(textRect1, "cot");
        QRectF textRect2 = painter.fontMetrics().boundingRect(int(rect.width()) / 2 + 8, int(rect.height() / 4 - vpadding),
                                                              int(rect.width()) / 2, int(rect.height() / 2),
                                                              Qt::AlignLeft, "-1");
        QFont a;
        a.setPixelSize(powersize);
        a.setFamily("Noto Sans");
        painter.setFont(a);
        painter.drawText(textRect2, "-1");
    } else {
        painter.setFont(m_font);
        painter.drawText(textRect, this->text());
    }

    painter.restore();
}
