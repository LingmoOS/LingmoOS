// SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#include "kioopenwith.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QWidget>
#include <private/qgenericunixservices_p.h>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>

#include <KBuildSycocaProgressDialog>
#include <KCompletion>
#include <KIO/OpenWith>
#include <KJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

namespace
{
QString desktopPortalService()
{
    return QStringLiteral("org.freedesktop.impl.portal.desktop.kde");
}

QString desktopPortalPath()
{
    return QStringLiteral("/org/freedesktop/portal/desktop");
}
} // namespace

KIOOpenWith::KIOOpenWith(QWidget *parentWidget, QObject *parent)
    : KIO::OpenWithHandlerInterface(parent)
    , m_parentWidget(parentWidget)
{
}

void KIOOpenWith::promptUserForApplication(KJob *job, const QList<QUrl> &urls, const QString &mimeType)
{
    Q_UNUSED(mimeType);

    QWidget *widget = nullptr;
    if (job) {
        widget = KJobWidgets::window(job);
    }

    if (!widget) {
        widget = m_parentWidget;
    }

    QString windowId;
    if (widget) {
        widget->window()->winId(); // ensure we have a handle so we can export a window (without this windowHandle() may be null)
        auto services = QGuiApplicationPrivate::platformIntegration()->services();
        if (auto unixServices = dynamic_cast<QGenericUnixServices *>(services)) {
            windowId = unixServices->portalWindowIdentifier(widget->window()->windowHandle());
        }
    }

    QDBusMessage message = QDBusMessage::createMethodCall(desktopPortalService(),
                                                          desktopPortalPath(),
                                                          QStringLiteral("org.freedesktop.impl.portal.AppChooser"),
                                                          QStringLiteral("ChooseApplicationPrivate"));

    QStringList urlStrings;
    for (const auto &url : urls) {
        urlStrings << url.toString();
    }

    KConfigGroup cg(KSharedConfig::openStateConfig(), QStringLiteral("Open-with settings"));
    // FIXME not used for anything inside the portal
    const auto completionMode = cg.readEntry("CompletionMode", int(KCompletion::CompletionNone));
    const QStringList history = cg.readEntry("History", QStringList());
    const QString lastChoice = cg.readEntry("LastChoice", QString());

    message << windowId //
            << urlStrings //
            << QVariantMap{
                   {QStringLiteral("ask"), true}, //
                   {QStringLiteral("last_choice"), lastChoice}, //
                   {QStringLiteral("history"), history}, //
                   {QStringLiteral("completionMode"), completionMode}, //
               };

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message, std::numeric_limits<int>::max());
    auto watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, mimeType, cg, widget](QDBusPendingCallWatcher *watcher) mutable {
        watcher->deleteLater();
        onApplicationChosen(*watcher, cg, mimeType, widget);
    });
}

KService::Ptr KIOOpenWith::makeService(const QVariantMap &resultMap, const QString &mimeType, QWidget *widget)
{
    constexpr auto saveNewApps = false; // NOTE: this isn't actually implemented in any UI

    const auto typedExec = resultMap.value(QStringLiteral("choice")).toString();
    const auto remember = resultMap.value(QStringLiteral("remember")).toBool();
    const auto openInTerminal = resultMap.value(QStringLiteral("openInTerminal")).toBool();
    const auto lingerTerminal = resultMap.value(QStringLiteral("lingerTerminal")).toBool();

    auto service = KService::serviceByDesktopName(typedExec);
    auto result = KIO::OpenWith::accept(service, typedExec, remember, mimeType, openInTerminal, lingerTerminal, saveNewApps);
    if (!result.accept) {
        KMessageBox::error(widget, result.error);
        return {};
    }

    if (result.rebuildSycoca) {
        KBuildSycocaProgressDialog::rebuildKSycoca(widget);
    }

    return service;
}

void KIOOpenWith::onApplicationChosen(const QDBusPendingReply<uint, QVariantMap> &reply, KConfigGroup cg, const QString &mimeType, QWidget *widget)
{
    if (reply.isError()) {
        qWarning() << "Couldn't get reply";
        qWarning() << "Error: " << reply.error().message();
        Q_EMIT canceled();
        return;
    }

    if (reply.argumentAt<0>() != 0) {
        Q_EMIT canceled();
        return;
    }

    auto resultMap = reply.argumentAt<1>();
    const QString choice = resultMap.value(QStringLiteral("choice")).toString();
    auto service = makeService(resultMap, mimeType, widget);
    if (!service) {
        // Message already displayed by makeService!
        Q_EMIT canceled();
        return;
    }

    Q_ASSERT(service);
    Q_ASSERT(service->isValid());
    if (!service || !service->isValid()) {
        KMessageBox::error(widget, i18n("Failed to launch for unknown reasons. Please try with a pre-existing application."));
        Q_EMIT canceled();
        return;
    }
    Q_EMIT serviceSelected(service);

    // Save new history
    QStringList history = cg.readEntry("History", QStringList());
    if (history.contains(choice)) {
        history.removeAll(choice);
    }
    history.prepend(choice);
    constexpr auto arbitraryHistoryMax = 15;
    while (history.size() > arbitraryHistoryMax) {
        history.pop_back();
    }
    cg.writeEntry("History", history);
    if (const auto name = service->desktopEntryName(); !name.isEmpty()) {
        cg.writeEntry("LastChoice", name);
    }
}
