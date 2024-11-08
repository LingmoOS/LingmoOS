#include "platformtheme.h"
#include "x11integration.h"
#include "qdbusmenubar_p.h"

#include <QApplication>
#include <QFont>
#include <QPalette>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QLibrary>
#include <QStyleFactory>
#include <QtQuickControls2/QQuickStyle>

// Qt Private
#include <private/qicon_p.h>
#include <private/qiconloader_p.h>
#include <private/qwindow_p.h>
#include <private/qguiapplication_p.h>

// Qt DBus
#include <QDBusConnection>
#include <QDBusInterface>

#include <KWindowSystem>

static const QByteArray s_x11AppMenuServiceNamePropertyName = QByteArrayLiteral("_KDE_NET_WM_APPMENU_SERVICE_NAME");
static const QByteArray s_x11AppMenuObjectPathPropertyName = QByteArrayLiteral("_KDE_NET_WM_APPMENU_OBJECT_PATH");

static bool checkDBusGlobalMenuAvailable()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QString registrarService = QStringLiteral("com.canonical.AppMenu.Registrar");
    return connection.interface()->isServiceRegistered(registrarService);
}

static bool isDBusGlobalMenuAvailable()
{
    static bool dbusGlobalMenuAvailable = checkDBusGlobalMenuAvailable();
    return dbusGlobalMenuAvailable;
}

extern void updateXdgIconSystemTheme();

void onDarkModeChanged()
{
    if (qApp->applicationName() == "systemsettings"
                || qApp->applicationName().contains("plasma")
                || qApp->applicationName().contains("QtCreator")) {
        return;
    }

    QStyle *style = QStyleFactory::create("lingmo");
    if (style) {
        qApp->setStyle(style);
    }
}

PlatformTheme::PlatformTheme()
    : m_hints(new HintsSettings)
{
    // qApp->setProperty("_hints_settings_object", (quintptr)m_hints);

    if (KWindowSystem::isPlatformX11()) {
        m_x11Integration.reset(new X11Integration());
        m_x11Integration->init();
    }

    connect(m_hints, &HintsSettings::systemFontChanged, this, &PlatformTheme::onFontChanged);
    connect(m_hints, &HintsSettings::systemFontPointSizeChanged, this, &PlatformTheme::onFontChanged);
    connect(m_hints, &HintsSettings::iconThemeChanged, this, &PlatformTheme::onIconThemeChanged);
    connect(m_hints, &HintsSettings::darkModeChanged, &onDarkModeChanged);

    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, false);
    setQtQuickControlsTheme();
}

PlatformTheme::~PlatformTheme()
{
}

QVariant PlatformTheme::themeHint(QPlatformTheme::ThemeHint hintType) const
{
    QVariant hint = m_hints->hint(hintType);
    if (hint.isValid()) {
        return hint;
    } else {
        return QPlatformTheme::themeHint(hintType);
    }
}

const QFont* PlatformTheme::font(Font type) const
{
    switch (type) {
        case SystemFont:
        case MessageBoxFont:
        case LabelFont:
        case TipLabelFont:
        case StatusBarFont:
        case PushButtonFont:
        case ItemViewFont:
        case ListViewFont:
        case HeaderViewFont:
        case ListBoxFont:
        case ComboMenuItemFont:
        case ComboLineEditFont: {
            const QString &fontName = m_hints->systemFont();
            qreal fontSize = m_hints->systemFontPointSize();
            static QFont font = QFont(QString());
            font.setFamily(fontName);
            font.setPointSizeF(fontSize);
            return &font;
        }
        case FixedFont: {
            const QString &fontName = m_hints->systemFixedFont();
            qreal fontSize = m_hints->systemFontPointSize();
            static QFont font = QFont(QString());
            font.setFamily(fontName);
            font.setPointSizeF(fontSize);
            return &font;
        }
        default: {
            const QString &fontName = m_hints->systemFont();
            qreal fontSize = m_hints->systemFontPointSize();
            static QFont font = QFont(QString());
            font.setFamily(fontName);
            font.setPointSizeF(fontSize);
            return &font;
        }
    }

    return QPlatformTheme::font(type);
}

QPlatformMenuBar *PlatformTheme::createPlatformMenuBar() const
{
    if (isDBusGlobalMenuAvailable()) {
        auto *menu = new QDBusMenuBar();

        QObject::connect(menu, &QDBusMenuBar::windowChanged, menu, [this, menu](QWindow *newWindow, QWindow *oldWindow) {
            const QString &serviceName = QDBusConnection::sessionBus().baseService();
            const QString &objectPath = menu->objectPath();

            if (m_x11Integration) {
                if (oldWindow) {
                    m_x11Integration->setWindowProperty(oldWindow, s_x11AppMenuServiceNamePropertyName, {});
                    m_x11Integration->setWindowProperty(oldWindow, s_x11AppMenuObjectPathPropertyName, {});
                }

                if (newWindow) {
                    m_x11Integration->setWindowProperty(newWindow, s_x11AppMenuServiceNamePropertyName, serviceName.toUtf8());
                    m_x11Integration->setWindowProperty(newWindow, s_x11AppMenuObjectPathPropertyName, objectPath.toUtf8());
                }
            }

//             if (m_kwaylandIntegration) {
//                 if (oldWindow) {
//                     m_kwaylandIntegration->setAppMenu(oldWindow, QString(), QString());
//                 }
//
//                 if (newWindow) {
//                     m_kwaylandIntegration->setAppMenu(newWindow, serviceName, objectPath);
//                 }
//             }
        });

        return menu;
    }

    return nullptr;
}

void PlatformTheme::onFontChanged()
{
    QFont font;
    font.setFamily(m_hints->systemFont());
    font.setPointSizeF(m_hints->systemFontPointSize());

    // Change font
    if (qobject_cast<QApplication *>(QCoreApplication::instance()))
        QApplication::setFont(font);
    else if (qobject_cast<QGuiApplication *>(QCoreApplication::instance()))
        QGuiApplication::setFont(font);
}

void PlatformTheme::onIconThemeChanged()
{
    QIconLoader::instance()->updateSystemTheme();
    updateXdgIconSystemTheme();

    QEvent update(QEvent::UpdateRequest);
    for (QWindow *window : qGuiApp->allWindows()) {
        if (window->type() == Qt::Desktop)
            continue;

        qApp->sendEvent(window, &update);
    }
}

void PlatformTheme::setQtQuickControlsTheme()
{
    //if the user has explicitly set something else, don't meddle
    if (!QQuickStyle::name().isEmpty()) {
        return;
    }

    if (qApp->applicationName() == "systemsettings"
            || qApp->applicationName().contains("plasma")) {
        QQuickStyle::setStyle("Plasma");
        QStyle *style = QStyleFactory::create("Breeze");
        qApp->setStyle(style);
        return;
    }

    QQuickStyle::setStyle(QLatin1String("lingmo-style"));
}
