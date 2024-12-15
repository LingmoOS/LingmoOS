// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colormenu.h"
#include "settings.h"
#include <QGraphicsDropShadowEffect>
#include "utils.h"
#include <QPoint>
#include <QPen>
#include <QTimer>
#include <QClipboard>
#include <QPainter>
#include <QApplication>
#include <QDebug>

ColorMenu::ColorMenu(int x, int y, int size, QColor color, QWidget *parent) : QWidget(parent)
{
    // Init window flags.
    setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
//    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
//    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    installEventFilter(this);

    // Init attributes.
    clickMenuItem = false;
    menuOffsetX = 10;
    menuOffsetY = 40;
    shadowBottomMargin = 20;
    shadowXMargin = 20;
    windowColor = color;
    windowSize = size;
    windowX = x;
    windowY = y;

    // Add drop shadow window effect.
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
    effect->setColor(QColor(0, 0, 0, 255 * 2 / 10));
    effect->setBlurRadius(10);
    effect->setXOffset(0);
    effect->setYOffset(5);
    this->setGraphicsEffect(effect);

    // Build menu and menu actions.
    colorMenu = new QMenu();
    connect(colorMenu, &QMenu::aboutToHide, this, [&]() {
        QTimer::singleShot(200, this, [&]() {
            if (!clickMenuItem) {
                exit();
            }
        });
    });

    rgbAction = new QAction("RGB", this);
    rgbAction->setCheckable(true);
    connect(rgbAction, &QAction::triggered, this, &ColorMenu::copyRGBColor);

    rgbFloatAction = new QAction("RGB (1.0)", this);
    rgbFloatAction->setCheckable(true);
    connect(rgbFloatAction, &QAction::triggered, this, &ColorMenu::copyFloatRGBColor);

    rgbaAction = new QAction("RGBA", this);
    rgbaAction->setCheckable(true);
    connect(rgbaAction, &QAction::triggered, this, &ColorMenu::copyRGBAColor);

    rgbaFloatAction = new QAction("RGBA (1.0)", this);
    rgbaFloatAction->setCheckable(true);
    connect(rgbaFloatAction, &QAction::triggered, this, &ColorMenu::copyFloatRGBAColor);

    hexAction = new QAction("HEX", this);
    hexAction->setCheckable(true);
    connect(hexAction, &QAction::triggered, this, &ColorMenu::copyHexColor);

    cmykAction = new QAction("CMYK", this);
    cmykAction->setCheckable(true);
    connect(cmykAction, &QAction::triggered, this, &ColorMenu::copyCmykColor);

    hsvAction = new QAction("HSV", this);
    hsvAction->setCheckable(true);
    connect(hsvAction, &QAction::triggered, this, &ColorMenu::copyHsvColor);

    colorMenu->addAction(rgbAction);
    colorMenu->addAction(rgbFloatAction);
    colorMenu->addAction(rgbaAction);
    colorMenu->addAction(rgbaFloatAction);
    colorMenu->addAction(hexAction);
    colorMenu->addAction(cmykAction);
    colorMenu->addAction(hsvAction);

    // Set menu action check status with color type.
    Settings *settings = new Settings();
    QString colorType = settings->getOption("color_type", "HEX").toString();

    if (colorType == "HEX") {
        hexAction->setChecked(true);
    } else if (colorType == "RGB") {
        rgbAction->setChecked(true);
    } else if (colorType == "RGBA") {
        rgbaAction->setChecked(true);
    } else if (colorType == "Float_RGB") {
        rgbFloatAction->setChecked(true);
    } else if (colorType == "Float_RGBA") {
        rgbaFloatAction->setChecked(true);
    } else if (colorType == "CMYK") {
        cmykAction->setChecked(true);
    } else if (colorType == "HSV") {
        hsvAction->setChecked(true);
    }

    // Move and resize window.
    move(x - shadowXMargin, y);
    resize(windowSize + shadowXMargin * 2, windowSize + shadowBottomMargin);
}

ColorMenu::~ColorMenu()
{
    delete hexAction;
    delete rgbAction;
    delete rgbFloatAction;
    delete rgbaAction;
    delete rgbaFloatAction;
    delete cmykAction;
    delete colorMenu;
}

void ColorMenu::paintEvent(QPaintEvent *)
{
    int x = shadowXMargin;

    QPainter painter(this);
    painter.setOpacity(1);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setBrush(windowColor);
    painter.drawRect(QRect(x + 1, 1, windowSize - 3, windowSize - 3));
    painter.setPen(windowColor);
    painter.drawRect(QRect(x + 1, 1, windowSize - 3, windowSize - 3));

    painter.setRenderHint(QPainter::Antialiasing, false);
    QPen outsidePen("#000000");
    outsidePen.setWidth(1);
    painter.setOpacity(0.2);
    painter.setPen(outsidePen);
    painter.drawRect(QRect(x, 0, windowSize - 1, windowSize - 1));

    painter.setRenderHint(QPainter::Antialiasing, false);
    QPen insidePen("#ffffff");
    insidePen.setWidth(2);
    insidePen.setJoinStyle(Qt::MiterJoin);
    painter.setOpacity(0.5);
    painter.setPen(insidePen);
    painter.drawRect(QRect(x + 2, 2, windowSize - 4, windowSize - 4));
}

//void ColorMenu::showMenu()
//{
//    colorMenu->exec(QPoint(windowX - windowSize / 2 + menuOffsetX, windowY - windowSize / 2 + menuOffsetY));
//}

void ColorMenu::copyRGBColor()
{
    clickMenuItem = true;
    copyColor(windowColor, "RGB");
}

void ColorMenu::copyFloatRGBColor()
{
    clickMenuItem = true;
    copyColor(windowColor, "Float_RGB");
}

void ColorMenu::copyRGBAColor()
{
    clickMenuItem = true;
    copyColor(windowColor, "RGBA");
}

void ColorMenu::copyFloatRGBAColor()
{
    clickMenuItem = true;
    copyColor(windowColor, "Float_RGBA");
}

void ColorMenu::copyHexColor()
{
    clickMenuItem = true;
    copyColor(windowColor, "HEX");
}

void ColorMenu::copyCmykColor()
{
    clickMenuItem = true;
    copyColor(windowColor, "CMYK");
}

void ColorMenu::copyHsvColor()
{
    clickMenuItem = true;
    copyColor(windowColor, "HSV");
}
