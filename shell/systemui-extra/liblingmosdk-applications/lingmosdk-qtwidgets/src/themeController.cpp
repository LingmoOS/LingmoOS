/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "themeController.h"
#include <QDebug>
#include <QVariant>
#include <QPainter>
#include <QGuiApplication>
#include <QPainterPath>
#include <QStringList>
#include <QApplication>

static  WidgetThemeFlag g_widgetThemeFlag = DefaultTheme;
static  ThemeFlag g_themeFlag = LightTheme;
static  IconFlag g_iconFlag = ClassicStyle;
static  int g_fontSize = 11;
static  QGSettings* g_gsetting=nullptr;
QStringList applist{
//    "lingmo-nm",
    "lingmo-menu",
    "lingmo-panel",
    "lingmo-sidebar",
    "lingmo-volume-control-applet-qt",
    "panellingmo-panel",
    "lingmo-power-manager-tray",
//    "lingmo-bluetooth",
    "sogouimebs",
    "lingmo-device-daemoon",
    "lingmo-notifications",
    "lingmo-flash-disk",
    "mktip",
    "lingmo-video"
};

static inline qreal mixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b - a) * bias;
}
static inline QColor adjustColor(const QColor c, int hRange = 0, int sRange = 0, int lRange = 0, int aRange = 0)
{
    QColor cc;
    int s, l, a;

    if((c.hslSaturationF() + sRange * 1.0 / 100.0) > 1){
//        qWarning() << "adjustColor s out of range1";
        s = 255;
    }
    else if((c.hslSaturationF() + sRange * 1.0 / 100.0) < 0){
//        qWarning() << "adjustColor s out of range0";
        s = 0;
    }
    else
        s = (c.hslSaturationF() + sRange * 1.0 / 100.0) *255.0;

    if((c.lightnessF() + lRange * 1.0 / 100.0) > 0.97){//根据设计稿寻光色彩识别   l应<=0.97
        qWarning() << "adjustColor l out of range1" << c.lightnessF() << lRange << (c.lightnessF() + lRange * 1.0 / 100.0);
        l = 255.0 * 0.97;
    }
    else if((c.lightnessF() + lRange * 1.0 / 100.0) < 0.15){//根据设计稿上l最小取的均值
        qWarning() << "adjustColor l out of range0";
        l = 0.15 * 255.0;
    }
    else
        l = (c.lightnessF() + lRange * 1.0 / 100.0) *255.0;

    if((c.alphaF() + aRange * 1.0 / 100.0) > 1){
        qWarning() << "adjustColor a out of range1";
        a = 255;
    }
    else if((c.alphaF() + aRange * 1.0 / 100.0) < 0){
        qWarning() << "adjustColor a out of range0";
        a = 0;
    }
    else
        a = (c.alphaF() + aRange * 1.0 / 100.0) *255.0;
    cc.setHsl(c.hslHue() + hRange, s, l, a);
    return  cc;
}

QPixmap ThemeController::drawSymbolicColoredPixmap(const QPixmap &source)
{
    QColor gray(128,128,128);
    QColor standard (31,32,34);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red())<20 && qAbs(color.green()-gray.green())<20 && qAbs(color.blue()-gray.blue())<20) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
                else if(qAbs(color.red()-standard.red())<20 && qAbs(color.green()-standard.green())<20 && qAbs(color.blue()-standard.blue())<20)
                {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
                else
                {
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap ThemeController::drawColoredPixmap(const QPixmap &source, const QColor &sampleColor)
{
//    QColor gray(128,128,128);
//    QColor standard (31,32,34);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++)
    {
        for (int y = 0; y < img.height(); y++)
        {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0)
            {
                color.setRed(sampleColor.red());
                color.setGreen(sampleColor.green());
                color.setBlue(sampleColor.blue());
                img.setPixelColor(x, y, color);
             }
        }
    }
    return QPixmap::fromImage(img);
}

QColor ThemeController::getCurrentIconColor()
{
    QPixmap pixmap = QIcon::fromTheme("open-menu-symbolic").pixmap(16,16);
    QImage img = pixmap.toImage();
    for (int x = 0; x < img.width(); x++)
    {
        for (int y = 0; y < img.height(); y++)
        {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0)
            {
                return color;
            }
        }
    }
}

QColor ThemeController::mixColor(const QColor &c1, const QColor &c2, qreal bias)
{
    if (bias <= 0.0) {
        return c1;
    }
    if (bias >= 1.0) {
        return c2;
    }
    if (qIsNaN(bias)) {
        return c1;
    }

    qreal r = mixQreal(c1.redF(),   c2.redF(),   bias);
    qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
    qreal b = mixQreal(c1.blueF(),  c2.blueF(),  bias);
    qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

    return QColor::fromRgbF(r, g, b, a);
}

WidgetThemeFlag ThemeController::widgetTheme()
{
    return g_widgetThemeFlag;
}

ThemeFlag ThemeController::themeMode()
{
    return g_themeFlag;
}

IconFlag ThemeController::iconTheme()
{
    return g_iconFlag;
}

int ThemeController::systemFontSize()
{
    if(!g_gsetting)
        return 11;
    if(g_gsetting->keys().contains("systemFontSize"))
    {
        g_fontSize = (int)g_gsetting->get("systemFontSize").toDouble();
    }
    return g_fontSize;
}

QPixmap ThemeController::drawFashionBackground(const QRect&rect,int sub_width,int sub_height,int radius, int flag)
{

//    int radius = 6;
//    int sub_width = 32;
//    int sub_height = 36;
    QPixmap framePixmap(rect.size());

    framePixmap.fill(Qt::transparent);
    QRect drawRect;
    drawRect = rect.adjusted(0, 0, 1, 1);

    auto baseColor = qApp->palette().color(QPalette::Active, QPalette::Button);

    QColor startColor = mixColor(baseColor,QColor(Qt::white),0.5);
    QColor endColor = mixColor(baseColor,QColor(Qt::black),0.1);

    QLinearGradient linearGradient;
    QPainterPath path;
    if(flag) //right
    {
        path.moveTo(drawRect.bottomRight() - QPoint(0,radius + sub_height));
        path.lineTo(drawRect.bottomRight() - QPoint(0,radius));
        path.quadTo(drawRect.bottomRight(), drawRect.bottomRight() - QPoint(radius,0));
        path.lineTo(drawRect.bottomRight() - QPoint(radius + sub_width,0));
        path.quadTo(drawRect.bottomRight(),drawRect.bottomRight() - QPoint(0,radius + sub_height));

        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        linearGradient.setStart(drawRect.right(), drawRect.bottom() - (radius + sub_height));
        linearGradient.setFinalStop(drawRect.right(), drawRect.bottom());
    }
    else //left
    {
        path.moveTo(drawRect.bottomLeft() - QPoint(0,radius + sub_height));
        path.lineTo(drawRect.bottomLeft() - QPoint(0,radius));
        path.quadTo(drawRect.bottomLeft(), drawRect.bottomLeft() + QPoint(radius,0));
        path.lineTo(drawRect.bottomLeft() + QPoint(radius + sub_width,0));
        path.quadTo(drawRect.bottomLeft(),drawRect.bottomLeft() - QPoint(0,radius + sub_height));

        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        linearGradient.setStart(drawRect.left(), drawRect.bottom() - (radius + sub_height));
        linearGradient.setFinalStop(drawRect.left(), drawRect.bottom());
    }
    QPainter painter(&framePixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    painter.setBrush(linearGradient);
    painter.drawPath(path);
    return framePixmap;
}

QColor ThemeController::lanhuHSLToQtHsl(int h, int s, int l, int a)
{
    QColor c;
    c.setHsl(h, s * 255.0 /100.0, l * 255.0 /100.0, a);
    return c;
}

QColor ThemeController::adjustH(const QColor c, int range)
{
    QColor cc;
    if((c.hslHue() + range) >= 0)
        cc.setHsl(c.hslHue() + range, c.hslSaturation(), c.lightness(), c.alpha());
    else
        cc.setHsl(c.hslHue(), c.hslSaturation(), c.lightness(), c.alpha());

    return  cc;
}

QColor ThemeController::adjustS(const QColor c, int range)
{
    QColor cc;
    int s;

    if((c.hslSaturationF() + range * 1.0 / 100.0) > 1){
//        qWarning() << "adjustS s out of range1";
        s = 255;
    }
    else if((c.hslSaturationF() + range * 1.0 / 100.0) < 0){
//        qWarning() << "adjustS s out of range0";
        s = 0;
    }
    else
        s = (c.hslSaturationF() + range * 1.0 / 100.0) *255.0;
    cc.setHsl(c.hslHue(), s, c.lightness(), c.alpha());
    return  cc;
}

QColor ThemeController::adjustL(const QColor c, int range)
{
    QColor cc;
    int l;

    if((c.lightnessF() + range * 1.0 / 100.0) > 0.97){
//        qWarning() << "adjustL l out of range1";
        l = 255.0 * 0.97;
    }
    else if((c.lightnessF() + range * 1.0 / 100.0) < 0.15){
//        qWarning() << "adjustL l out of range0" << c.lightnessF() << range << (c.lightnessF() + range * 1.0 / 100.0);
        l = 255.0 * 0.15;
    }
    else
        l = (c.lightnessF() + range * 1.0 / 100.0) *255.0;

    cc.setHsl(c.hslHue(), c.hslSaturation(), l, c.alpha());
    return  cc;
}

QColor ThemeController::adjustA(const QColor c, int range)
{
    QColor cc;
    int a;
    if((c.alphaF() + range * 1.0 / 100.0) > 1){
//        qWarning() << "adjustA a out of range1";
        a = 255;
    }
    else if((c.alphaF() + range * 1.0 / 100.0) < 0){
//        qWarning() << "adjustA a out of range0";
        a = 0;
    }
    else
        a = (c.alphaF() + range * 1.0 / 100.0) *255.0;


    cc.setHsl(c.hslHue(), c.hslSaturation(), c.lightness(), a);
    return  cc;
}

QColor ThemeController::highlightClick(bool isDark, QPalette p)
{
    if(isDark){
        return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 0, -25, -34);//hightlight->210,72%,71% 210,47%,37%
    }
    return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 3, 10, 31);//hightlight->209,87%,57% 212,97%,88%
}

QColor ThemeController::highlightHover(bool isDark, QPalette p)
{
    if(isDark){
        return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 0, -26, -40);//210,46%,31%
    }
    return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 4, 2, 36);//213,89%,93%
}

QColor ThemeController::buttonDarkClick(bool isDark, QPalette p)
{
    if(isDark){
        return adjustColor(p.color(QPalette::Disabled, QPalette::ButtonText), 0, 1, 5);
    }
    return QColor();
}

//void ThemeController::normalButtonColor(bool isDark, QPalette p, QPen &defaultPen, QBrush &defaultBrush, QPen &hoverPen, QBrush &hoverBrush, QPen &clickPen, QBrush &clickBrush, QPen &disablePen, QBrush &disableBrush)
//{
//    //default
//    if(isDark){
//        defaultPen = QPen(QBrush(adjustColor(p.color(QPalette::Active, QPalette::Button), 0, 0, 8)),
//                          1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
//    }
//    else
//        defaultPen = QPen(QBrush(adjustColor(p.color(QPalette::Active, QPalette::Button), 0, 0, -25)),
//                          1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

//    defaultBrush = p.brush(QPalette::Active, QPalette::Button);

//    //hover
//    if (isDark) {
//        //hightlight->210,72%,71% 210,67%,56%
//        hoverPen = QPen(QBrush(adjustColor(p.color(QPalette::Active, QPalette::Highlight), 0, -5, -15)),
//                        1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
//    } else {
//        //hightlight->209,87%,57% 213,73%,72%
//        hoverPen = QPen(QBrush(adjustColor(p.color(QPalette::Active, QPalette::Highlight), 4, -14, 15)),
//                        1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
//    }
//    hoverBrush = QBrush(highlightHover(isDark, p));


//    //click
//    if (isDark) {
//        //hightlight->210,72%,71%
//        clickPen = QPen(p.brush(QPalette::Active, QPalette::Highlight),
//                        1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
//    } else {
//        //hightlight->209,87%,57% 214,66%,60%
//        clickPen = QPen(QBrush(adjustColor(p.color(QPalette::Active, QPalette::Highlight), 5, -21, 3)),
//                        1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
//    }
//    clickBrush = QBrush(highlightClick(isDark, p));

//    //disable
//    if (isDark) {
//        disablePen = QPen(QBrush(adjustL(p.color(QPalette::Disabled, QPalette::Button), 18)),
//                          1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
//        disableBrush = p.brush(QPalette::Disabled, QPalette::Button);

//    } else {
//        disablePen = QPen(p.brush(QPalette::Disabled, QPalette::Button),
//                          1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
//        disableBrush = p.brush(QPalette::Inactive, QPalette::Button);
//    }
//}

bool ThemeController::systemLang()
{
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ")
        return true;
    else
        return false;
}

ThemeController::ThemeController()
    :m_gsetting(nullptr)
{
    if(QGSettings::isSchemaInstalled(FITTHEMEWINDOW))
    {
        m_gsetting = new QGSettings(FITTHEMEWINDOW);
        g_gsetting = m_gsetting;
        initThemeStyle();
    }
}

ThemeController::~ThemeController()
{

}

void ThemeController::initThemeStyle()
{
    if(!m_gsetting)
        return;
    if(m_gsetting->keys().contains("styleName"))
    {
        QString styleName = m_gsetting->get("styleName").toString();
        if(styleName == "lingmo-dark" || styleName == "lingmo-black" || (styleName == "lingmo-default" && applist.contains(QApplication::applicationName())))
            g_themeFlag = DarkTheme;
        else
            g_themeFlag = LightTheme;
    }

    //初始化图标主题
    if(m_gsetting->keys().contains("iconThemeName"))
    {
        QString iconThemeName = m_gsetting->get("iconThemeName").toString();
        if(iconThemeName == "lingmo-icon-theme-default")
            g_iconFlag = DefaultStyle;
        else
        {
            g_iconFlag = ClassicStyle;
        }
    }
    if(m_gsetting->keys().contains("widgetThemeName"))
    {
        //初始化样式主题
        QString widgetThemeName = m_gsetting->get("widgetThemeName").toString();
        if(widgetThemeName == "default")
            g_widgetThemeFlag = DefaultTheme;
        else if(widgetThemeName == "classical")
            g_widgetThemeFlag = ClassicTheme;
        else
           g_widgetThemeFlag = FashionTheme;
    }
}
