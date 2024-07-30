/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "statusnotifier.h"

#include <QAction>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QMenu>

#include <KLocalizedString>
#include <KNotification>
#include <KStatusNotifierItem>

#include "crashedapplication.h"
#include "drkonqi.h"
#include "statusnotifier_activationclosetimer.h"

namespace
{
QString activationMessage(StatusNotifier::Activation activation)
{
    switch (activation) {
    case StatusNotifier::Activation::NotAllowed:
        return i18nc("Notification text", "The application closed unexpectedly.");
    case StatusNotifier::Activation::Allowed:
        return i18nc("Notification text", "Please report this error to help improve this software.");
    case StatusNotifier::Activation::AlreadySubmitting:
        return i18nc("Notification text", "The application closed unexpectedly. A report is being automatically submitted.");
    }
    Q_ASSERT(false);
    return {};
}
} // namespace

StatusNotifier::StatusNotifier(QObject *parent)
    : QObject(parent)
    , m_sni(new KStatusNotifierItem(this))
{
    CrashedApplication *crashedApp = DrKonqi::crashedApplication();

    // this is used for both the SNI tooltip as well as the notification
    m_title = i18nc("Placeholder is an application name; it crashed", "%1 Closed Unexpectedly", crashedApp->name());
}

StatusNotifier::~StatusNotifier() = default;

void StatusNotifier::show()
{
    CrashedApplication *crashedApp = DrKonqi::crashedApplication();

    connect(this, &StatusNotifier::activated, this, &StatusNotifier::deleteLater);
    // The expiring the notifier doesn't necessarily quit immediately, make sure to hide the SNI by deleting it (there
    // is no way to hide an SNI).
    connect(this, &StatusNotifier::expired, m_sni, &QObject::deleteLater);

    m_sni->setTitle(m_title);
    m_sni->setIconByName(QStringLiteral("tools-report-bug"));
    m_sni->setStatus(KStatusNotifierItem::Active);
    m_sni->setCategory(KStatusNotifierItem::SystemServices);
    m_sni->setToolTip(m_sni->iconName(), m_sni->title(), i18n("Please report this error to help improve this software."));
    connect(m_sni, &KStatusNotifierItem::activateRequested, this, &StatusNotifier::activated);

    // you cannot turn off that "Do you really want to quit?" message, so we'll add our own below
    m_sni->setStandardActionsEnabled(false);

    auto *sniMenu = new QMenu();
    auto *action = new QAction(QIcon::fromTheme(QStringLiteral("tools-report-bug")), i18n("Report &Bug"), nullptr);
    connect(action, &QAction::triggered, this, &StatusNotifier::activated);
    sniMenu->addAction(action);
    sniMenu->setDefaultAction(action);

    if (canBeRestarted(crashedApp)) {
        action = new QAction(QIcon::fromTheme(QStringLiteral("system-reboot")), i18n("&Restart Application"), nullptr);
        connect(action, &QAction::triggered, crashedApp, &CrashedApplication::restart);
        // once restarted successfully, disable restart option
        connect(crashedApp, &CrashedApplication::restarted, action, [action](bool success) {
            action->setEnabled(!success);
        });
        sniMenu->addAction(action);
    }

    sniMenu->addSeparator();

    action = new QAction(QIcon::fromTheme(QStringLiteral("application-exit")), i18nc("Allows the user to hide this notifier item", "Hide"), nullptr);
    connect(action, &QAction::triggered, this, &StatusNotifier::expired);
    sniMenu->addAction(action);

    m_sni->setContextMenu(sniMenu);

    // Depending on the environmental state we may auto-activate or auto-close the SNI.
    auto timer = new ActivationCloseTimer(this);
    connect(timer, &ActivationCloseTimer::autoActivate, this, &StatusNotifier::activated);
    connect(timer, &ActivationCloseTimer::autoClose, this, &StatusNotifier::expired);
    auto dbusWatcher = new DBusServiceWatcher(timer);
    auto idleWatcher = new IdleWatcher(timer);
    timer->start(dbusWatcher, idleWatcher);
}

void StatusNotifier::notify(Activation activation)
{
    CrashedApplication *crashedApp = DrKonqi::crashedApplication();

    const auto activationAllowed = activation != Activation::NotAllowed;
    const QString title = activationAllowed ? m_title : crashedApp->name();
    const QString message = activationMessage(activation);

    KNotification *notification = KNotification::event(QStringLiteral("applicationcrash"),
                                                       title,
                                                       message,
                                                       QStringLiteral("tools-report-bug"),
                                                       KNotification::DefaultEvent | KNotification::SkipGrouping);

    if (activationAllowed) {
        if (activation == StatusNotifier::Activation::AlreadySubmitting) {
            auto details = notification->addAction(i18nc("@action:button, keep short", "Add Details"));
            connect(details, &KNotificationAction::activated, this, &StatusNotifier::sentryActivated);
        } else {
            auto action = notification->addAction(i18nc("Notification action button, keep short", "Report Bug"));
            connect(action, &KNotificationAction::activated, this, &StatusNotifier::activated);
        }
    }
    if (canBeRestarted(crashedApp)) {
        auto action = notification->addAction(i18nc("Notification action button, keep short", "Restart App"));
        connect(action, &KNotificationAction::activated, this, [crashedApp]() {
            if (canBeRestarted(crashedApp)) {
                crashedApp->restart();
            }
        });
    }

    // when the SNI disappears you won't be able to interact with the notification anymore anyway, so close it
    if (activationAllowed) {
        connect(this, &StatusNotifier::activated, notification, &KNotification::close);
        connect(this, &StatusNotifier::expired, notification, &KNotification::close);
    } else {
        // No SNI means we should quit when the notification is gone
        connect(notification, &KNotification::closed, this, &StatusNotifier::expired);
    }
}

bool StatusNotifier::notificationServiceRegistered()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.freedesktop.Notifications"));
}

bool StatusNotifier::canBeRestarted(CrashedApplication *app)
{
    return !app->hasBeenRestarted() && app->fakeExecutableBaseName() != QLatin1String("drkonqi");
}

#include "moc_statusnotifier.cpp"
