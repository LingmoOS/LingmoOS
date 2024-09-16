// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplatformtheme.h"
#include "private/dplatformtheme_p.h"

#include <QVariant>
#include <QTimer>
#include <QMetaProperty>
#include <QDebug>

#include <functional>

DGUI_BEGIN_NAMESPACE

// "/deepin/palette" 为调色板属性的存储位置
// 在x11平台下，将使用_DEEPIN_PALETTE作为存储调色板数据的窗口属性
DPlatformThemePrivate::DPlatformThemePrivate(Dtk::Gui::DPlatformTheme *qq)
    : DNativeSettingsPrivate(qq, QByteArrayLiteral("/deepin/palette"))
{

}

void DPlatformThemePrivate::_q_onThemePropertyChanged(const QByteArray &name, const QVariant &value)
{
    D_Q(DPlatformTheme);

    // 转发属性变化的信号，此信号来源可能为parent theme或“非调色板”的属性变化。
    // 使用队列的形式转发，避免多次发出同样的信号
    q->staticMetaObject.invokeMethod(q, "propertyChanged", Qt::QueuedConnection,
                                     Q_ARG(const QByteArray&, name), Q_ARG(const QVariant&, value));

    if (QByteArrayLiteral("Gtk/FontName") == name) {
        Q_EMIT q->gtkFontNameChanged(value.toByteArray());
        return;
    }

    if (name.startsWith("Qt/DPI/")) {
        const QString &screen_name = QString::fromLocal8Bit(name.mid(7));

        if (!screen_name.isEmpty()) {
            bool ok = false;
            int dpi = value.toInt(&ok);

            Q_EMIT q->dotsPerInchChanged(screen_name, ok ? dpi : -1);
        }

        return;
    }

    if (QByteArrayLiteral("Xft/DPI") == name) {
        bool ok = false;
        int dpi = value.toInt(&ok);
        Q_EMIT q->dotsPerInchChanged(QString(), ok ? dpi : -1);
    }

    const QByteArrayList &list = name.split('/');

    if (list.count() != 2)
        return;

    QByteArray pn = list.last();

    if (pn.isEmpty())
        return;

    // 转换首字母为小写
    pn[0] = QChar(pn.at(0)).toLower().toLatin1();

    // 直接使用静态的meta object，防止通过metaObject函数调用到dynamic metaobject
    const QMetaObject *mo = &DPlatformTheme::staticMetaObject;
    int index = mo->indexOfProperty(pn.constData());

    if (index < 0)
        return;

    const QMetaProperty &p = mo->property(index);
    bool is_parent_signal = q->sender() != theme;

    // 当自己的属性有效时应该忽略父主题的属性变化信号，优先以自身的属性值为准。
    if (is_parent_signal && p.read(q).isValid()) {
        return;
    }

    if (p.hasNotifySignal()) {
        // invoke会做Q_ASSERT(mobj->cast(object))判断, DPlatformTheme的dynamic metaObject为
        // qt5platform-plugin插件的DNativeSettings. 导致崩溃.
        // invokeOnGadget与invoke代码逻辑一致, 只是少了异步支持.
        if (!p.notifySignal().invokeOnGadget(q, QGenericArgument(value.typeName(), value.constData())))
            qWarning() << "_q_onThemePropertyChanged() error when notify signal" << p.notifySignal().name();
    }
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

    d->theme = new DNativeSettings(window, QByteArray(), this);
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
    connect(d->theme, SIGNAL(propertyChanged(const QByteArray &, const QVariant &)),
            this, SLOT(_q_onThemePropertyChanged(const QByteArray &, const QVariant &)));
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

#define FETCH_PROPERTY(Name, Function) \
    D_DC(DPlatformTheme); \
    QVariant value = d->theme->getSetting(QByteArrayLiteral(Name)); \
    if (d->fallbackProperty && !value.isValid() && d->parent) \
        return d->parent->Function(); \

#define FETCH_PROPERTY_WITH_ARGS(Name, Function, Args) \
    D_DC(DPlatformTheme); \
    QVariant value = d->theme->getSetting(Name); \
    if (d->fallbackProperty && !value.isValid() && d->parent) \
        return d->parent->Function(Args); \

int DPlatformTheme::cursorBlinkTime() const
{
    FETCH_PROPERTY("Net/CursorBlinkTime", cursorBlinkTime)

    return value.toInt();
}

int DPlatformTheme::cursorBlinkTimeout() const
{
    FETCH_PROPERTY("Net/CursorBlinkTimeout", cursorBlinkTimeout)

    return value.toInt();
}

bool DPlatformTheme::cursorBlink() const
{
    FETCH_PROPERTY("Net/CursorBlink", cursorBlink)

    return value.toInt();
}

int DPlatformTheme::doubleClickDistance() const
{
    FETCH_PROPERTY("Net/DoubleClickDistance", doubleClickDistance)

    return value.toInt();
}

int DPlatformTheme::doubleClickTime() const
{
    FETCH_PROPERTY("Net/DoubleClickTime", doubleClickTime)

    return value.toInt();
}

int DPlatformTheme::dndDragThreshold() const
{
    FETCH_PROPERTY("Net/DndDragThreshold", dndDragThreshold)

    return value.toInt();
}

int DPlatformTheme::windowRadius() const
{
    return windowRadius(-1);
}

int DPlatformTheme::windowRadius(int defaultValue) const
{
    Q_D(const DPlatformTheme);

    QVariant value = d->theme->getSetting(QByteArrayLiteral("DTK/WindowRadius"));
    bool ok = false;

    if (d->fallbackProperty && !value.isValid() && d->parent)
        return d->parent->windowRadius(defaultValue);

    int radius = value.toInt(&ok);

    return ok ? radius : defaultValue;
}

QByteArray DPlatformTheme::themeName() const
{
    FETCH_PROPERTY("Net/ThemeName", themeName)

    return value.toByteArray();
}

QByteArray DPlatformTheme::iconThemeName() const
{
    FETCH_PROPERTY("Net/IconThemeName", iconThemeName)

    return value.toByteArray();
}

QByteArray DPlatformTheme::soundThemeName() const
{
    FETCH_PROPERTY("Net/SoundThemeName", soundThemeName)

    return value.toByteArray();
}

QByteArray DPlatformTheme::fontName() const
{
    FETCH_PROPERTY("Qt/FontName", fontName)

    return value.toByteArray();
}

QByteArray DPlatformTheme::monoFontName() const
{
    FETCH_PROPERTY("Qt/MonoFontName", monoFontName)

    return value.toByteArray();
}

qreal DPlatformTheme::fontPointSize() const
{
    FETCH_PROPERTY("Qt/FontPointSize", fontPointSize)

    return value.toDouble();
}

QByteArray DPlatformTheme::gtkFontName() const
{
    FETCH_PROPERTY("Gtk/FontName", gtkFontName)

    return value.toByteArray();
}

QColor DPlatformTheme::activeColor() const
{
    FETCH_PROPERTY("Qt/ActiveColor", activeColor)

    return qvariant_cast<QColor>(value);
}

bool DPlatformTheme::isValidPalette() const
{
    return !allKeys().isEmpty();
}

#define GET_COLOR(Role) qvariant_cast<QColor>(getSetting(QByteArrayLiteral(#Role)))
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
QColor DPlatformTheme::window() const
{
    return GET_COLOR(window);
}

QColor DPlatformTheme::windowText() const
{
    return GET_COLOR(windowText);
}

QColor DPlatformTheme::base() const
{
    return GET_COLOR(base);
}

QColor DPlatformTheme::alternateBase() const
{
    return GET_COLOR(alternateBase);
}

QColor DPlatformTheme::toolTipBase() const
{
    return GET_COLOR(toolTipBase);
}

QColor DPlatformTheme::toolTipText() const
{
    return GET_COLOR(toolTipText);
}

QColor DPlatformTheme::text() const
{
    return GET_COLOR(text);
}

QColor DPlatformTheme::button() const
{
    return GET_COLOR(button);
}

QColor DPlatformTheme::buttonText() const
{
    return GET_COLOR(buttonText);
}

QColor DPlatformTheme::brightText() const
{
    return GET_COLOR(brightText);
}

QColor DPlatformTheme::light() const
{
    return GET_COLOR(light);
}

QColor DPlatformTheme::midlight() const
{
    return GET_COLOR(midlight);
}

QColor DPlatformTheme::dark() const
{
    return GET_COLOR(dark);
}

QColor DPlatformTheme::mid() const
{
    return GET_COLOR(mid);
}

QColor DPlatformTheme::shadow() const
{
    return GET_COLOR(shadow);
}

QColor DPlatformTheme::highlight() const
{
    return GET_COLOR(highlight);
}

QColor DPlatformTheme::highlightedText() const
{
    return GET_COLOR(highlightedText);
}

QColor DPlatformTheme::link() const
{
    return GET_COLOR(link);
}

QColor DPlatformTheme::linkVisited() const
{
    return GET_COLOR(linkVisited);
}

QColor DPlatformTheme::itemBackground() const
{
    return GET_COLOR(itemBackground);
}

QColor DPlatformTheme::textTitle() const
{
    return GET_COLOR(textTitle);
}

QColor DPlatformTheme::textTips() const
{
    return GET_COLOR(textTips);
}

QColor DPlatformTheme::textWarning() const
{
    return GET_COLOR(textWarning);
}

QColor DPlatformTheme::textLively() const
{
    return GET_COLOR(textLively);
}

QColor DPlatformTheme::lightLively() const
{
    return GET_COLOR(lightLively);
}

QColor DPlatformTheme::darkLively() const
{
    return GET_COLOR(darkLively);
}

QColor DPlatformTheme::frameBorder() const
{
    return GET_COLOR(frameBorder);
}
#endif

int DPlatformTheme::dotsPerInch(const QString &screenName) const
{
    bool ok = false;

    if (!screenName.isEmpty()) {
        FETCH_PROPERTY_WITH_ARGS("Qt/DPI/" + screenName.toLocal8Bit(), dotsPerInch, screenName);
        int dpi = value.toInt(&ok);

        if (ok)
            return dpi;
    }

    FETCH_PROPERTY_WITH_ARGS("Xft/DPI", dotsPerInch, screenName);
    int dpi = value.toInt(&ok);
    return ok ? dpi : -1;
}

/*!
  \property DPlatformTheme::sizeMode
  \brief This property holds the sizeMode of the system's SizeMode.
 */
int DPlatformTheme::sizeMode() const
{
    D_DC(DPlatformTheme);
    QVariant value = d->theme->getSetting(QByteArrayLiteral("DTK/SizeMode"));
    return value.toInt();
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
    FETCH_PROPERTY("Qt/ScrollBarPolicy", scrollBarPolicy)

    return qvariant_cast<int>(value);
}

void DPlatformTheme::setCursorBlinkTime(int cursorBlinkTime)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Net/CursorBlinkTime", cursorBlinkTime);
}

void DPlatformTheme::setCursorBlinkTimeout(int cursorBlinkTimeout)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Net/CursorBlinkTimeout", cursorBlinkTimeout);
}

void DPlatformTheme::setCursorBlink(bool cursorBlink)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Net/CursorBlink", cursorBlink);
}

void DPlatformTheme::setDoubleClickDistance(int doubleClickDistance)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Net/DoubleClickDistance", doubleClickDistance);
}

void DPlatformTheme::setDoubleClickTime(int doubleClickTime)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Net/DoubleClickTime", doubleClickTime);
}

void DPlatformTheme::setDndDragThreshold(int dndDragThreshold)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Net/DndDragThreshold", dndDragThreshold);
}

void DPlatformTheme::setThemeName(const QByteArray &themeName)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Net/ThemeName", themeName);
}

void DPlatformTheme::setIconThemeName(const QByteArray &iconThemeName)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Net/IconThemeName", iconThemeName);
}

void DPlatformTheme::setSoundThemeName(const QByteArray &soundThemeName)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Net/SoundThemeName", soundThemeName);
}

void DPlatformTheme::setFontName(const QByteArray &fontName)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Qt/FontName", fontName);
}

void DPlatformTheme::setMonoFontName(const QByteArray &monoFontName)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Qt/MonoFontName", monoFontName);
}

void DPlatformTheme::setFontPointSize(qreal fontPointSize)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Qt/FontPointSize", fontPointSize);
}

void DPlatformTheme::setGtkFontName(const QByteArray &fontName)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Gtk/FontName", fontName);
}

void DPlatformTheme::setActiveColor(const QColor activeColor)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("Qt/ActiveColor", activeColor);
}

#define SET_COLOR(Role) setSetting(QByteArrayLiteral(#Role), Role)
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
void DPlatformTheme::setWindow(const QColor &window)
{
    SET_COLOR(window);
}

void DPlatformTheme::setWindowText(const QColor &windowText)
{
    SET_COLOR(windowText);
}

void DPlatformTheme::setBase(const QColor &base)
{
    SET_COLOR(base);
}

void DPlatformTheme::setAlternateBase(const QColor &alternateBase)
{
    SET_COLOR(alternateBase);
}

void DPlatformTheme::setToolTipBase(const QColor &toolTipBase)
{
    SET_COLOR(toolTipBase);
}

void DPlatformTheme::setToolTipText(const QColor &toolTipText)
{
    SET_COLOR(toolTipText);
}

void DPlatformTheme::setText(const QColor &text)
{
    SET_COLOR(text);
}

void DPlatformTheme::setButton(const QColor &button)
{
    SET_COLOR(button);
}

void DPlatformTheme::setButtonText(const QColor &buttonText)
{
    SET_COLOR(buttonText);
}

void DPlatformTheme::setBrightText(const QColor &brightText)
{
    SET_COLOR(brightText);
}

void DPlatformTheme::setLight(const QColor &light)
{
    SET_COLOR(light);
}

void DPlatformTheme::setMidlight(const QColor &midlight)
{
    SET_COLOR(midlight);
}

void DPlatformTheme::setDark(const QColor &dark)
{
    SET_COLOR(dark);
}

void DPlatformTheme::setMid(const QColor &mid)
{
    SET_COLOR(mid);
}

void DPlatformTheme::setShadow(const QColor &shadow)
{
    SET_COLOR(shadow);
}

void DPlatformTheme::setHighlight(const QColor &highlight)
{
    SET_COLOR(highlight);
}

void DPlatformTheme::setHighlightedText(const QColor &highlightText)
{
    SET_COLOR(highlightText);
}

void DPlatformTheme::setLink(const QColor &link)
{
    SET_COLOR(link);
}

void DPlatformTheme::setLinkVisited(const QColor &linkVisited)
{
    SET_COLOR(linkVisited);
}

void DPlatformTheme::setItemBackground(const QColor &itemBackground)
{
    SET_COLOR(itemBackground);
}

void DPlatformTheme::setTextTitle(const QColor &textTitle)
{
    SET_COLOR(textTitle);
}

void DPlatformTheme::setTextTips(const QColor &textTips)
{
    SET_COLOR(textTips);
}

void DPlatformTheme::setTextWarning(const QColor &textWarning)
{
    SET_COLOR(textWarning);
}

void DPlatformTheme::setTextLively(const QColor &textLively)
{
    SET_COLOR(textLively);
}

void DPlatformTheme::setLightLively(const QColor &lightLively)
{
    SET_COLOR(lightLively);
}

void DPlatformTheme::setDarkLively(const QColor &darkLively)
{
    SET_COLOR(darkLively);
}

void DPlatformTheme::setFrameBorder(const QColor &frameBorder)
{
    SET_COLOR(frameBorder);
}
#endif

void DPlatformTheme::setDotsPerInch(const QString &screenName, int dpi)
{
    D_D(DPlatformTheme);

    if (screenName.isEmpty()) {
        d->theme->setSetting("Xft/DPI", dpi);
    } else {
        d->theme->setSetting("Qt/DPI/" + screenName.toLocal8Bit(), dpi);
    }
}

void DPlatformTheme::setWindowRadius(int windowRadius)
{
    D_D(DPlatformTheme);

    d->theme->setSetting("DTK/WindowRadius", windowRadius);
}

DGUI_END_NAMESPACE

#include "moc_dplatformtheme.cpp"
