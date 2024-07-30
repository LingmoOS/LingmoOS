/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lingmodesktoptheme.h"
#include <KColorScheme>
#include <KColorUtils>
#include <KConfigGroup>
#include <QDebug>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QScopeGuard>

#ifndef Q_OS_ANDROID
#include <QDBusConnection>

#include <KIconColors>
#include <KIconLoader>
#endif

class StyleSingleton : public QObject
{
    Q_OBJECT

public:
    struct Colors {
        QPalette palette;
        KColorScheme selectionScheme;
        KColorScheme scheme;
    };

    explicit StyleSingleton()
        : QObject()
        , buttonScheme(QPalette::Active, KColorScheme::ColorSet::Button)
    {
        connect(qGuiApp, &QGuiApplication::paletteChanged, this, &StyleSingleton::refresh);

#ifndef Q_OS_ANDROID
        // Use DBus in order to listen for settings changes directly, as the
        // QApplication doesn't expose the font variants we're looking for,
        // namely smallFont.
        QDBusConnection::sessionBus().connect(QString(),
                                              QStringLiteral("/KDEPlatformTheme"),
                                              QStringLiteral("org.kde.KDEPlatformTheme"),
                                              QStringLiteral("refreshFonts"),
                                              this,
                                              SLOT(notifyWatchersConfigurationChange()));
#endif

        connect(qGuiApp, &QGuiApplication::fontDatabaseChanged, this, &StyleSingleton::notifyWatchersConfigurationChange);
        connect(qGuiApp, &QGuiApplication::fontChanged, this, &StyleSingleton::notifyWatchersConfigurationChange);

        /* QtTextRendering uses less memory, so use it in low power mode.
         *
         * For scale factors greater than 2, native rendering doesn't actually do much.
         * Does native rendering even work when scaleFactor >= 2?
         *
         * NativeTextRendering is still distorted sometimes with fractional scale
         * factors, despite https://bugreports.qt.io/browse/QTBUG-67007 being closed.
         * 1.5x scaling looks generally OK, but there are occasional and difficult to
         * reproduce issues with all fractional scale factors.
         */
        qreal devicePixelRatio = qGuiApp->devicePixelRatio();
#ifndef Q_OS_ANDROID
        QQuickWindow::TextRenderType defaultTextRenderType = (int(devicePixelRatio) == devicePixelRatio //
                                                                  ? QQuickWindow::NativeTextRendering
                                                                  : QQuickWindow::QtTextRendering);

        // Allow setting the text rendering type with an environment variable
        QByteArrayList validInputs = {"qttextrendering", "qtrendering", "nativetextrendering", "nativerendering"};
        QByteArray input = qgetenv("QT_QUICK_DEFAULT_TEXT_RENDER_TYPE").toLower();
        if (validInputs.contains(input)) {
            if (input == validInputs[0] || input == validInputs[1]) {
                defaultTextRenderType = QQuickWindow::QtTextRendering;
            } else {
                defaultTextRenderType = QQuickWindow::NativeTextRendering;
            }
        }

        QQuickWindow::setTextRenderType(defaultTextRenderType);
#else
        // Native rendering on android is broken, so prefer Qt rendering in
        // this case.
        QQuickWindow::setTextRenderType(QQuickWindow::QtTextRendering);
#endif

        smallFont = loadSmallFont();
    }

    QFont loadSmallFont() const
    {
        KSharedConfigPtr ptr = KSharedConfig::openConfig();
        KConfigGroup general(ptr->group("general"));

        return general.readEntry("smallestReadableFont", []() {
            auto smallFont = qApp->font();
#ifndef Q_OS_WIN
            if (smallFont.pixelSize() != -1) {
                smallFont.setPixelSize(smallFont.pixelSize() - 2);
            } else {
                smallFont.setPointSize(smallFont.pointSize() - 2);
            }
#endif
            return smallFont;
        }());
    }

    void refresh()
    {
        m_cache.clear();
        buttonScheme = KColorScheme(QPalette::Active, KColorScheme::ColorSet::Button);

        notifyWatchersPaletteChange();
    }

    Colors loadColors(LingmoUI::Platform::PlatformTheme::ColorSet cs, QPalette::ColorGroup group)
    {
        const auto key = qMakePair(cs, group);
        auto it = m_cache.constFind(key);
        if (it != m_cache.constEnd())
            return *it;

        using LingmoUI::Platform::PlatformTheme;

        KColorScheme::ColorSet set;

        switch (cs) {
        case PlatformTheme::Button:
            set = KColorScheme::ColorSet::Button;
            break;
        case PlatformTheme::Selection:
            set = KColorScheme::ColorSet::Selection;
            break;
        case PlatformTheme::Tooltip:
            set = KColorScheme::ColorSet::Tooltip;
            break;
        case PlatformTheme::View:
            set = KColorScheme::ColorSet::View;
            break;
        case PlatformTheme::Complementary:
            set = KColorScheme::ColorSet::Complementary;
            break;
        case PlatformTheme::Header:
            set = KColorScheme::ColorSet::Header;
            break;
        case PlatformTheme::Window:
        default:
            set = KColorScheme::ColorSet::Window;
        }

        // HACK/FIXME: Working around the fact that KColorScheme changes the selection background color when inactive by default.
        // Yes, this is horrible.
        QPalette::ColorGroup selectionGroup = group == QPalette::Inactive ? QPalette::Active : group;
        Colors ret = {{}, KColorScheme(selectionGroup, KColorScheme::ColorSet::Selection), KColorScheme(group, set)};

        QPalette pal;
        for (auto state : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
            pal.setBrush(state, QPalette::WindowText, ret.scheme.foreground());
            pal.setBrush(state, QPalette::Window, ret.scheme.background());
            pal.setBrush(state, QPalette::Base, ret.scheme.background());
            pal.setBrush(state, QPalette::Text, ret.scheme.foreground());
            pal.setBrush(state, QPalette::Button, ret.scheme.background());
            pal.setBrush(state, QPalette::ButtonText, ret.scheme.foreground());
            pal.setBrush(state, QPalette::Highlight, ret.selectionScheme.background());
            pal.setBrush(state, QPalette::HighlightedText, ret.selectionScheme.foreground());
            pal.setBrush(state, QPalette::ToolTipBase, ret.scheme.background());
            pal.setBrush(state, QPalette::ToolTipText, ret.scheme.foreground());

            pal.setColor(state, QPalette::Light, ret.scheme.shade(KColorScheme::LightShade));
            pal.setColor(state, QPalette::Midlight, ret.scheme.shade(KColorScheme::MidlightShade));
            pal.setColor(state, QPalette::Mid, ret.scheme.shade(KColorScheme::MidShade));
            pal.setColor(state, QPalette::Dark, ret.scheme.shade(KColorScheme::DarkShade));
            pal.setColor(state, QPalette::Shadow, QColor(0, 0, 0, 51 /* 20% */)); // ret.scheme.shade(KColorScheme::ShadowShade));

            pal.setBrush(state, QPalette::AlternateBase, ret.scheme.background(KColorScheme::AlternateBackground));
            pal.setBrush(state, QPalette::Link, ret.scheme.foreground(KColorScheme::LinkText));
            pal.setBrush(state, QPalette::LinkVisited, ret.scheme.foreground(KColorScheme::VisitedText));

            pal.setBrush(state, QPalette::PlaceholderText, ret.scheme.foreground(KColorScheme::InactiveText));
            pal.setBrush(state,
                         QPalette::BrightText,
                         KColorUtils::hcyColor(KColorUtils::hue(pal.buttonText().color()),
                                               KColorUtils::chroma(pal.buttonText().color()),
                                               1 - KColorUtils::luma(pal.buttonText().color())));
        }
        ret.palette = pal;
        m_cache.insert(key, ret);
        return ret;
    }

    void notifyWatchersPaletteChange()
    {
        for (auto watcher : std::as_const(watchers)) {
            watcher->syncColors();
        }
    }

    Q_SLOT void notifyWatchersConfigurationChange()
    {
        smallFont = loadSmallFont();
        for (auto watcher : std::as_const(watchers)) {
            watcher->setSmallFont(smallFont);
            watcher->setDefaultFont(qApp->font());
        }
    }

    KColorScheme buttonScheme;
    QFont smallFont;

    QList<LingmoDesktopTheme *> watchers;

private:
    QHash<QPair<LingmoUI::Platform::PlatformTheme::ColorSet, QPalette::ColorGroup>, Colors> m_cache;
};
Q_GLOBAL_STATIC(StyleSingleton, s_style)

LingmoDesktopTheme::LingmoDesktopTheme(QObject *parent)
    : PlatformTheme(parent)
{
    // We don't use KIconLoader on Android so we don't support recoloring there
#ifndef Q_OS_ANDROID
    setSupportsIconColoring(true);
#endif

    auto parentItem = qobject_cast<QQuickItem *>(parent);
    if (parentItem) {
        connect(parentItem, &QQuickItem::enabledChanged, this, &LingmoDesktopTheme::syncColors);
        connect(parentItem, &QQuickItem::visibleChanged, this, &LingmoDesktopTheme::syncColors);
        connect(parentItem, &QQuickItem::windowChanged, this, &LingmoDesktopTheme::syncWindow);
    }

    s_style->watchers.append(this);

    setDefaultFont(qGuiApp->font());
    setSmallFont(s_style->smallFont);

    syncWindow();
    syncColors();
}

LingmoDesktopTheme::~LingmoDesktopTheme()
{
    s_style->watchers.removeOne(this);
}

void LingmoDesktopTheme::syncWindow()
{
    if (m_window) {
        disconnect(m_window.data(), &QWindow::activeChanged, this, &LingmoDesktopTheme::syncColors);
    }

    QWindow *window = nullptr;

    auto parentItem = qobject_cast<QQuickItem *>(parent());
    if (parentItem) {
        QQuickWindow *qw = parentItem->window();

        window = QQuickRenderControl::renderWindowFor(qw);
        if (!window) {
            window = qw;
        }
        if (qw) {
            connect(qw, &QQuickWindow::sceneGraphInitialized, this, &LingmoDesktopTheme::syncWindow);
        }
    }
    m_window = window;

    if (window) {
        connect(m_window.data(), &QWindow::activeChanged, this, &LingmoDesktopTheme::syncColors);
        syncColors();
    }
}

QIcon LingmoDesktopTheme::iconFromTheme(const QString &name, const QColor &customColor)
{
#ifndef Q_OS_ANDROID
    if (customColor != Qt::transparent) {
        KIconColors colors;
        colors.setText(customColor);
        return KDE::icon(name, colors);
    } else {
        return KDE::icon(name);
    }

#else
    // On Android we don't want to use the KIconThemes-based loader since that appears to be broken
    return QIcon::fromTheme(name);
#endif
}

void LingmoDesktopTheme::syncColors()
{
    QPalette::ColorGroup group = (QPalette::ColorGroup)colorGroup();
    auto parentItem = qobject_cast<QQuickItem *>(parent());
    if (parentItem) {
        if (!parentItem->isEnabled()) {
            group = QPalette::Disabled;
        } else if (m_window && !m_window->isActive() && m_window->isExposed()) {
            // Why also checking the window is exposed?
            // in the case of QQuickWidget the window() will never be active
            // and the widgets will always have the inactive palette.
            // better to always show it active than always show it inactive
            group = QPalette::Inactive;
        }
    }

    const auto colors = s_style->loadColors(colorSet(), group);

    // foreground
    setTextColor(colors.scheme.foreground(KColorScheme::NormalText).color());
    setDisabledTextColor(colors.scheme.foreground(KColorScheme::InactiveText).color());
    setHighlightedTextColor(colors.selectionScheme.foreground(KColorScheme::NormalText).color());
    setActiveTextColor(colors.scheme.foreground(KColorScheme::ActiveText).color());
    setLinkColor(colors.scheme.foreground(KColorScheme::LinkText).color());
    setVisitedLinkColor(colors.scheme.foreground(KColorScheme::VisitedText).color());
    setNegativeTextColor(colors.scheme.foreground(KColorScheme::NegativeText).color());
    setNeutralTextColor(colors.scheme.foreground(KColorScheme::NeutralText).color());
    setPositiveTextColor(colors.scheme.foreground(KColorScheme::PositiveText).color());

    // background
    setHighlightColor(colors.selectionScheme.background(KColorScheme::NormalBackground).color());
    setBackgroundColor(colors.scheme.background(KColorScheme::NormalBackground).color());

    // HACK: It's awful, but people sometimes complain about their color scheme not working well with the theme.
    // This is because I'm using colors that weren't used before and lots of themes have bad colors for previously unused colors.
    QColor alternateBackgroundOriginalColor = colors.scheme.background(KColorScheme::AlternateBackground).color();
    // #bdc3c7 is the old default for the Ocean color scheme.
    // #4d4d4d is the old default for the Ocean Dark color scheme.
    // Most color schemes use one of these 2 colors.
    if (colorSet() == ColorSet::Button && (alternateBackgroundOriginalColor == QColor("#bdc3c7") || alternateBackgroundOriginalColor == QColor("#4d4d4d"))) {
        setAlternateBackgroundColor(KColorUtils::tint(backgroundColor(), highlightColor(), 0.4));
    } else {
        setAlternateBackgroundColor(alternateBackgroundOriginalColor);
    }
    setActiveBackgroundColor(colors.scheme.background(KColorScheme::ActiveBackground).color());
    setLinkBackgroundColor(colors.scheme.background(KColorScheme::LinkBackground).color());
    setVisitedLinkBackgroundColor(colors.scheme.background(KColorScheme::VisitedBackground).color());
    setNegativeBackgroundColor(colors.scheme.background(KColorScheme::NegativeBackground).color());
    setNeutralBackgroundColor(colors.scheme.background(KColorScheme::NeutralBackground).color());
    setPositiveBackgroundColor(colors.scheme.background(KColorScheme::PositiveBackground).color());

    // decoration
    setHoverColor(colors.scheme.decoration(KColorScheme::HoverColor).color());
    setFocusColor(colors.scheme.decoration(KColorScheme::FocusColor).color());
}

bool LingmoDesktopTheme::event(QEvent *event)
{
    if (event->type() == LingmoUI::Platform::PlatformThemeEvents::DataChangedEvent::type) {
        syncColors();
    }

    if (event->type() == LingmoUI::Platform::PlatformThemeEvents::ColorSetChangedEvent::type) {
        syncColors();
    }

    if (event->type() == LingmoUI::Platform::PlatformThemeEvents::ColorGroupChangedEvent::type) {
        syncColors();
    }

    return PlatformTheme::event(event);
}

#include "lingmodesktoptheme.moc"

#include "moc_lingmodesktoptheme.cpp"
