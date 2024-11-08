/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*             2020 KylinSoft Co., Ltd.
* This file is part of Lingmo-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "capturebutton.h"
#include "src/widgets/capture/capturewidget.h"
#include "src/utils/confighandler.h"
#include "src/tools/capturetool.h"
#include "src/tools/toolfactory.h"
#include "src/utils/globalvalues.h"
#include "src/utils/colorutils.h"
#include <QFont>
#include <QIcon>
#include <QPropertyAnimation>
#include <QToolTip>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
// Button represents a single button of the capture widget, it can enable
// multiple functionality.

CaptureButton::CaptureButton(const ButtonType t, QWidget *parent) : QPushButton(parent),
    m_buttonType(t)
{
    initButton();
    setCursor(Qt::ArrowCursor);
    updateIcon();
#ifndef SUPPORT_NEWUI
    if (t == TYPE_OPTION) {
        QLabel *label = new  QLabel(this);
        QLabel *label2 = new  QLabel(this);
        label->setAlignment(Qt::AlignRight);
        label->setGeometry(0, 10, 47, 24);
        label2->setAlignment(Qt::AlignCenter|Qt::AlignHCenter);
        label->setFont(font);
        label->setText(tr("option"));
        label2->setGeometry(47, 13, 20, 20);
        if ((m_context.style_name.compare("lingmo-white") == 0)
            || (m_context.style_name.compare("lingmo-default") == 0)
            || (m_context.style_name.compare("lingmo-light") == 0)) {
            label2->setPixmap(QPixmap(QStringLiteral(":/img/material/white/down.svg")));
        } else if ((m_context.style_name.compare("lingmo-dark") == 0)
                   || (m_context.style_name.compare("lingmo-black") == 0)) {
            label2->setPixmap(QPixmap(QStringLiteral(":/img/material/dark-theme/down.png")));
        }
        this->setFixedSize(GlobalValues::buttonBaseSize()*2, GlobalValues::buttonBaseSize());
        QFont f = this->font();
        f.setBold(true);
        setFlat(false);
    } else {
#endif
    if (t == TYPE_SAVE) {
        setFlat(false);
        setText(tr("Save"));
        this->setFixedSize(65, 30);
        this->setProperty("isImportant", true);
    } else if (t == TYPE_PIN) {
        this->setFixedSize(31, 31);
        setStyleSheet(globalStyleSheet());
    } else {
        setStyleSheet(globalStyleSheet());
    }
#ifndef SUPPORT_NEWUI
}

#endif
}

void CaptureButton::initButton()
{
    m_context.style_settings = new QGSettings("org.lingmo.style");
    m_context.style_name = m_context.style_settings->get("style-name").toString();
    m_tool = ToolFactory().CreateTool(m_buttonType, this);
    setFocusPolicy(Qt::NoFocus);
#ifndef SUPPORT_NEWUI
    if (m_buttonType == CaptureButton::TYPE_OPTION) {
        resize(GlobalValues::buttonBaseSize()*2+12, GlobalValues::buttonBaseSize());
        setMask(QRegion(QRect(-1, -1, GlobalValues::buttonBaseSize()*2+16,
                              GlobalValues::buttonBaseSize()*2+4),
                        QRegion::Rectangle));
    } else {
#endif
    resize(GlobalValues::buttonBaseSize(), GlobalValues::buttonBaseSize());
    setMask(QRegion(QRect(-1, -1, GlobalValues::buttonBaseSize()+2,
                          GlobalValues::buttonBaseSize()+2),
                    QRegion::Rectangle));

    if (m_buttonType == CaptureButton::TYPE_SAVE) {
        resize(GlobalValues::buttonBaseSize()*2+12, GlobalValues::buttonBaseSize());
        setMask(QRegion(QRect(-1, -1, 66, 40),
                        QRegion::Rectangle));
    } else if (m_buttonType == CaptureButton::TYPE_PIN) {
        resize(GlobalValues::buttonBaseSize()-9, GlobalValues::buttonBaseSize()-9);
    }
#ifndef SUPPORT_NEWUI
}

#endif
    setToolTip(m_tool->description());
    m_emergeAnimation = new  QPropertyAnimation(this, "size", this);
    m_emergeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_emergeAnimation->setDuration(80);
    m_emergeAnimation->setStartValue(QSize(0, 0));
    m_emergeAnimation->setEndValue(
        QSize(GlobalValues::buttonBaseSize(), GlobalValues::buttonBaseSize()));
    auto dsEffect = new QGraphicsDropShadowEffect(this);
    // dsEffect->setBlurRadius(5);
    dsEffect->setOffset(0);
    dsEffect->setColor(QColor(Qt::black));
    setGraphicsEffect(dsEffect);
}

void CaptureButton::updateIcon()
{
    setIcon(icon());
    setIconSize(size()*0.6);
    setFlat(true);
}

QVector<CaptureButton::ButtonType> CaptureButton::getIterableButtonTypes()
{
    return iterableButtonTypes;
}

QString CaptureButton::globalStyleSheet()
{
    QColor mainColor = ConfigHandler().uiMainColorValue();
    QString baseSheet = "CaptureButton {"
                        "border-width:0px;"                     // 边框宽度像素
                        "border-radius:0px;"
                        "background-color: %1; color: white }"
                        "CaptureButton:hover { background-color: %2; }"
                        "CaptureButton:pressed:!hover { "
                        "background-color: %1; }";
    // define color when mouse is hovering
    QColor contrast = ColorUtils::contrastColor(m_mainColor);

    // foreground color
    QString color = ColorUtils::colorIsDark(mainColor) ? "white" : "black";

    return baseSheet.arg(Qt::blue).arg(Qt::gray);
}

QString CaptureButton::styleSheet() const
{
    QString baseSheet = "CaptureButton { "
                        "background-color: %1; color: white}"
                        "CaptureButton:hover { background-color: %2; }"
                        "CaptureButton:pressed:!hover { "
                        "background-color: %1; }";
    // define color when mouse is hovering
    QColor contrast = ColorUtils::contrastColor(m_mainColor);
    // foreground color
    QString color = ColorUtils::colorIsDark(m_mainColor) ? "white" : "black";

    // return color;
    return baseSheet.arg(Qt::blue).arg(Qt::blue);
}

// get icon returns the icon for the type of button
QIcon CaptureButton::icon() const
{
    return m_tool->icon(m_mainColor,
                    #ifdef ENABLE_RECORD
                        m_tool->getIsInitActive()
                    #else
                        false
                    #endif
                        , m_context);
}

void CaptureButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        emit pressedButton(this);
        emit pressed();
    }
}

void CaptureButton::animatedShow()
{
    if (!isVisible()) {
        show();
        m_emergeAnimation->start();
        connect(m_emergeAnimation, &QPropertyAnimation::finished, this, [](){
        });
    }
}

CaptureTool *CaptureButton::tool() const
{
    return m_tool;
}

void CaptureButton::setColor(const QColor &c)
{
    m_mainColor = c;
    // setStyleSheet(styleSheet());
    // updateIcon();
}

QColor CaptureButton::m_mainColor = ConfigHandler().uiMainColorValue();

static std::map<CaptureButton::ButtonType, int> buttonTypeOrder {
#ifdef ENABLE_RECORD
    {
        CaptureButton::  TYPE_CUT, 0
    },
    { CaptureButton::  TYPE_LUPING, 1 },
    { CaptureButton::  TYPE_RECT, 2 },
    { CaptureButton:: TYPE_CIRCLE, 3 },
    { CaptureButton::  TYPE_LINE, 4 },
    { CaptureButton::  TYPE_ARROW, 5 },
    { CaptureButton:: TYPE_PEN, 6 },
    { CaptureButton:: TYPE_MARKER, 7 },
    { CaptureButton::  TYPE_TEXT, 8 },
    { CaptureButton::  TYPE_BLUR, 9 },
    { CaptureButton:: TYPR_UNDO, 10 },
    { CaptureButton:: TYPE_OPTION, 11 },
    { CaptureButton:: TYPE_CLOSE, 12 },
    { CaptureButton:: TYPE_COPY, 13 },
    { CaptureButton:: TYPE_SAVE, 14 },
    { CaptureButton:: TYPE_SAVEAS, 15 },
    { CaptureButton:: TYPE_PIN, 16 },
    {CaptureButton::  TYPE_RECORD_CURSOR, 17 },
    {CaptureButton::  TYPE_RECORD_CURSOR, 18 },
    {CaptureButton::  TYPE_RECORD_CURSOR, 19 },
    {CaptureButton::  TYPE_RECORD_OPTION, 20 },
    {CaptureButton::  TYPE_RECORD_START, 21 },
#else
#ifdef SUPPORT_NEWUI
    {
        CaptureButton::  TYPE_RECT, 0
    },
    { CaptureButton:: TYPE_CIRCLE, 1 },
    { CaptureButton::  TYPE_LINE, 2 },
    { CaptureButton::  TYPE_ARROW, 3 },
    { CaptureButton:: TYPE_PEN, 4 },
    { CaptureButton:: TYPE_MARKER, 5 },
    { CaptureButton::  TYPE_TEXT, 6 },
    { CaptureButton::  TYPE_BLUR, 7 },
    { CaptureButton:: TYPR_UNDO, 8 },
    { CaptureButton:: TYPE_CLOSE, 9 },
    { CaptureButton:: TYPE_COPY, 10 },
    { CaptureButton:: TYPE_SAVE, 11 },
    { CaptureButton:: TYPE_PIN, 12 },
#else
    {
        CaptureButton::  TYPE_RECT, 0
    },
    { CaptureButton:: TYPE_CIRCLE, 1 },
    { CaptureButton::  TYPE_LINE, 2 },
    { CaptureButton::  TYPE_ARROW, 3 },
    { CaptureButton:: TYPE_PEN, 4 },
    { CaptureButton:: TYPE_MARKER, 5 },
    { CaptureButton::  TYPE_TEXT, 6 },
    { CaptureButton::  TYPE_BLUR, 7 },
    { CaptureButton:: TYPR_UNDO, 8 },
    { CaptureButton:: TYPE_OPTION, 9 },
    { CaptureButton:: TYPE_CLOSE, 10 },
    { CaptureButton:: TYPE_COPY, 11 },
    { CaptureButton:: TYPE_SAVE, 12 },
    { CaptureButton:: TYPE_SAVEAS, 13 },
    { CaptureButton:: TYPE_PIN, 14 },
#endif
#endif
};

int CaptureButton::getPriorityByButton(CaptureButton::ButtonType b)
{
    auto it = buttonTypeOrder.find(b);
    return it == buttonTypeOrder.cend() ? (int)buttonTypeOrder.size() : it->second;
}

QVector<CaptureButton::ButtonType> CaptureButton::iterableButtonTypes = {
#ifdef ENABLE_RECORD
    CaptureButton:: TYPE_CUT,
    CaptureButton:: TYPE_LUPING,
#endif
    CaptureButton:: TYPE_RECT,
    CaptureButton:: TYPE_CIRCLE,
    CaptureButton:: TYPE_LINE,
    CaptureButton:: TYPE_ARROW,
    CaptureButton:: TYPE_PEN,
    CaptureButton:: TYPE_MARKER,
    CaptureButton:: TYPE_TEXT,
    CaptureButton:: TYPE_BLUR,
    CaptureButton:: TYPR_UNDO,
#ifndef SUPPORT_NEWUI
    CaptureButton:: TYPE_OPTION,
#endif
    CaptureButton:: TYPE_CLOSE,
    CaptureButton:: TYPE_COPY,
    CaptureButton:: TYPE_SAVE,
#ifndef SUPPORT_NEWUI
    CaptureButton:: TYPE_SAVEAS,
#endif
    CaptureButton:: TYPE_PIN,
#ifdef ENABLE_RECORD
    CaptureButton:: TYPE_RECORD_CURSOR,
    CaptureButton:: TYPE_RECORD_AUDIO,
    CaptureButton:: TYPE_RECORD_FOLLOW_MOUSE,
    CaptureButton:: TYPE_RECORD_OPTION,
    CaptureButton:: TYPE_RECORD_START,
#endif
};
