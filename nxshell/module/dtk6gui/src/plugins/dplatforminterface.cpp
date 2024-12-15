// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplatformtheme.h"
#include "private/dplatforminterface_p.h"

DGUI_BEGIN_NAMESPACE

DPlatformInterface::DPlatformInterface(DPlatformTheme *platformTheme)
    : m_platformTheme(platformTheme)
{

}

DPlatformInterface::~DPlatformInterface()
{

}

int DPlatformInterface::cursorBlinkTime() const
{
    return {};
}

int DPlatformInterface::cursorBlinkTimeout() const
{
    return {};
}

bool DPlatformInterface::cursorBlink() const
{
    return {};
}

int DPlatformInterface::doubleClickDistance() const
{
    return {};
}

int DPlatformInterface::doubleClickTime() const
{
    return {};
}

int DPlatformInterface::dndDragThreshold() const
{
    return {};
}

int DPlatformInterface::windowRadius() const
{
    return -1;
}

int DPlatformInterface::windowRadius(int defaultValue) const
{
    return defaultValue;
}

QByteArray DPlatformInterface::themeName() const
{
    return {};
}

QByteArray DPlatformInterface::iconThemeName() const
{
    return {};
}

QByteArray DPlatformInterface::soundThemeName() const
{
    return {};
}

QByteArray DPlatformInterface::fontName() const
{
    return {};
}

QByteArray DPlatformInterface::monoFontName() const
{
    return {};
}

qreal DPlatformInterface::fontPointSize() const
{
    return {};
}

QByteArray DPlatformInterface::gtkFontName() const
{
    return {};
}

QColor DPlatformInterface::activeColor() const
{
    return {};
}

QColor DPlatformInterface::darkActiveColor() const
{
    return {};
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
QColor DPlatformInterface::window() const
{
    return {};
}

QColor DPlatformInterface::windowText() const
{
    return {};
}

QColor DPlatformInterface::base() const
{
    return {};
}

QColor DPlatformInterface::alternateBase() const
{
    return {};
}

QColor DPlatformInterface::toolTipBase() const
{
    return {};
}

QColor DPlatformInterface::toolTipText() const
{
    return {};
}

QColor DPlatformInterface::text() const
{
    return {};
}

QColor DPlatformInterface::button() const
{
    return {};
}

QColor DPlatformInterface::buttonText() const
{
    return {};
}

QColor DPlatformInterface::brightText() const
{
    return {};
}

QColor DPlatformInterface::light() const
{
    return {};
}

QColor DPlatformInterface::midlight() const
{
    return {};
}

QColor DPlatformInterface::dark() const
{
    return {};
}

QColor DPlatformInterface::mid() const
{
    return {};
}

QColor DPlatformInterface::shadow() const
{
    return {};
}

QColor DPlatformInterface::highlight() const
{
    return {};
}

QColor DPlatformInterface::highlightedText() const
{
    return {};
}

QColor DPlatformInterface::link() const
{
    return {};
}

QColor DPlatformInterface::linkVisited() const
{
    return {};
}

QColor DPlatformInterface::itemBackground() const
{
    return {};
}

QColor DPlatformInterface::textTitle() const
{
    return {};
}

QColor DPlatformInterface::textTips() const
{
    return {};
}

QColor DPlatformInterface::textWarning() const
{
    return {};
}

QColor DPlatformInterface::textLively() const
{
    return {};
}

QColor DPlatformInterface::lightLively() const
{
    return {};
}

QColor DPlatformInterface::darkLively() const
{
    return {};
}

QColor DPlatformInterface::frameBorder() const
{
    return {};
}
#endif

int DPlatformInterface::dotsPerInch(const QString &) const
{
    return {};
}

int DPlatformInterface::sizeMode() const
{
    return {};
}

int DPlatformInterface::scrollBarPolicy() const
{
    return {};
}

void DPlatformInterface::setCursorBlinkTime(int)
{

}

void DPlatformInterface::setCursorBlinkTimeout(int)
{

}

void DPlatformInterface::setCursorBlink(bool)
{

}

void DPlatformInterface::setDoubleClickDistance(int)
{

}

void DPlatformInterface::setDoubleClickTime(int)
{

}

void DPlatformInterface::setDndDragThreshold(int)
{

}

void DPlatformInterface::setThemeName(const QByteArray &)
{

}

void DPlatformInterface::setIconThemeName(const QByteArray &)
{

}

void DPlatformInterface::setSoundThemeName(const QByteArray &)
{

}

void DPlatformInterface::setFontName(const QByteArray &)
{

}

void DPlatformInterface::setMonoFontName(const QByteArray &)
{

}

void DPlatformInterface::setFontPointSize(qreal)
{

}

void DPlatformInterface::setGtkFontName(const QByteArray &)
{

}

void DPlatformInterface::setActiveColor(const QColor)
{

}

void DPlatformInterface::setDarkActiveColor(const QColor &)
{

}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
void DPlatformInterface::setWindow(const QColor &)
{

}

void DPlatformInterface::setWindowText(const QColor &)
{

}

void DPlatformInterface::setBase(const QColor &)
{

}

void DPlatformInterface::setAlternateBase(const QColor &)
{

}

void DPlatformInterface::setToolTipBase(const QColor &)
{

}

void DPlatformInterface::setToolTipText(const QColor &)
{

}

void DPlatformInterface::setText(const QColor &)
{

}

void DPlatformInterface::setButton(const QColor &)
{

}

void DPlatformInterface::setButtonText(const QColor &)
{

}

void DPlatformInterface::setBrightText(const QColor &)
{

}

void DPlatformInterface::setLight(const QColor &)
{

}

void DPlatformInterface::setMidlight(const QColor &)
{

}

void DPlatformInterface::setDark(const QColor &)
{

}

void DPlatformInterface::setMid(const QColor &)
{

}

void DPlatformInterface::setShadow(const QColor &)
{

}

void DPlatformInterface::setHighlight(const QColor &)
{

}

void DPlatformInterface::setHighlightedText(const QColor &)
{

}

void DPlatformInterface::setLink(const QColor &)
{

}

void DPlatformInterface::setLinkVisited(const QColor &)
{

}

void DPlatformInterface::setItemBackground(const QColor &)
{

}

void DPlatformInterface::setTextTitle(const QColor &)
{

}

void DPlatformInterface::setTextTips(const QColor &)
{

}

void DPlatformInterface::setTextWarning(const QColor &)
{

}

void DPlatformInterface::setTextLively(const QColor &)
{

}

void DPlatformInterface::setLightLively(const QColor &)
{

}

void DPlatformInterface::setDarkLively(const QColor &)
{

}

void DPlatformInterface::setFrameBorder(const QColor &)
{

}
#endif

void DPlatformInterface::setDotsPerInch(const QString &, int)
{

}

void DPlatformInterface::setWindowRadius(int)
{

}

DGUI_END_NAMESPACE
