/*
 * Qt5-LINGMO's Library
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <QVariant>
#include <QStandardPaths>
#include "qt5-lingmo-platform-theme.h"
#include "settings/lingmo-style-settings.h"
#include "effects/highlight-effect.h"

#include <QFontDatabase>
#include <QApplication>
#include <QTimer>

#include <QPluginLoader>
#include <QIconEnginePlugin>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QFileInfo>
#include <QIcon>
#endif

#include <QApplication>
#include <QWidget>

#include <QDebug>
#include <private/qgenericunixthemes_p.h>
#include <private/qdbustrayicon_p.h>

#include <QQuickStyle>

#include "widget/messagebox/message-box.h"
//#include "widget/filedialog/kyfiledialog.h"
#include "platform-theme-fontdata.h"
#include "../qt5-lingmo-filedialog/filedialoginterface.h"

#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
static bool isDBusTrayAvailable() {
    static bool dbusTrayAvailable = false;
    static bool dbusTrayAvailableKnown = false;
    if (!dbusTrayAvailableKnown) {
        QDBusMenuConnection conn;
        if (conn.isStatusNotifierHostRegistered())
            dbusTrayAvailable = true;
        dbusTrayAvailableKnown = true;
    }
    return dbusTrayAvailable;
}
#endif

void Qt5LINGMOPlatformTheme::slotChangeStyle(const QString& key)
{
    auto settings = LINGMOStyleSettings::globalInstance();
    if (key == "iconThemeName" || key == "icon-theme-name") {
        QString icontheme = settings->get("icon-theme-name").toString();

        QIcon::setThemeName(icontheme);

        QIcon icon = qApp->windowIcon();
        qApp->setWindowIcon(QIcon::fromTheme(icon.name()));
        // update all widgets for repaint new themed icons.
        for (auto widget : QApplication::allWidgets()) {
            widget->update();
        }
    }

    if (key == "systemFont" || key == "system-font") {
        //Skip QGuiApplication avoid it crash when we setfont
        auto *app = qobject_cast<QApplication *>(qApp);
        if(app == nullptr)
            return;

        QString font = settings->get("system-font").toString();

        QFontDatabase db;
        int id = 0;
        if (!db.families().contains(font)) {
            PlatformThemeFontData fontData;
            QMap<QString, QString> fontMap = fontData.getAllFontInformation();
            if(fontMap.contains(font)){
                auto iter = fontMap.find(font);
                id = QFontDatabase::addApplicationFont(iter.value());
            }
        }

        QFontDatabase newDb;
        if (newDb.families().contains(font)) {
            QFont oldFont = QApplication::font();
            m_system_font.setFamily(font);
            m_fixed_font.setFamily(font);
            oldFont.setFamily(font);
            QApplication::setFont(oldFont);
        }
    }
    if (key == "systemFontSize" || key == "system-font-size") {
        //Skip QGuiApplication avoid it crash when we setfont
        auto *app = qobject_cast<QApplication *>(qApp);
        if(app == nullptr)
            return;

        if (qApp->property("noChangeSystemFontSize").isValid() && qApp->property("noChangeSystemFontSize").toBool())
            return;
        double fontSize = settings->get("system-font-size").toString().toDouble();
        if (fontSize > 0) {
            QFont oldFont = QApplication::font();
            m_system_font.setPointSize(fontSize);
            m_fixed_font.setPointSize(fontSize*1.2);
            oldFont.setPointSizeF(fontSize);
            QApplication::setFont(oldFont);
        }
    }
}

Qt5LINGMOPlatformTheme::Qt5LINGMOPlatformTheme(const QStringList &args)
{
    //FIXME:
    Q_UNUSED(args)
    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        auto settings = LINGMOStyleSettings::globalInstance();

        //set font
        auto fontName = settings->get("systemFont").toString();
        auto fontSize = settings->get("systemFontSize").toString().toDouble();
        if (qApp->property("noChangeSystemFontSize").isValid() && qApp->property("noChangeSystemFontSize").toBool())
            fontSize = 11;
        m_system_font.setFamily(fontName);
        m_system_font.setPointSizeF(fontSize);

        m_fixed_font.setFamily(fontName);
        m_fixed_font.setPointSizeF(fontSize*1.2);

        /*!
         * \bug
         * if we set app font, qwizard title's font will
         * become very small. I handle the wizard title
         * in ProxyStyle::polish().
         */
        //Skip QGuiApplication avoid it crash when we setfont
        auto *app = qobject_cast<QApplication *>(qApp);
        if(app != nullptr) {
            QApplication::setFont(m_system_font);
        }

        if (app->applicationName().toLower().contains(QLatin1String("kwin"))) {
            QDBusConnection::sessionBus().connect(QString(),
                                                  QStringLiteral("/LINGMOPlatformTheme"),
                                                  QStringLiteral("org.lingmo.LINGMOPlatformTheme"),
                                                  QStringLiteral("refreshFonts"),
                                                  this,
                                                  SLOT(slotChangeStyle(QString)));
        }

        connect(settings, &QGSettings::changed, this, &Qt5LINGMOPlatformTheme::slotChangeStyle);
    }

//    // add qqc2 style
//    if (QFile::exists(QString("%1/kf5/kirigami/org.lingmo.style.so").arg(QT_PLUGIN_INSTALL_DIRS))) {
//        QQuickStyle::setStyle("org.lingmo.style");
//    }
    if(qApp->inherits("QApplication") == true && qAppName() != "cura") {
        QQuickStyle::setStyle("org.lingmo.style");
    }

    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        QGuiApplication::setLayoutDirection(Qt::RightToLeft);
    } else {
        QGuiApplication::setLayoutDirection(Qt::LeftToRight);
    }
}

Qt5LINGMOPlatformTheme::~Qt5LINGMOPlatformTheme()
{
}

const QPalette *Qt5LINGMOPlatformTheme::palette(Palette type) const
{
    //FIXME:
    return QPlatformTheme::palette(type);
}

const QFont *Qt5LINGMOPlatformTheme::font(Font type) const
{
    //FIXME:
    if (type == FixedFont)
        return &m_fixed_font;
    return &m_system_font;
    switch (type) {
    case SystemFont:
        return &m_system_font;
    case TitleBarFont:
    case FixedFont:
    case GroupBoxTitleFont:
        return &m_fixed_font;
    default:
        return &m_system_font;
    }
    return QPlatformTheme::font(type);
}

QVariant Qt5LINGMOPlatformTheme::themeHint(ThemeHint hint) const
{
    if(qAppName() == "cura")
        return QPlatformTheme::themeHint(hint);

    switch (hint) {
    case QPlatformTheme::StyleNames:{
//        qDebug() << "Qt5LINGMOPlatformTheme....";
//        if (LINGMOStyleSettings::isSchemaInstalled("org.lingmo.style")) {
//            if (auto settings = LINGMOStyleSettings::globalInstance()) {
//                QString themeName = settings->get("widget-theme-name").toString();
//                if(themeName == "classical")
//                return QStringList()<< "lingmo-default";
//            }
//        }
        return QStringList()<< "lingmo";
    }

    case QPlatformTheme::SystemIconThemeName: {
        if (LINGMOStyleSettings::isSchemaInstalled("org.lingmo.style")) {
            if (auto settings = LINGMOStyleSettings::globalInstance()) {
                QString icontheme = settings->get("icon-theme-name").toString();

                return QStringList()<<icontheme;
            }
        }
        return "hicolor";
    }

    case QPlatformTheme::SystemIconFallbackThemeName:
        return "hicolor";
    case QPlatformTheme::IconThemeSearchPaths:
        return xdgIconThemePaths();
//    case WheelScrollLines:
//    {
//        return QVariant(1);
//    }break;
    default:
        break;
    }
    return QPlatformTheme::themeHint(hint);
}

QStringList Qt5LINGMOPlatformTheme::xdgIconThemePaths() const {
    QStringList paths;
    // Add home directory first in search path
    const QFileInfo homeIconDir(QDir::homePath() + QLatin1String("/.icons"));
    if (homeIconDir.isDir())
        paths.prepend(homeIconDir.absoluteFilePath());

    paths.append(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                           QStringLiteral("icons"),
                                           QStandardPaths::LocateDirectory));
    return paths;
}

QIconEngine *Qt5LINGMOPlatformTheme::createIconEngine(const QString &iconName) const
{
//    QPluginLoader l(XDG_ICON_ENGINE_PATH);
//    if (l.instance()) {
//        auto p = dynamic_cast<QIconEnginePlugin *>(l.instance());
//        auto engine = p->create();
//        qDebug()<<"use my engine";
//        return engine;
//    } else {
//        qDebug()<<"use common engine";
//        return QPlatformTheme::createIconEngine(iconName);
//    }

//    //return new XdgIconLoaderEngine(iconName);

    return QPlatformTheme::createIconEngine(iconName);
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
bool Qt5LINGMOPlatformTheme::usePlatformNativeDialog(DialogType type) const
{
    switch (type) {
    case QPlatformTheme::FileDialog:
        return true;
    case QPlatformTheme::FontDialog:
    case QPlatformTheme::ColorDialog:
        return false;
    case QPlatformTheme::MessageDialog:
//        if (qAppName() == "lingmo-control-center")
//            return false;
        return true;
    default:
        break;
    }

    return false;
}

QPlatformDialogHelper *Qt5LINGMOPlatformTheme::createPlatformDialogHelper(DialogType type) const
{
    QObject* plugin = nullptr;
    const char* c = LINGMOPLATFORMTHEME_DIR;
    QString path = QString::fromUtf8(c);

    //QString path = LINGMOPLATFORMTHEME_DIR;
    if (QFile::exists(QString("%1/platformthemes/libqt5-lingmo-filedialog.so").arg(path))) {
        QPluginLoader loader(QString("%1/platformthemes/libqt5-lingmo-filedialog.so").arg(path));
        plugin = loader.instance();
    }

    switch (type) {
    case QPlatformTheme::FileDialog:
        if (plugin) {
            return qobject_cast<FileDialogInterface *>(plugin)->create();
        }
                //return new KyFileDialogHelper;
    case QPlatformTheme::FontDialog:
    case QPlatformTheme::ColorDialog:
        return QPlatformTheme::createPlatformDialogHelper(type);
    case QPlatformTheme::MessageDialog:
        return new MessageBoxHelper;
    default:
        break;
    }

    return nullptr;
}
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
QIcon Qt5LINGMOPlatformTheme::fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions iconOptions) const
{
    //FIXME:
    return QPlatformTheme::fileIcon(fileInfo, iconOptions);
}
#endif

#ifdef GLOBAL_MENU
QPlatformMenuBar *Qt5LINGMOPlatformTheme::createPlatformMenuBar() const
{
    return QPlatformTheme::createPlatformMenuBar();
}
#endif

#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
QPlatformSystemTrayIcon *Qt5LINGMOPlatformTheme::createPlatformSystemTrayIcon() const
{
    if (isDBusTrayAvailable())
        return new QDBusTrayIcon();
    return nullptr;
}

#endif
