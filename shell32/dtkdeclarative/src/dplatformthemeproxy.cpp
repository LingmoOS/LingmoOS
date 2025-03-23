// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplatformthemeproxy.h"
#include "private/dplatformthemeproxy_p.h"

DQUICK_BEGIN_NAMESPACE

/*!
 * \~chinese \class DPlatformThemeProxy
 * \~chinese \brief 一个代理类，用于导出dtkgui的DPlatformTheme类的功能到QML使用，功能和接口基本和DPlatformTheme一致
 */

DPlatformThemeProxyPrivate::DPlatformThemeProxyPrivate(DPlatformThemeProxy *qq)
    : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
{
}

DPlatformThemeProxy::DPlatformThemeProxy(DPlatformTheme *proxy, QObject *parent)
    : QObject(parent)
    , DTK_CORE_NAMESPACE::DObject(*new DPlatformThemeProxyPrivate(this))
{
    d_func()->proxy = proxy;

    connect(proxy, &DPlatformTheme::cursorBlinkTimeChanged, this, &DPlatformThemeProxy::cursorBlinkTimeChanged);
    connect(proxy, &DPlatformTheme::cursorBlinkTimeoutChanged, this, &DPlatformThemeProxy::cursorBlinkTimeoutChanged);
    connect(proxy, &DPlatformTheme::cursorBlinkChanged, this, &DPlatformThemeProxy::cursorBlinkChanged);
    connect(proxy, &DPlatformTheme::doubleClickDistanceChanged, this, &DPlatformThemeProxy::doubleClickDistanceChanged);
    connect(proxy, &DPlatformTheme::doubleClickTimeChanged, this, &DPlatformThemeProxy::doubleClickTimeChanged);
    connect(proxy, &DPlatformTheme::dndDragThresholdChanged, this, &DPlatformThemeProxy::dndDragThresholdChanged);
    connect(proxy, &DPlatformTheme::windowRadiusChanged, this, &DPlatformThemeProxy::windowRadiusChanged);

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    // QPalette
    connect(proxy, &DPlatformTheme::windowChanged, this, &DPlatformThemeProxy::windowChanged);
    connect(proxy, &DPlatformTheme::windowTextChanged, this, &DPlatformThemeProxy::windowTextChanged);
    connect(proxy, &DPlatformTheme::baseChanged, this, &DPlatformThemeProxy::baseChanged);
    connect(proxy, &DPlatformTheme::alternateBaseChanged, this, &DPlatformThemeProxy::alternateBaseChanged);
    connect(proxy, &DPlatformTheme::toolTipBaseChanged, this, &DPlatformThemeProxy::toolTipBaseChanged);
    connect(proxy, &DPlatformTheme::toolTipTextChanged, this, &DPlatformThemeProxy::toolTipTextChanged);
    connect(proxy, &DPlatformTheme::textChanged, this, &DPlatformThemeProxy::textChanged);
    connect(proxy, &DPlatformTheme::buttonChanged, this, &DPlatformThemeProxy::buttonChanged);
    connect(proxy, &DPlatformTheme::buttonTextChanged, this, &DPlatformThemeProxy::buttonTextChanged);
    connect(proxy, &DPlatformTheme::brightTextChanged, this, &DPlatformThemeProxy::brightTextChanged);
    connect(proxy, &DPlatformTheme::lightChanged, this, &DPlatformThemeProxy::lightChanged);
    connect(proxy, &DPlatformTheme::midlightChanged, this, &DPlatformThemeProxy::midlightChanged);
    connect(proxy, &DPlatformTheme::darkChanged, this, &DPlatformThemeProxy::darkChanged);
    connect(proxy, &DPlatformTheme::midChanged, this, &DPlatformThemeProxy::midChanged);
    connect(proxy, &DPlatformTheme::shadowChanged, this, &DPlatformThemeProxy::shadowChanged);
    connect(proxy, &DPlatformTheme::highlightChanged, this, &DPlatformThemeProxy::highlightChanged);
    connect(proxy, &DPlatformTheme::highlightedTextChanged, this, &DPlatformThemeProxy::highlightedTextChanged);
    connect(proxy, &DPlatformTheme::linkChanged, this, &DPlatformThemeProxy::linkChanged);
    connect(proxy, &DPlatformTheme::linkVisitedChanged, this, &DPlatformThemeProxy::linkVisitedChanged);

    // DPalette
    connect(proxy, &DPlatformTheme::itemBackgroundChanged, this, &DPlatformThemeProxy::itemBackgroundChanged);
    connect(proxy, &DPlatformTheme::textTitleChanged, this, &DPlatformThemeProxy::textTitleChanged);
    connect(proxy, &DPlatformTheme::textTipsChanged, this, &DPlatformThemeProxy::textTipsChanged);
    connect(proxy, &DPlatformTheme::textWarningChanged, this, &DPlatformThemeProxy::textWarningChanged);
    connect(proxy, &DPlatformTheme::textLivelyChanged, this, &DPlatformThemeProxy::textLivelyChanged);
    connect(proxy, &DPlatformTheme::lightLivelyChanged, this, &DPlatformThemeProxy::lightLivelyChanged);
    connect(proxy, &DPlatformTheme::darkLivelyChanged, this, &DPlatformThemeProxy::darkLivelyChanged);
    connect(proxy, &DPlatformTheme::frameBorderChanged, this, &DPlatformThemeProxy::frameBorderChanged);
#endif

    // theme
    connect(proxy, &DPlatformTheme::themeNameChanged, this, &DPlatformThemeProxy::themeNameChanged);
    connect(proxy, &DPlatformTheme::iconThemeNameChanged, this, &DPlatformThemeProxy::iconThemeNameChanged);
    connect(proxy, &DPlatformTheme::soundThemeNameChanged, this, &DPlatformThemeProxy::soundThemeNameChanged);

    // font
    connect(proxy, &DPlatformTheme::fontNameChanged, this, &DPlatformThemeProxy::fontNameChanged);
    connect(proxy, &DPlatformTheme::monoFontNameChanged, this, &DPlatformThemeProxy::monoFontNameChanged);
    connect(proxy, &DPlatformTheme::fontPointSizeChanged, this, &DPlatformThemeProxy::fontPointSizeChanged);
    connect(proxy, &DPlatformTheme::gtkFontNameChanged, this, &DPlatformThemeProxy::gtkFontNameChanged);
}

DPlatformThemeProxy::~DPlatformThemeProxy()
{
}

int DPlatformThemeProxy::cursorBlinkTime() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->cursorBlinkTime();
}

int DPlatformThemeProxy::cursorBlinkTimeout() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->cursorBlinkTimeout();
}

bool DPlatformThemeProxy::cursorBlink() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->cursorBlink();
}

int DPlatformThemeProxy::doubleClickDistance() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->doubleClickDistance();
}

int DPlatformThemeProxy::doubleClickTime() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->doubleClickTime();
}

int DPlatformThemeProxy::dndDragThreshold() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->dndDragThreshold();
}

int DPlatformThemeProxy::windowRadius() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->windowRadius();
}

int DPlatformThemeProxy::windowRadius(int defaultValue) const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->windowRadius(defaultValue);
}

QByteArray DPlatformThemeProxy::themeName() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->themeName();
}

QByteArray DPlatformThemeProxy::iconThemeName() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->iconThemeName();
}

QByteArray DPlatformThemeProxy::soundThemeName() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->soundThemeName();
}

QByteArray DPlatformThemeProxy::fontName() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->fontName();
}

QByteArray DPlatformThemeProxy::monoFontName() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->monoFontName();
}

qreal DPlatformThemeProxy::fontPointSize() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->fontPointSize();
}

QByteArray DPlatformThemeProxy::gtkFontName() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->gtkFontName();
}

QColor DPlatformThemeProxy::activeColor() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->activeColor();
}

bool DPlatformThemeProxy::isValidPalette() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->isValidPalette();
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
QColor DPlatformThemeProxy::window() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->window();
}

QColor DPlatformThemeProxy::windowText() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->windowText();
}

QColor DPlatformThemeProxy::base() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->base();
}

QColor DPlatformThemeProxy::alternateBase() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->alternateBase();
}

QColor DPlatformThemeProxy::toolTipBase() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->toolTipBase();
}

QColor DPlatformThemeProxy::toolTipText() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->toolTipText();
}

QColor DPlatformThemeProxy::text() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->text();
}

QColor DPlatformThemeProxy::button() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->button();
}

QColor DPlatformThemeProxy::buttonText() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->buttonText();
}

QColor DPlatformThemeProxy::brightText() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->brightText();
}

QColor DPlatformThemeProxy::light() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->light();
}

QColor DPlatformThemeProxy::midlight() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->midlight();
}

QColor DPlatformThemeProxy::dark() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->dark();
}

QColor DPlatformThemeProxy::mid() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->mid();
}

QColor DPlatformThemeProxy::shadow() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->shadow();
}

QColor DPlatformThemeProxy::highlight() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->highlight();
}

QColor DPlatformThemeProxy::highlightedText() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->highlightedText();
}

QColor DPlatformThemeProxy::link() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->link();
}

QColor DPlatformThemeProxy::linkVisited() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->linkVisited();
}

QColor DPlatformThemeProxy::itemBackground() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->itemBackground();
}

QColor DPlatformThemeProxy::textTitle() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->textTitle();
}

QColor DPlatformThemeProxy::textTips() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->textTips();
}

QColor DPlatformThemeProxy::textWarning() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->textWarning();
}

QColor DPlatformThemeProxy::textLively() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->textLively();
}

QColor DPlatformThemeProxy::lightLively() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->lightLively();
}

QColor DPlatformThemeProxy::darkLively() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->darkLively();
}

QColor DPlatformThemeProxy::frameBorder() const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->frameBorder();
}
#endif

int DPlatformThemeProxy::dotsPerInch(const QString &screenName) const
{
    D_DC(DPlatformThemeProxy);

    return d->proxy->dotsPerInch(screenName);
}

void DPlatformThemeProxy::setCursorBlinkTime(int cursorBlinkTime)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setCursorBlinkTime(cursorBlinkTime);
}

void DPlatformThemeProxy::setCursorBlinkTimeout(int cursorBlinkTimeout)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setCursorBlinkTimeout(cursorBlinkTimeout);
}

void DPlatformThemeProxy::setCursorBlink(bool cursorBlink)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setCursorBlink(cursorBlink);
}

void DPlatformThemeProxy::setDoubleClickDistance(int doubleClickDistance)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setDoubleClickDistance(doubleClickDistance);
}

void DPlatformThemeProxy::setDoubleClickTime(int doubleClickTime)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setDoubleClickTime(doubleClickTime);
}

void DPlatformThemeProxy::setDndDragThreshold(int dndDragThreshold)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setDndDragThreshold(dndDragThreshold);
}

void DPlatformThemeProxy::setThemeName(const QByteArray &themeName)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setThemeName(themeName);
}

void DPlatformThemeProxy::setIconThemeName(const QByteArray &iconThemeName)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setIconThemeName(iconThemeName);
}

void DPlatformThemeProxy::setSoundThemeName(const QByteArray &soundThemeName)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setSoundThemeName(soundThemeName);
}

void DPlatformThemeProxy::setFontName(const QByteArray &fontName)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setFontName(fontName);
}

void DPlatformThemeProxy::setMonoFontName(const QByteArray &monoFontName)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setMonoFontName(monoFontName);
}

void DPlatformThemeProxy::setFontPointSize(qreal fontPointSize)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setFontPointSize(fontPointSize);
}

void DPlatformThemeProxy::setGtkFontName(const QByteArray &fontName)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setGtkFontName(fontName);
}

void DPlatformThemeProxy::setActiveColor(const QColor activeColor)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setActiveColor(activeColor);
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
void DPlatformThemeProxy::setWindow(const QColor &window)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setWindow(window);
}

void DPlatformThemeProxy::setWindowText(const QColor &windowText)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setWindowText(windowText);
}

void DPlatformThemeProxy::setBase(const QColor &base)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setBase(base);
}

void DPlatformThemeProxy::setAlternateBase(const QColor &alternateBase)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setAlternateBase(alternateBase);
}

void DPlatformThemeProxy::setToolTipBase(const QColor &toolTipBase)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setToolTipBase(toolTipBase);
}

void DPlatformThemeProxy::setToolTipText(const QColor &toolTipText)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setToolTipText(toolTipText);
}

void DPlatformThemeProxy::setText(const QColor &text)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setText(text);
}

void DPlatformThemeProxy::setButton(const QColor &button)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setButton(button);
}

void DPlatformThemeProxy::setButtonText(const QColor &buttonText)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setButtonText(buttonText);
}

void DPlatformThemeProxy::setBrightText(const QColor &brightText)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setBrightText(brightText);
}

void DPlatformThemeProxy::setLight(const QColor &light)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setLight(light);
}

void DPlatformThemeProxy::setMidlight(const QColor &midlight)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setMidlight(midlight);
}

void DPlatformThemeProxy::setDark(const QColor &dark)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setDark(dark);
}

void DPlatformThemeProxy::setMid(const QColor &mid)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setMid(mid);
}

void DPlatformThemeProxy::setShadow(const QColor &shadow)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setShadow(shadow);
}

void DPlatformThemeProxy::setHighlight(const QColor &highlight)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setHighlight(highlight);
}

void DPlatformThemeProxy::setHighlightedText(const QColor &highlightText)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setHighlightedText(highlightText);
}

void DPlatformThemeProxy::setLink(const QColor &link)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setLink(link);
}

void DPlatformThemeProxy::setLinkVisited(const QColor &linkVisited)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setLinkVisited(linkVisited);
}

void DPlatformThemeProxy::setItemBackground(const QColor &itemBackground)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setItemBackground(itemBackground);
}

void DPlatformThemeProxy::setTextTitle(const QColor &textTitle)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setTextTitle(textTitle);
}

void DPlatformThemeProxy::setTextTips(const QColor &textTips)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setTextTips(textTips);
}

void DPlatformThemeProxy::setTextWarning(const QColor &textWarning)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setTextWarning(textWarning);
}

void DPlatformThemeProxy::setTextLively(const QColor &textLively)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setTextLively(textLively);
}

void DPlatformThemeProxy::setLightLively(const QColor &lightLively)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setLightLively(lightLively);
}

void DPlatformThemeProxy::setDarkLively(const QColor &darkLively)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setDarkLively(darkLively);
}

void DPlatformThemeProxy::setFrameBorder(const QColor &frameBorder)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setFrameBorder(frameBorder);
}
#endif

void DPlatformThemeProxy::setDotsPerInch(const QString &screenName, int dpi)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setDotsPerInch(screenName, dpi);
}

void DPlatformThemeProxy::setWindowRadius(int windowRadius)
{
    D_D(DPlatformThemeProxy);

    return d->proxy->setWindowRadius(windowRadius);
}

DQUICK_END_NAMESPACE
