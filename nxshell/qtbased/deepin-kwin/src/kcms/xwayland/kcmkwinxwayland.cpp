/*
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcmkwinxwayland.h"

#include <KAboutData>
#include <KApplicationTrader>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QKeySequence>

#include <kwinxwaylanddata.h>

K_PLUGIN_FACTORY_WITH_JSON(KcmXwaylandFactory, "kcm_kwinxwayland.json", registerPlugin<KcmXwayland>(); registerPlugin<KWinXwaylandData>();)

KcmXwayland::KcmXwayland(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent)
    , m_data(new KWinXwaylandData(this))
    , m_settings(new KWinXwaylandSettings(m_data))
{
    registerSettings(m_settings);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterType<KWinXwaylandSettings>();
#else
    qmlRegisterAnonymousType<KWinXwaylandSettings>("org.kde.kwin.kwinxwaylandsettings", 1);
#endif

    setAboutData(new KAboutData(QStringLiteral("deepin-kcm_kwinxwayland"),
                                i18n("Legacy X11 App Support"),
                                QStringLiteral("1.0"),
                                i18n("Allow legacy X11 apps to read keystrokes typed in other apps"),
                                KAboutLicense::GPL));
}

KcmXwayland::~KcmXwayland() = default;

#include "kcmkwinxwayland.moc"
