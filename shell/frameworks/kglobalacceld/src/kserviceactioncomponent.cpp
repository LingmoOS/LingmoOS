/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kserviceactioncomponent.h"
#include "globalshortcutcontext.h"
#include "logging_p.h"

#include <QFileInfo>

#include <KIO/ApplicationLauncherJob>
#include <KIO/UntrustedProgramHandlerInterface>
#include <KNotificationJobUiDelegate>

#include "config-kglobalaccel.h"
#if HAVE_X11
#include <KStartupInfo>
#include <private/qtx11extras_p.h>
#endif

class UntrustedProgramHandler : public KIO::UntrustedProgramHandlerInterface
{
public:
    UntrustedProgramHandler(QObject *parent)
        : KIO::UntrustedProgramHandlerInterface(parent)
    {
    }

    void showUntrustedProgramWarning(KJob * /*job*/, const QString & /*programName*/) override
    {
        Q_EMIT result(true);
    }
};

QString makeUniqueName(const KService::Ptr &service)
{
    if (service->storageId().startsWith(QLatin1Char('/'))) {
        return QFileInfo(service->storageId()).fileName();
    }

    return service->storageId();
}

KServiceActionComponent::KServiceActionComponent(KService::Ptr service)
    : Component(makeUniqueName(service), service->name())
    , m_service(service)
{
}

KServiceActionComponent::~KServiceActionComponent() = default;

void KServiceActionComponent::emitGlobalShortcutPressed(const GlobalShortcut &shortcut)
{
    KIO::ApplicationLauncherJob *job = nullptr;

    if (shortcut.uniqueName() == QLatin1String("_launch")) {
        job = new KIO::ApplicationLauncherJob(m_service);
    } else {
        const auto actions = m_service->actions();
        const auto it = std::find_if(actions.cbegin(), actions.cend(), [&shortcut](const KServiceAction &action) {
            return action.name() == shortcut.uniqueName();
        });
        if (it == actions.cend()) {
            qCCritical(KGLOBALACCELD, "failed to find an action matching the '%s' name", qPrintable(shortcut.uniqueName()));
            return;
        } else {
            job = new KIO::ApplicationLauncherJob(*it);
        }
    }

    auto *delegate = new KNotificationJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled);
    // ApplicationLauncherJob refuses to launch desktop files in /usr/share/kglobalaccel/ unless they are marked as executable
    // to avoid that add our own UntrustedProgramHandler that accepts the launch regardless
    new UntrustedProgramHandler(delegate);
    job->setUiDelegate(delegate);
#if HAVE_X11
    if (QX11Info::isPlatformX11()) {
        // Create a startup id ourselves. Otherwise ApplicationLauncherJob will query X11 to get a timestamp, which causes a deadlock
        auto startupId = KStartupInfo::createNewStartupIdForTimestamp(QX11Info::appTime());
        job->setStartupId(startupId);
    }
#endif
    job->start();
}

void KServiceActionComponent::loadFromService()
{
    const QString type = m_service->property<QString>(QStringLiteral("X-KDE-GlobalShortcutType"));

    // Type can be Application or Service
    // For applications add a lauch shortcut
    // If no type is set assume Application
    if (type.isEmpty() || type == QLatin1String("Application")) {
        const QString shortcutString = m_service->property<QStringList>(QStringLiteral("X-KDE-Shortcuts")).join(QLatin1Char('\t'));
        GlobalShortcut *shortcut = registerShortcut(QStringLiteral("_launch"), m_service->name(), shortcutString, shortcutString);
        shortcut->setIsPresent(true);
    }

    const auto lstActions = m_service->actions();
    for (const KServiceAction &action : lstActions) {
        const QString shortcutString = action.property<QStringList>(QStringLiteral("X-KDE-Shortcuts")).join(QLatin1Char('\t'));
        GlobalShortcut *shortcut = registerShortcut(action.name(), action.text(), shortcutString, shortcutString);
        shortcut->setIsPresent(true);
    }
}

bool KServiceActionComponent::cleanUp()
{
    qCDebug(KGLOBALACCELD) << "Disabling desktop file";

    const auto shortcuts = allShortcuts();
    for (GlobalShortcut *shortcut : shortcuts) {
        shortcut->setIsPresent(false);
    }

    return Component::cleanUp();
}

void KServiceActionComponent::writeSettings(KConfigGroup &config) const
{
    // Clear the config so we remove entries after forgetGlobalShortcut
    config.deleteGroup();

    // Now write all contexts
    for (GlobalShortcutContext *context : std::as_const(_contexts)) {
        KConfigGroup contextGroup;

        if (context->uniqueName() == QLatin1String("default")) {
            contextGroup = config;
        } else {
            contextGroup = KConfigGroup(&config, context->uniqueName());
        }

        for (const GlobalShortcut *shortcut : std::as_const(context->_actionsMap)) {
            // We do not write fresh shortcuts.
            // We do not write session shortcuts
            if (shortcut->isFresh() || shortcut->isSessionShortcut()) {
                continue;
            }

            if (shortcut->keys() != shortcut->defaultKeys()) {
                contextGroup.writeEntry(shortcut->uniqueName(), stringFromKeys(shortcut->keys()));
            } else {
                contextGroup.revertToDefault(shortcut->uniqueName());
            }
        }
    }
}

void KServiceActionComponent::loadSettings(KConfigGroup &configGroup)
{
    // Action shortcuts
    const auto actions = m_service->actions();
    for (const KServiceAction &action : actions) {
        const QString defaultShortcutString = action.property<QString>(QStringLiteral("X-KDE-Shortcuts")).replace(QLatin1Char(','), QLatin1Char('\t'));
        const QString shortcutString = configGroup.readEntry(action.name(), defaultShortcutString);

        GlobalShortcut *shortcut = registerShortcut(action.name(), action.text(), shortcutString, defaultShortcutString);
        shortcut->setIsPresent(true);
    }

    const QString type = m_service->property<QString>(QStringLiteral("X-KDE-GlobalShortcutType"));

    // Type can be Application or Service
    // For applications add a lauch shortcut
    // If no type is set assume Application
    if (type.isEmpty() || type == QLatin1String("Application")) {
        const QString defaultShortcutString = m_service->property<QString>(QStringLiteral("X-KDE-Shortcuts")).replace(QLatin1Char(','), QLatin1Char('\t'));
        const QString shortcutString = configGroup.readEntry("_launch", defaultShortcutString);
        GlobalShortcut *shortcut = registerShortcut(QStringLiteral("_launch"), m_service->name(), shortcutString, defaultShortcutString);
        shortcut->setIsPresent(true);
    }
}

#include "moc_kserviceactioncomponent.cpp"
