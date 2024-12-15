// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplatformtheme.h"
#include "dguiapplicationhelper.h"
#include "private/dplatformtheme_p.h"

#ifndef DTK_DISABLE_XCB
#include "plugins/platform/xcb/dxcbplatforminterface.h"
#endif
#ifndef DTK_DISABLE_TREELAND
#include "plugins/platform/treeland/dtreelandplatforminterface.h"
#endif
#include "private/dplatforminterface_p.h"

#include <QVariant>
#include <QTimer>
#include <QMetaProperty>
#include <QDebug>
#include <DGuiApplicationHelper>
#include <DConfig>

DGUI_BEGIN_NAMESPACE
#define DTK_PREFERENCE_NAME "org.lingmo.dtk.preference"
#define DTK_SIZE_MODE_KEY "sizeMode"
#define DTK_SCROLLBAR_POLICY_KEY "scrollBarPolicy"

static DPlatformInterfaceFactory::HelperCreator OutsideInterfaceCreator = nullptr;

void DPlatformInterfaceFactory::registerInterface(HelperCreator creator)
{
    OutsideInterfaceCreator = creator;
}

// "/lingmo/palette" 为调色板属性的存储位置
// 在x11平台下，将使用_LINGMO_PALETTE作为存储调色板数据的窗口属性
DPlatformThemePrivate::DPlatformThemePrivate(Dtk::Gui::DPlatformTheme *qq)
    : DNativeSettingsPrivate(qq, QByteArrayLiteral("/lingmo/palette"))
{

}

void DPlatformThemePrivate::onQtColorChanged(QPalette::ColorRole role, const QColor &color)
{
    if (!palette) {
        palette = new DPalette();
    }

    palette->setColor(QPalette::Normal, role, color);
    notifyPaletteChanged();
}

void DPlatformThemePrivate::onDtkColorChanged(DPalette::ColorType type, const QColor &color)
{
    if (!palette) {
        palette = new DPalette();
    }

    palette->setColor(QPalette::Normal, type, color);
    notifyPaletteChanged();
}

void DPlatformThemePrivate::notifyPaletteChanged()
{
    if (notifyPaletteChangeTimer && notifyPaletteChangeTimer->isActive())
        return;

    D_Q(DPlatformTheme);

    if (!notifyPaletteChangeTimer) {
        notifyPaletteChangeTimer = new QTimer(q);
        q->connect(notifyPaletteChangeTimer, &QTimer::timeout, q, [q, this] {
            Q_EMIT q->paletteChanged(*palette);
        });
    }

    notifyPaletteChangeTimer->start(300);
}

void DPlatformThemePrivate::onDtkPreferenceDConfigChanged(const QString &key)
{
    D_Q(DPlatformTheme);
    if (key == DTK_SIZE_MODE_KEY) {
        sizeMode = static_cast<DGuiApplicationHelper::SizeMode>(dtkPreferenceConfig->value(key).toInt());
        Q_EMIT q->sizeModeChanged(sizeMode);
    } else if (key == DTK_SCROLLBAR_POLICY_KEY) {
        scrollBarPolicy = static_cast<Qt::ScrollBarPolicy>(dtkPreferenceConfig->value(key).toInt());
        Q_EMIT q->scrollBarPolicyChanged(scrollBarPolicy);
    }
}

/*!
  \class Dtk::Gui::DPlatformTheme
  \inmodule dtkgui
  \brief 一个提供窗口主题的类.

 */
DPlatformTheme::DPlatformTheme(quint32 window, QObject *parent)
    : DNativeSettings(*new DPlatformThemePrivate(this),
                      &DPlatformTheme::staticMetaObject,
                      window, parent)
{
    D_D(DPlatformTheme);

    if (OutsideInterfaceCreator) {
        d->platformInterface = OutsideInterfaceCreator(this);
    } else {
#ifndef DTK_DISABLE_XCB
        if (DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsXWindowPlatform)) {
            d->platformInterface = new DXCBPlatformInterface(0, this);
        }
#endif

#ifndef DTK_DISABLE_TREELAND
        if (DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsWaylandPlatform)) {
            d->platformInterface = new DTreelandPlatformInterface(this);
        }
#endif
    }

    if (!d->platformInterface) {
        d->platformInterface = new DPlatformInterface(this);
    }

    d->theme = new DNativeSettings(window, QByteArray(), this);

    d->dtkPreferenceConfig = DConfig::createGeneric(DTK_PREFERENCE_NAME, "", this);
    d->sizeMode = static_cast<DGuiApplicationHelper::SizeMode>(d->dtkPreferenceConfig->value(DTK_SIZE_MODE_KEY).toInt());
    d->scrollBarPolicy = static_cast<Qt::ScrollBarPolicy>(d->dtkPreferenceConfig->value(DTK_SCROLLBAR_POLICY_KEY).toInt());
    connect(d->dtkPreferenceConfig, &DConfig::valueChanged, this, [this](const QString &key) -> void{
        D_D(DPlatformTheme);
        d->onDtkPreferenceDConfigChanged(key);
    });

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    connect(this, &DPlatformTheme::windowChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Window, std::placeholders::_1));
    connect(this, &DPlatformTheme::windowTextChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::WindowText, std::placeholders::_1));
    connect(this, &DPlatformTheme::baseChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Base, std::placeholders::_1));
    connect(this, &DPlatformTheme::alternateBaseChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::AlternateBase, std::placeholders::_1));
    connect(this, &DPlatformTheme::toolTipBaseChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::ToolTipBase, std::placeholders::_1));
    connect(this, &DPlatformTheme::toolTipTextChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::ToolTipText, std::placeholders::_1));
    connect(this, &DPlatformTheme::textChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Text, std::placeholders::_1));
    connect(this, &DPlatformTheme::buttonChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Button, std::placeholders::_1));
    connect(this, &DPlatformTheme::buttonTextChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::ButtonText, std::placeholders::_1));
    connect(this, &DPlatformTheme::brightTextChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::BrightText, std::placeholders::_1));
    connect(this, &DPlatformTheme::lightChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Light, std::placeholders::_1));
    connect(this, &DPlatformTheme::midlightChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Midlight, std::placeholders::_1));
    connect(this, &DPlatformTheme::darkChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Dark, std::placeholders::_1));
    connect(this, &DPlatformTheme::midChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Mid, std::placeholders::_1));
    connect(this, &DPlatformTheme::shadowChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Shadow, std::placeholders::_1));
    connect(this, &DPlatformTheme::highlightChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Highlight, std::placeholders::_1));
    connect(this, &DPlatformTheme::highlightedTextChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::HighlightedText, std::placeholders::_1));
    connect(this, &DPlatformTheme::linkChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::Link, std::placeholders::_1));
    connect(this, &DPlatformTheme::linkVisitedChanged, std::bind(&DPlatformThemePrivate::onQtColorChanged, d, QPalette::LinkVisited, std::placeholders::_1));
    connect(this, &DPlatformTheme::itemBackgroundChanged, std::bind(&DPlatformThemePrivate::onDtkColorChanged, d, DPalette::ItemBackground, std::placeholders::_1));
    connect(this, &DPlatformTheme::textTitleChanged, std::bind(&DPlatformThemePrivate::onDtkColorChanged, d, DPalette::TextTitle, std::placeholders::_1));
    connect(this, &DPlatformTheme::textTipsChanged, std::bind(&DPlatformThemePrivate::onDtkColorChanged, d, DPalette::TextTips, std::placeholders::_1));
    connect(this, &DPlatformTheme::textWarningChanged, std::bind(&DPlatformThemePrivate::onDtkColorChanged, d, DPalette::TextWarning, std::placeholders::_1));
    connect(this, &DPlatformTheme::textLivelyChanged, std::bind(&DPlatformThemePrivate::onDtkColorChanged, d, DPalette::TextLively, std::placeholders::_1));
    connect(this, &DPlatformTheme::lightLivelyChanged, std::bind(&DPlatformThemePrivate::onDtkColorChanged, d, DPalette::LightLively, std::placeholders::_1));
    connect(this, &DPlatformTheme::darkLivelyChanged, std::bind(&DPlatformThemePrivate::onDtkColorChanged, d, DPalette::DarkLively, std::placeholders::_1));
    connect(this, &DPlatformTheme::frameBorderChanged, std::bind(&DPlatformThemePrivate::onDtkColorChanged, d, DPalette::FrameBorder, std::placeholders::_1));
#endif
}

DPlatformTheme::DPlatformTheme(quint32 window, DPlatformTheme *parent)
    : DPlatformTheme(window, static_cast<QObject*>(parent))
{
    d_func()->parent = parent;

    // 将主题相关的属性改变信号从父主题中继承
    // 假设 A 被 B 继承，B 被 C 继承，当 A 中出发属性变化时，不仅要能通知到B，还要能通知到C
    connect(parent->d_func()->theme, SIGNAL(propertyChanged(const QByteArray &, const QVariant &)),
            d_func()->theme, SIGNAL(propertyChanged(const QByteArray &, const QVariant &)));
}

DPlatformTheme::~DPlatformTheme()
{
    D_D(DPlatformTheme);

    if (d->palette) {
        delete d->palette;
    }
    if (d->platformInterface) {
        delete d->platformInterface;
    }
}

bool DPlatformTheme::isValid() const
{
    return !themeName().isEmpty() || isValidPalette() || activeColor().isValid();
}

DPlatformTheme *DPlatformTheme::parentTheme() const
{
    D_DC(DPlatformTheme);

    return d->parent;
}

void DPlatformTheme::setFallbackProperty(bool fallback)
{
    D_D(DPlatformTheme);

    d->fallbackProperty = fallback;
}

DPalette DPlatformTheme::palette() const
{
    D_DC(DPlatformTheme);

    if (!d->palette) {
        if (!isValid())
            return DPalette();

        DPalette *pa = new DPalette();
        const_cast<DPlatformThemePrivate*>(d)->palette = pa;

#define SET_PALETTE_COLOR(Role) \
    pa->setColor(DPalette::Role, qvariant_cast<QColor>(getSetting(QByteArrayLiteral(#Role))))

        SET_PALETTE_COLOR(Window);
        SET_PALETTE_COLOR(WindowText);
        SET_PALETTE_COLOR(Base);
        SET_PALETTE_COLOR(AlternateBase);
        SET_PALETTE_COLOR(ToolTipBase);
        SET_PALETTE_COLOR(ToolTipText);
        SET_PALETTE_COLOR(Text);
        SET_PALETTE_COLOR(Button);
        SET_PALETTE_COLOR(ButtonText);
        SET_PALETTE_COLOR(BrightText);
        SET_PALETTE_COLOR(Light);
        SET_PALETTE_COLOR(Midlight);
        SET_PALETTE_COLOR(Dark);
        SET_PALETTE_COLOR(Mid);
        SET_PALETTE_COLOR(Shadow);
        SET_PALETTE_COLOR(Highlight);
        SET_PALETTE_COLOR(HighlightedText);
        SET_PALETTE_COLOR(Link);
        SET_PALETTE_COLOR(LinkVisited);
        SET_PALETTE_COLOR(ItemBackground);
        SET_PALETTE_COLOR(TextTitle);
        SET_PALETTE_COLOR(TextTips);
        SET_PALETTE_COLOR(TextWarning);
        SET_PALETTE_COLOR(TextLively);
        SET_PALETTE_COLOR(LightLively);
        SET_PALETTE_COLOR(DarkLively);
        SET_PALETTE_COLOR(FrameBorder);
    }

    return *d->palette;
}

DPalette DPlatformTheme::fetchPalette(const DPalette &base, bool *ok) const
{
    D_DC(DPlatformTheme);

    DPalette palette = base;

    if (isValidPalette() && d->palette) {
        if (ok) {
            *ok = true;
        }

        const DPalette *pa = d->palette;

        for (int i = 0; i < QPalette::NColorRoles; ++i) {
            const QColor &color = pa->color(QPalette::Normal, static_cast<QPalette::ColorRole>(i));

            if (color.isValid()) {
                palette.setColor(QPalette::Normal, static_cast<QPalette::ColorRole>(i), color);
            }
        }

        for (int i = 0; i < DPalette::NColorTypes; ++i) {
            const QColor &color = pa->color(QPalette::Normal, static_cast<DPalette::ColorType>(i));

            if (color.isValid()) {
                palette.setColor(QPalette::Normal, static_cast<DPalette::ColorType>(i), color);
            }
        }

        return d->parent ? d->parent->fetchPalette(palette, nullptr) : palette;
    }

    return d->parent ? d->parent->fetchPalette(palette, ok) : palette;
}

void DPlatformTheme::setPalette(const DPalette &palette)
{
#define SET_PALETTE(Role) \
    set##Role(palette.color(QPalette::Normal, DPalette::Role))
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    SET_PALETTE(Window);
    SET_PALETTE(WindowText);
    SET_PALETTE(Base);
    SET_PALETTE(AlternateBase);
    SET_PALETTE(ToolTipBase);
    SET_PALETTE(ToolTipText);
    SET_PALETTE(Text);
    SET_PALETTE(Button);
    SET_PALETTE(ButtonText);
    SET_PALETTE(BrightText);
    SET_PALETTE(Light);
    SET_PALETTE(Midlight);
    SET_PALETTE(Dark);
    SET_PALETTE(Mid);
    SET_PALETTE(Shadow);
    SET_PALETTE(Highlight);
    SET_PALETTE(HighlightedText);
    SET_PALETTE(Link);
    SET_PALETTE(LinkVisited);
    SET_PALETTE(ItemBackground);
    SET_PALETTE(TextTitle);
    SET_PALETTE(TextTips);
    SET_PALETTE(TextWarning);
    SET_PALETTE(TextLively);
    SET_PALETTE(LightLively);
    SET_PALETTE(DarkLively);
    SET_PALETTE(FrameBorder);
#endif
}

int DPlatformTheme::cursorBlinkTime() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->cursorBlinkTime();
}

int DPlatformTheme::cursorBlinkTimeout() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->cursorBlinkTimeout();
}

bool DPlatformTheme::cursorBlink() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->cursorBlink();
}

int DPlatformTheme::doubleClickDistance() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->doubleClickDistance();
}

int DPlatformTheme::doubleClickTime() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->doubleClickTime();
}

int DPlatformTheme::dndDragThreshold() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->dndDragThreshold();
}

int DPlatformTheme::windowRadius() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->windowRadius();
}

int DPlatformTheme::windowRadius(int defaultValue) const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->windowRadius(defaultValue);
}

QByteArray DPlatformTheme::themeName() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->themeName();
}

QByteArray DPlatformTheme::iconThemeName() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->iconThemeName();
}

QByteArray DPlatformTheme::soundThemeName() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->soundThemeName();
}

QByteArray DPlatformTheme::fontName() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->fontName();
}

QByteArray DPlatformTheme::monoFontName() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->monoFontName();
}

qreal DPlatformTheme::fontPointSize() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->fontPointSize();
}

QByteArray DPlatformTheme::gtkFontName() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->gtkFontName();
}

QColor DPlatformTheme::activeColor() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->activeColor();
}

QColor DPlatformTheme::darkActiveColor() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->darkActiveColor();
}

bool DPlatformTheme::isValidPalette() const
{
    return !allKeys().isEmpty();
}

#define GET_COLOR(Role) qvariant_cast<QColor>(getSetting(QByteArrayLiteral(#Role)))
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
QColor DPlatformTheme::window() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->window();
}

QColor DPlatformTheme::windowText() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->windowText();
}

QColor DPlatformTheme::base() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->base();
}

QColor DPlatformTheme::alternateBase() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->alternateBase();
}

QColor DPlatformTheme::toolTipBase() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->toolTipBase();
}

QColor DPlatformTheme::toolTipText() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->toolTipText();
}

QColor DPlatformTheme::text() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->text();
}

QColor DPlatformTheme::button() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->button();
}

QColor DPlatformTheme::buttonText() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->buttonText();
}

QColor DPlatformTheme::brightText() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->brightText();
}

QColor DPlatformTheme::light() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->light();
}

QColor DPlatformTheme::midlight() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->midlight();
}

QColor DPlatformTheme::dark() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->dark();
}

QColor DPlatformTheme::mid() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->mid();
}

QColor DPlatformTheme::shadow() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->shadow();
}

QColor DPlatformTheme::highlight() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->highlight();
}

QColor DPlatformTheme::highlightedText() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->highlightedText();
}

QColor DPlatformTheme::link() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->link();
}

QColor DPlatformTheme::linkVisited() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->linkVisited();
}

QColor DPlatformTheme::itemBackground() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->itemBackground();
}

QColor DPlatformTheme::textTitle() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->textTitle();
}

QColor DPlatformTheme::textTips() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->textTips();
}

QColor DPlatformTheme::textWarning() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->textWarning();
}

QColor DPlatformTheme::textLively() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->textLively();
}

QColor DPlatformTheme::lightLively() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->lightLively();
}

QColor DPlatformTheme::darkLively() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->darkLively();
}

QColor DPlatformTheme::frameBorder() const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->frameBorder();
}
#endif

int DPlatformTheme::dotsPerInch(const QString &screenName) const
{
    D_DC(DPlatformTheme);
    return d->platformInterface->dotsPerInch(screenName);
}

/*!
  \property DPlatformTheme::sizeMode
  \brief This property holds the sizeMode of the system's SizeMode.
 */
int DPlatformTheme::sizeMode() const
{
    D_DC(DPlatformTheme);
    return d->sizeMode;
}

/*!
  \property DPlatformTheme::scrollBarPolicy
  \brief This property holds the scrollBarPolicy of the system. same as Qt::ScrollBarPolicy
  \retval 0 show as needed auto hide, default
  \retval 1 always off
  \retval 2 always on
 */
int DPlatformTheme::scrollBarPolicy() const
{
    D_DC(DPlatformTheme);
    return d->scrollBarPolicy;
}

void DPlatformTheme::setCursorBlinkTime(int cursorBlinkTime)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setCursorBlinkTime(cursorBlinkTime);
}

void DPlatformTheme::setCursorBlinkTimeout(int cursorBlinkTimeout)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setCursorBlinkTimeout(cursorBlinkTimeout);
}

void DPlatformTheme::setCursorBlink(bool cursorBlink)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setCursorBlink(cursorBlink);
}

void DPlatformTheme::setDoubleClickDistance(int doubleClickDistance)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setDoubleClickDistance(doubleClickDistance);
}

void DPlatformTheme::setDoubleClickTime(int doubleClickTime)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setDoubleClickTime(doubleClickTime);
}

void DPlatformTheme::setDndDragThreshold(int dndDragThreshold)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setDndDragThreshold(dndDragThreshold);
}

void DPlatformTheme::setThemeName(const QByteArray &themeName)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setThemeName(themeName);
}

void DPlatformTheme::setIconThemeName(const QByteArray &iconThemeName)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setIconThemeName(iconThemeName);
}

void DPlatformTheme::setSoundThemeName(const QByteArray &soundThemeName)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setSoundThemeName(soundThemeName);
}

void DPlatformTheme::setFontName(const QByteArray &fontName)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setFontName(fontName);
}

void DPlatformTheme::setMonoFontName(const QByteArray &monoFontName)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setMonoFontName(monoFontName);
}

void DPlatformTheme::setFontPointSize(qreal fontPointSize)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setFontPointSize(fontPointSize);
}

void DPlatformTheme::setGtkFontName(const QByteArray &fontName)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setGtkFontName(fontName);
}

void DPlatformTheme::setActiveColor(const QColor activeColor)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setActiveColor(activeColor);
}

void DPlatformTheme::setDarkActiveColor(const QColor &activeColor)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setDarkActiveColor(activeColor);
}

#define SET_COLOR(Role) setSetting(QByteArrayLiteral(#Role), Role)
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
void DPlatformTheme::setWindow(const QColor &window)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setWindow(window);
}

void DPlatformTheme::setWindowText(const QColor &windowText)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setWindowText(windowText);
}

void DPlatformTheme::setBase(const QColor &base)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setBase(base);
}

void DPlatformTheme::setAlternateBase(const QColor &alternateBase)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setAlternateBase(alternateBase);
}

void DPlatformTheme::setToolTipBase(const QColor &toolTipBase)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setToolTipBase(toolTipBase);
}

void DPlatformTheme::setToolTipText(const QColor &toolTipText)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setToolTipText(toolTipText);
}

void DPlatformTheme::setText(const QColor &text)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setText(text);
}

void DPlatformTheme::setButton(const QColor &button)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setButton(button);
}

void DPlatformTheme::setButtonText(const QColor &buttonText)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setButtonText(buttonText);
}

void DPlatformTheme::setBrightText(const QColor &brightText)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setBrightText(brightText);
}

void DPlatformTheme::setLight(const QColor &light)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setLight(light);
}

void DPlatformTheme::setMidlight(const QColor &midlight)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setMidlight(midlight);
}

void DPlatformTheme::setDark(const QColor &dark)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setDark(dark);
}

void DPlatformTheme::setMid(const QColor &mid)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setMid(mid);
}

void DPlatformTheme::setShadow(const QColor &shadow)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setShadow(shadow);
}

void DPlatformTheme::setHighlight(const QColor &highlight)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setHighlight(highlight);
}

void DPlatformTheme::setHighlightedText(const QColor &highlightText)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setHighlightedText(highlightText);
}

void DPlatformTheme::setLink(const QColor &link)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setLink(link);
}

void DPlatformTheme::setLinkVisited(const QColor &linkVisited)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setLinkVisited(linkVisited);
}

void DPlatformTheme::setItemBackground(const QColor &itemBackground)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setItemBackground(itemBackground);
}

void DPlatformTheme::setTextTitle(const QColor &textTitle)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setTextTitle(textTitle);
}

void DPlatformTheme::setTextTips(const QColor &textTips)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setTextTips(textTips);
}

void DPlatformTheme::setTextWarning(const QColor &textWarning)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setTextWarning(textWarning);
}

void DPlatformTheme::setTextLively(const QColor &textLively)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setTextLively(textLively);
}

void DPlatformTheme::setLightLively(const QColor &lightLively)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setLightLively(lightLively);
}

void DPlatformTheme::setDarkLively(const QColor &darkLively)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setDarkLively(darkLively);
}

void DPlatformTheme::setFrameBorder(const QColor &frameBorder)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setFrameBorder(frameBorder);
}
#endif

void DPlatformTheme::setDotsPerInch(const QString &screenName, int dpi)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setDotsPerInch(screenName, dpi);
}

void DPlatformTheme::setWindowRadius(int windowRadius)
{
    D_DC(DPlatformTheme);
    return d->platformInterface->setWindowRadius(windowRadius);
}

DGUI_END_NAMESPACE

#include "moc_dplatformtheme.cpp"
