// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dxcbplatforminterface.h"
#include "dxcbplatforminterface_p.h"

#include <DNativeSettings>
#include <QPlatformSurfaceEvent>
#include <QGuiApplication>
#include <QMetaObject>
#include <QMetaProperty>

#include "dplatformtheme.h"
#include "dguiapplicationhelper.h"
#include <qpa/qplatformwindow.h>

DGUI_BEGIN_NAMESPACE

#define DEFINE_CONST_CHAR(Name) const char _##Name[] = "_d_" #Name

#define FETCH_PROPERTY(Name, Function) \
    D_DC(DXCBPlatformInterface); \
    QVariant value = d->theme->getSetting(QByteArrayLiteral(Name)); \
    if (d->fallbackProperty && !value.isValid() && d->parent) \
        return d->parent->Function(); \

#define FETCH_PROPERTY_WITH_ARGS(Name, Function, Args) \
    D_DC(DXCBPlatformInterface); \
    QVariant value = d->theme->getSetting(Name); \
    if (d->fallbackProperty && !value.isValid() && d->parent) \
        return d->parent->Function(Args); \


DXCBPlatformInterfacePrivate::DXCBPlatformInterfacePrivate(DXCBPlatformInterface *qq)
    :DCORE_NAMESPACE::DObjectPrivate(qq)
{
}

void DXCBPlatformInterfacePrivate::_q_onThemePropertyChanged(const QByteArray &name, const QVariant &value)
{
    D_Q(DXCBPlatformInterface); 

    // 转发属性变化的信号，此信号来源可能为parent theme或“非调色板”的属性变化。
    // 使用队列的形式转发，避免多次发出同样的信号
    // q->staticMetaObject.invokeMethod(q, "propertyChanged", Qt::QueuedConnection,
    //                                  Q_ARG(const QByteArray&, name), Q_ARG(const QVariant&, value));

    if (QByteArrayLiteral("Gtk/FontName") == name) {
        Q_EMIT q->m_platformTheme->gtkFontNameChanged(value.toByteArray());
        return;
    }

    if (name.startsWith("Qt/DPI/")) {
        const QString &screen_name = QString::fromLocal8Bit(name.mid(7));

        if (!screen_name.isEmpty()) {
            bool ok = false;
            int dpi = value.toInt(&ok);

            Q_EMIT q->m_platformTheme->dotsPerInchChanged(screen_name, ok ? dpi : -1);
        }

        return;
    }

    if (QByteArrayLiteral("Xft/DPI") == name) {
        bool ok = false;
        int dpi = value.toInt(&ok);
        Q_EMIT q->m_platformTheme->dotsPerInchChanged(QString(), ok ? dpi : -1);
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
            // return;

        if (!p.notifySignal().invokeOnGadget(q->m_platformTheme, QGenericArgument(value.typeName(), value.constData())))
            qWarning() << "_q_onThemePropertyChanged() error when notify signal" << p.notifySignal().name();
    }
}


DXCBPlatformInterface::DXCBPlatformInterface(quint32 window, DPlatformTheme *platformTheme)
    : DPlatformInterface(platformTheme)
    , Core::DObject(*new DXCBPlatformInterfacePrivate(this))
{
    D_D(DXCBPlatformInterface);

    d->theme = new DNativeSettings(window, QByteArray(), platformTheme);

    connect(d->theme, SIGNAL(propertyChanged(const QByteArray &, const QVariant &)),
        this, SLOT(_q_onThemePropertyChanged(const QByteArray &, const QVariant &)));
}

int DXCBPlatformInterface::cursorBlinkTime() const
{
    FETCH_PROPERTY("Net/CursorBlinkTime", cursorBlinkTime)

    return value.toInt();
}

int DXCBPlatformInterface::cursorBlinkTimeout() const
{
    FETCH_PROPERTY("Net/CursorBlinkTimeout", cursorBlinkTimeout)

    return value.toInt();
}

bool DXCBPlatformInterface::cursorBlink() const
{
    FETCH_PROPERTY("Net/CursorBlink", cursorBlink)

    return value.toInt();
}

int DXCBPlatformInterface::doubleClickDistance() const
{
    FETCH_PROPERTY("Net/DoubleClickDistance", doubleClickDistance)

    return value.toInt();
}

int DXCBPlatformInterface::doubleClickTime() const
{
    FETCH_PROPERTY("Net/DoubleClickTime", doubleClickTime)

    return value.toInt();
}

int DXCBPlatformInterface::dndDragThreshold() const
{
    FETCH_PROPERTY("Net/DndDragThreshold", dndDragThreshold)

    return value.toInt();
}

int DXCBPlatformInterface::windowRadius() const
{
    return windowRadius(-1);
}

int DXCBPlatformInterface::windowRadius(int defaultValue) const
{
    Q_D(const DXCBPlatformInterface);

    QVariant value = d->theme->getSetting(QByteArrayLiteral("DTK/WindowRadius"));
    bool ok = false;

    if (d->fallbackProperty && !value.isValid() && d->parent)
        return d->parent->windowRadius(defaultValue);

    int radius = value.toInt(&ok);

    return ok ? radius : defaultValue;
}

QByteArray DXCBPlatformInterface::themeName() const
{
    FETCH_PROPERTY("Net/ThemeName", themeName)

    return value.toByteArray();
}

QByteArray DXCBPlatformInterface::iconThemeName() const
{
    FETCH_PROPERTY("Net/IconThemeName", iconThemeName)

    return value.toByteArray();
}

QByteArray DXCBPlatformInterface::soundThemeName() const
{
    FETCH_PROPERTY("Net/SoundThemeName", soundThemeName)

    return value.toByteArray();
}

QByteArray DXCBPlatformInterface::fontName() const
{
    FETCH_PROPERTY("Qt/FontName", fontName)

    return value.toByteArray();
}

QByteArray DXCBPlatformInterface::monoFontName() const
{
    FETCH_PROPERTY("Qt/MonoFontName", monoFontName)

    return value.toByteArray();
}

qreal DXCBPlatformInterface::fontPointSize() const
{
    FETCH_PROPERTY("Qt/FontPointSize", fontPointSize)

    return value.toDouble();
}

QByteArray DXCBPlatformInterface::gtkFontName() const
{
    FETCH_PROPERTY("Gtk/FontName", gtkFontName)

    return value.toByteArray();
}

QColor DXCBPlatformInterface::activeColor() const
{
    FETCH_PROPERTY("Qt/ActiveColor", activeColor)

    return qvariant_cast<QColor>(value);
}

QColor DXCBPlatformInterface::darkActiveColor() const
{
    FETCH_PROPERTY("Qt/DarkActiveColor", darkActiveColor)

    return qvariant_cast<QColor>(value);
}

static QColor getSetting(const QByteArray &key)
{
    qWarning() << "Not implemented, key:" << key;
    return {};
}

#define GET_COLOR(Role) qvariant_cast<QColor>(getSetting(QByteArrayLiteral(#Role)))
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
QColor DXCBPlatformInterface::window() const
{
    return GET_COLOR(window);
}

QColor DXCBPlatformInterface::windowText() const
{
    return GET_COLOR(windowText);
}

QColor DXCBPlatformInterface::base() const
{
    return GET_COLOR(base);
}

QColor DXCBPlatformInterface::alternateBase() const
{
    return GET_COLOR(alternateBase);
}

QColor DXCBPlatformInterface::toolTipBase() const
{
    return GET_COLOR(toolTipBase);
}

QColor DXCBPlatformInterface::toolTipText() const
{
    return GET_COLOR(toolTipText);
}

QColor DXCBPlatformInterface::text() const
{
    return GET_COLOR(text);
}

QColor DXCBPlatformInterface::button() const
{
    return GET_COLOR(button);
}

QColor DXCBPlatformInterface::buttonText() const
{
    return GET_COLOR(buttonText);
}

QColor DXCBPlatformInterface::brightText() const
{
    return GET_COLOR(brightText);
}

QColor DXCBPlatformInterface::light() const
{
    return GET_COLOR(light);
}

QColor DXCBPlatformInterface::midlight() const
{
    return GET_COLOR(midlight);
}

QColor DXCBPlatformInterface::dark() const
{
    return GET_COLOR(dark);
}

QColor DXCBPlatformInterface::mid() const
{
    return GET_COLOR(mid);
}

QColor DXCBPlatformInterface::shadow() const
{
    return GET_COLOR(shadow);
}

QColor DXCBPlatformInterface::highlight() const
{
    return GET_COLOR(highlight);
}

QColor DXCBPlatformInterface::highlightedText() const
{
    return GET_COLOR(highlightedText);
}

QColor DXCBPlatformInterface::link() const
{
    return GET_COLOR(link);
}

QColor DXCBPlatformInterface::linkVisited() const
{
    return GET_COLOR(linkVisited);
}

QColor DXCBPlatformInterface::itemBackground() const
{
    return GET_COLOR(itemBackground);
}

QColor DXCBPlatformInterface::textTitle() const
{
    return GET_COLOR(textTitle);
}

QColor DXCBPlatformInterface::textTips() const
{
    return GET_COLOR(textTips);
}

QColor DXCBPlatformInterface::textWarning() const
{
    return GET_COLOR(textWarning);
}

QColor DXCBPlatformInterface::textLively() const
{
    return GET_COLOR(textLively);
}

QColor DXCBPlatformInterface::lightLively() const
{
    return GET_COLOR(lightLively);
}

QColor DXCBPlatformInterface::darkLively() const
{
    return GET_COLOR(darkLively);
}

QColor DXCBPlatformInterface::frameBorder() const
{
    return GET_COLOR(frameBorder);
}
#endif

int DXCBPlatformInterface::dotsPerInch(const QString &screenName) const
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
  \property DXCBPlatformInterface::sizeMode
  \brief This property holds the sizeMode of the system's SizeMode.
 */
int DXCBPlatformInterface::sizeMode() const
{
    D_DC(DXCBPlatformInterface);
    QVariant value = d->theme->getSetting(QByteArrayLiteral("DTK/SizeMode"));
    return value.toInt();
}

/*!
  \property DXCBPlatformInterface::scrollBarPolicy
  \brief This property holds the scrollBarPolicy of the system. same as Qt::ScrollBarPolicy
  \retval 0 show as needed auto hide, default
  \retval 1 always off
  \retval 2 always on
 */
int DXCBPlatformInterface::scrollBarPolicy() const
{
    FETCH_PROPERTY("Qt/ScrollBarPolicy", scrollBarPolicy)

    return qvariant_cast<int>(value);
}

void DXCBPlatformInterface::setCursorBlinkTime(int cursorBlinkTime)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Net/CursorBlinkTime", cursorBlinkTime);
}

void DXCBPlatformInterface::setCursorBlinkTimeout(int cursorBlinkTimeout)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Net/CursorBlinkTimeout", cursorBlinkTimeout);
}

void DXCBPlatformInterface::setCursorBlink(bool cursorBlink)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Net/CursorBlink", cursorBlink);
}

void DXCBPlatformInterface::setDoubleClickDistance(int doubleClickDistance)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Net/DoubleClickDistance", doubleClickDistance);
}

void DXCBPlatformInterface::setDoubleClickTime(int doubleClickTime)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Net/DoubleClickTime", doubleClickTime);
}

void DXCBPlatformInterface::setDndDragThreshold(int dndDragThreshold)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Net/DndDragThreshold", dndDragThreshold);
}

void DXCBPlatformInterface::setThemeName(const QByteArray &themeName)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Net/ThemeName", themeName);
}

void DXCBPlatformInterface::setIconThemeName(const QByteArray &iconThemeName)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Net/IconThemeName", iconThemeName);
}

void DXCBPlatformInterface::setSoundThemeName(const QByteArray &soundThemeName)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Net/SoundThemeName", soundThemeName);
}

void DXCBPlatformInterface::setFontName(const QByteArray &fontName)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Qt/FontName", fontName);
}

void DXCBPlatformInterface::setMonoFontName(const QByteArray &monoFontName)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Qt/MonoFontName", monoFontName);
}

void DXCBPlatformInterface::setFontPointSize(qreal fontPointSize)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Qt/FontPointSize", fontPointSize);
}

void DXCBPlatformInterface::setGtkFontName(const QByteArray &fontName)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Gtk/FontName", fontName);
}

void DXCBPlatformInterface::setActiveColor(const QColor activeColor)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Qt/ActiveColor", activeColor);
}

void DXCBPlatformInterface::setDarkActiveColor(const QColor &activeColor)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("Qt/DarkActiveColor", activeColor);
}

static void setSetting(const QByteArray &key, const QColor &color)
{
    qWarning() << "Not implemented, key: " << key << "value: " << color;
}

#define SET_COLOR(Role) setSetting(QByteArrayLiteral(#Role), Role)
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
void DXCBPlatformInterface::setWindow(const QColor &window)
{
    SET_COLOR(window);
}

void DXCBPlatformInterface::setWindowText(const QColor &windowText)
{
    SET_COLOR(windowText);
}

void DXCBPlatformInterface::setBase(const QColor &base)
{
    SET_COLOR(base);
}

void DXCBPlatformInterface::setAlternateBase(const QColor &alternateBase)
{
    SET_COLOR(alternateBase);
}

void DXCBPlatformInterface::setToolTipBase(const QColor &toolTipBase)
{
    SET_COLOR(toolTipBase);
}

void DXCBPlatformInterface::setToolTipText(const QColor &toolTipText)
{
    SET_COLOR(toolTipText);
}

void DXCBPlatformInterface::setText(const QColor &text)
{
    SET_COLOR(text);
}

void DXCBPlatformInterface::setButton(const QColor &button)
{
    SET_COLOR(button);
}

void DXCBPlatformInterface::setButtonText(const QColor &buttonText)
{
    SET_COLOR(buttonText);
}

void DXCBPlatformInterface::setBrightText(const QColor &brightText)
{
    SET_COLOR(brightText);
}

void DXCBPlatformInterface::setLight(const QColor &light)
{
    SET_COLOR(light);
}

void DXCBPlatformInterface::setMidlight(const QColor &midlight)
{
    SET_COLOR(midlight);
}

void DXCBPlatformInterface::setDark(const QColor &dark)
{
    SET_COLOR(dark);
}

void DXCBPlatformInterface::setMid(const QColor &mid)
{
    SET_COLOR(mid);
}

void DXCBPlatformInterface::setShadow(const QColor &shadow)
{
    SET_COLOR(shadow);
}

void DXCBPlatformInterface::setHighlight(const QColor &highlight)
{
    SET_COLOR(highlight);
}

void DXCBPlatformInterface::setHighlightedText(const QColor &highlightText)
{
    SET_COLOR(highlightText);
}

void DXCBPlatformInterface::setLink(const QColor &link)
{
    SET_COLOR(link);
}

void DXCBPlatformInterface::setLinkVisited(const QColor &linkVisited)
{
    SET_COLOR(linkVisited);
}

void DXCBPlatformInterface::setItemBackground(const QColor &itemBackground)
{
    SET_COLOR(itemBackground);
}

void DXCBPlatformInterface::setTextTitle(const QColor &textTitle)
{
    SET_COLOR(textTitle);
}

void DXCBPlatformInterface::setTextTips(const QColor &textTips)
{
    SET_COLOR(textTips);
}

void DXCBPlatformInterface::setTextWarning(const QColor &textWarning)
{
    SET_COLOR(textWarning);
}

void DXCBPlatformInterface::setTextLively(const QColor &textLively)
{
    SET_COLOR(textLively);
}

void DXCBPlatformInterface::setLightLively(const QColor &lightLively)
{
    SET_COLOR(lightLively);
}

void DXCBPlatformInterface::setDarkLively(const QColor &darkLively)
{
    SET_COLOR(darkLively);
}

void DXCBPlatformInterface::setFrameBorder(const QColor &frameBorder)
{
    SET_COLOR(frameBorder);
}
#endif

void DXCBPlatformInterface::setDotsPerInch(const QString &screenName, int dpi)
{
    D_D(DXCBPlatformInterface);

    if (screenName.isEmpty()) {
        d->theme->setSetting("Xft/DPI", dpi);
    } else {
        d->theme->setSetting("Qt/DPI/" + screenName.toLocal8Bit(), dpi);
    }
}

void DXCBPlatformInterface::setWindowRadius(int windowRadius)
{
    D_D(DXCBPlatformInterface);

    d->theme->setSetting("DTK/WindowRadius", windowRadius);
}

DGUI_END_NAMESPACE

#include "moc_dxcbplatforminterface.cpp"
