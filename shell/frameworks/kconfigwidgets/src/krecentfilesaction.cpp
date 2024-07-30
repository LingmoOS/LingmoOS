/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "krecentfilesaction.h"
#include "krecentfilesaction_p.h"

#include <QActionGroup>
#include <QDir>
#include <QGuiApplication>
#include <QMenu>
#include <QMimeDatabase>
#include <QMimeType>
#include <QScreen>

#if HAVE_QTDBUS
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#endif

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KShell>

#include <set>

KRecentFilesAction::KRecentFilesAction(QObject *parent)
    : KSelectAction(parent)
    , d_ptr(new KRecentFilesActionPrivate(this))
{
    Q_D(KRecentFilesAction);
    d->init();
}

KRecentFilesAction::KRecentFilesAction(const QString &text, QObject *parent)
    : KSelectAction(parent)
    , d_ptr(new KRecentFilesActionPrivate(this))
{
    Q_D(KRecentFilesAction);
    d->init();

    // Want to keep the ampersands
    setText(text);
}

KRecentFilesAction::KRecentFilesAction(const QIcon &icon, const QString &text, QObject *parent)
    : KSelectAction(parent)
    , d_ptr(new KRecentFilesActionPrivate(this))
{
    Q_D(KRecentFilesAction);
    d->init();

    setIcon(icon);
    // Want to keep the ampersands
    setText(text);
}

void KRecentFilesActionPrivate::init()
{
    Q_Q(KRecentFilesAction);
    delete q->menu();
    q->setMenu(new QMenu());
    q->setToolBarMode(KSelectAction::MenuMode);
    m_noEntriesAction = q->menu()->addAction(i18n("No Entries"));
    m_noEntriesAction->setObjectName(QStringLiteral("no_entries"));
    m_noEntriesAction->setEnabled(false);
    clearSeparator = q->menu()->addSeparator();
    clearSeparator->setVisible(false);
    clearSeparator->setObjectName(QStringLiteral("separator"));
    clearAction = q->menu()->addAction(QIcon::fromTheme(QStringLiteral("edit-clear-history")), i18n("Clear List"), q, &KRecentFilesAction::clear);
    clearAction->setObjectName(QStringLiteral("clear_action"));
    clearAction->setVisible(false);
    q->setEnabled(false);
    q->connect(q, &KSelectAction::actionTriggered, q, [this](QAction *action) {
        urlSelected(action);
    });

    q->connect(q->menu(), &QMenu::aboutToShow, q, [q] {
        std::vector<RecentActionInfo> &recentActions = q->d_ptr->m_recentActions;
        // Set icons lazily based on the mimetype
        for (auto action : recentActions) {
            if (action.action->icon().isNull()) {
                if (!action.mimeType.isValid()) {
                    action.mimeType = QMimeDatabase().mimeTypeForFile(action.url.path(), QMimeDatabase::MatchExtension);
                }

                if (!action.mimeType.isDefault()) {
                    action.action->setIcon(QIcon::fromTheme(action.mimeType.iconName()));
                }
            }
        }
    });
}

KRecentFilesAction::~KRecentFilesAction() = default;

void KRecentFilesActionPrivate::urlSelected(QAction *action)
{
    Q_Q(KRecentFilesAction);

    auto it = findByAction(action);

    Q_ASSERT(it != m_recentActions.cend()); // Should never happen

    const QUrl url = it->url; // BUG: 461448; see iterator invalidation rules
    Q_EMIT q->urlSelected(url);
}

// TODO: remove this helper function, it will crash if you use it in a loop
void KRecentFilesActionPrivate::removeAction(std::vector<RecentActionInfo>::iterator it)
{
    Q_Q(KRecentFilesAction);
    delete q->KSelectAction::removeAction(it->action);
    m_recentActions.erase(it);
}

int KRecentFilesAction::maxItems() const
{
    Q_D(const KRecentFilesAction);
    return d->m_maxItems;
}

void KRecentFilesAction::setMaxItems(int maxItems)
{
    Q_D(KRecentFilesAction);
    // set new maxItems
    d->m_maxItems = std::max(maxItems, 0);

    // Remove all excess items, oldest (i.e. first added) first
    const int difference = static_cast<int>(d->m_recentActions.size()) - d->m_maxItems;
    if (difference > 0) {
        auto beginIt = d->m_recentActions.begin();
        auto endIt = d->m_recentActions.begin() + difference;
        for (auto it = beginIt; it < endIt; ++it) {
            // Remove the action from the menus, action groups ...etc
            delete KSelectAction::removeAction(it->action);
        }
        d->m_recentActions.erase(beginIt, endIt);
    }
}

static QString titleWithSensibleWidth(const QString &nameValue, const QString &value)
{
    // Calculate 3/4 of screen geometry, we do not want
    // action titles to be bigger than that
    // Since we do not know in which screen we are going to show
    // we choose the min of all the screens
    int maxWidthForTitles = INT_MAX;
    const auto screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        maxWidthForTitles = qMin(maxWidthForTitles, screen->availableGeometry().width() * 3 / 4);
    }
    const QFontMetrics fontMetrics = QFontMetrics(QFont());

    QString title = nameValue + QLatin1String(" [") + value + QLatin1Char(']');
    const int nameWidth = fontMetrics.boundingRect(title).width();
    if (nameWidth > maxWidthForTitles) {
        // If it does not fit, try to cut only the whole path, though if the
        // name is too long (more than 3/4 of the whole text) we cut it a bit too
        const int nameValueMaxWidth = maxWidthForTitles * 3 / 4;
        QString cutNameValue;
        QString cutValue;
        if (nameWidth > nameValueMaxWidth) {
            cutNameValue = fontMetrics.elidedText(nameValue, Qt::ElideMiddle, nameValueMaxWidth);
            cutValue = fontMetrics.elidedText(value, Qt::ElideMiddle, maxWidthForTitles - nameValueMaxWidth);
        } else {
            cutNameValue = nameValue;
            cutValue = fontMetrics.elidedText(value, Qt::ElideMiddle, maxWidthForTitles - nameWidth);
        }
        title = cutNameValue + QLatin1String(" [") + cutValue + QLatin1Char(']');
    }
    return title;
}

void KRecentFilesAction::addUrl(const QUrl &url, const QString &name)
{
    Q_D(KRecentFilesAction);

    // ensure we never add items if we want none
    if (d->m_maxItems == 0) {
        return;
    }

    if (url.isLocalFile() && url.toLocalFile().startsWith(QDir::tempPath())) {
        return;
    }

    // Remove url if it already exists in the list
    removeUrl(url);

    // Remove oldest item if already maxItems in list
    Q_ASSERT(d->m_maxItems > 0);
    if (static_cast<int>(d->m_recentActions.size()) == d->m_maxItems) {
        d->removeAction(d->m_recentActions.begin());
    }

    const QString pathOrUrl(url.toDisplayString(QUrl::PreferLocalFile));
    const QString tmpName = !name.isEmpty() ? name : url.fileName();
#ifdef Q_OS_WIN
    const QString file = url.isLocalFile() ? QDir::toNativeSeparators(pathOrUrl) : pathOrUrl;
#else
    const QString file = pathOrUrl;
#endif

    d->m_noEntriesAction->setVisible(false);
    d->clearSeparator->setVisible(true);
    d->clearAction->setVisible(true);
    setEnabled(true);
    // add file to list
    const QString title = titleWithSensibleWidth(tmpName, KShell::tildeCollapse(file));

#if HAVE_QTDBUS
    static bool isKdeSession = qgetenv("XDG_CURRENT_DESKTOP") == "KDE";
    if (isKdeSession) {
        const QDBusConnection bus = QDBusConnection::sessionBus();
        if (bus.isConnected() && bus.interface()->isServiceRegistered(QStringLiteral("org.kde.ActivityManager"))) {
            const static QString activityService = QStringLiteral("org.kde.ActivityManager");
            const static QString activityResources = QStringLiteral("/ActivityManager/Resources");
            const static QString activityResouceInferface = QStringLiteral("org.kde.ActivityManager.Resources");
            const QMimeType mimeType = QMimeDatabase().mimeTypeForFile(url.path(), QMimeDatabase::MatchExtension);

            const auto urlString = url.toString(QUrl::PreferLocalFile);
            QDBusMessage message =
                QDBusMessage::createMethodCall(activityService, activityResources, activityResouceInferface, QStringLiteral("RegisterResourceEvent"));
            message.setArguments({qApp->desktopFileName(), uint(0) /* WinId */, urlString, uint(0) /* eventType Accessed */});
            bus.asyncCall(message);

            message = QDBusMessage::createMethodCall(activityService, activityResources, activityResouceInferface, QStringLiteral("RegisterResourceMimetype"));
            message.setArguments({urlString, mimeType.name()});
            bus.asyncCall(message);

            message = QDBusMessage::createMethodCall(activityService, activityResources, activityResouceInferface, QStringLiteral("RegisterResourceTitle"));
            message.setArguments({urlString, url.fileName()});
            bus.asyncCall(message);
        }
    }
#endif

    QAction *action = new QAction(title, selectableActionGroup());
    addAction(action, url, tmpName, QMimeType());
}

void KRecentFilesAction::addAction(QAction *action, const QUrl &url, const QString &name, const QMimeType &mimeType)
{
    Q_D(KRecentFilesAction);
    menu()->insertAction(menu()->actions().value(0), action);
    d->m_recentActions.push_back({action, url, name, mimeType});
}

QAction *KRecentFilesAction::removeAction(QAction *action)
{
    Q_D(KRecentFilesAction);
    auto it = d->findByAction(action);
    Q_ASSERT(it != d->m_recentActions.cend());
    d->m_recentActions.erase(it);
    return KSelectAction::removeAction(action);
}

void KRecentFilesAction::removeUrl(const QUrl &url)
{
    Q_D(KRecentFilesAction);

    auto it = d->findByUrl(url);

    if (it != d->m_recentActions.cend()) {
        d->removeAction(it);
    };
}

QList<QUrl> KRecentFilesAction::urls() const
{
    Q_D(const KRecentFilesAction);

    QList<QUrl> list;
    list.reserve(d->m_recentActions.size());

    using Info = KRecentFilesActionPrivate::RecentActionInfo;
    // Reverse order to match how the actions appear in the menu
    std::transform(d->m_recentActions.crbegin(), d->m_recentActions.crend(), std::back_inserter(list), [](const Info &info) {
        return info.url;
    });

    return list;
}

void KRecentFilesAction::clear()
{
    clearEntries();
    Q_EMIT recentListCleared();
}

void KRecentFilesAction::clearEntries()
{
    Q_D(KRecentFilesAction);
    KSelectAction::clear();
    d->m_recentActions.clear();
    d->m_noEntriesAction->setVisible(true);
    d->clearSeparator->setVisible(false);
    d->clearAction->setVisible(false);
    setEnabled(false);
}

void KRecentFilesAction::loadEntries(const KConfigGroup &_config)
{
    Q_D(KRecentFilesAction);
    clearEntries();

    QString key;
    QString value;
    QString nameKey;
    QString nameValue;
    QString title;
    QUrl url;

    KConfigGroup cg = _config;
    // "<default>" means the group was constructed with an empty name
    if (cg.name() == QLatin1String("<default>")) {
        cg = KConfigGroup(cg.config(), QStringLiteral("RecentFiles"));
    }

    std::set<QUrl> seenUrls;

    bool thereAreEntries = false;
    // read file list
    for (int i = 1; i <= d->m_maxItems; i++) {
        key = QStringLiteral("File%1").arg(i);
        value = cg.readPathEntry(key, QString());
        if (value.isEmpty()) {
            continue;
        }
        url = QUrl::fromUserInput(value);

        auto [it, isNewUrl] = seenUrls.insert(url);
        // Don't restore if this url has already been restored (e.g. broken config)
        if (!isNewUrl) {
            continue;
        }

#ifdef Q_OS_WIN
        // convert to backslashes
        if (url.isLocalFile()) {
            value = QDir::toNativeSeparators(value);
        }
#endif

        nameKey = QStringLiteral("Name%1").arg(i);
        nameValue = cg.readPathEntry(nameKey, url.fileName());
        title = titleWithSensibleWidth(nameValue, KShell::tildeCollapse(value));
        if (!value.isNull()) {
            thereAreEntries = true;
            addAction(new QAction(title, selectableActionGroup()), url, nameValue);
        }
    }
    if (thereAreEntries) {
        d->m_noEntriesAction->setVisible(false);
        d->clearSeparator->setVisible(true);
        d->clearAction->setVisible(true);
        setEnabled(true);
    }
}

void KRecentFilesAction::saveEntries(const KConfigGroup &_cg)
{
    Q_D(KRecentFilesAction);

    KConfigGroup cg = _cg;
    // "<default>" means the group was constructed with an empty name
    if (cg.name() == QLatin1String("<default>")) {
        cg = KConfigGroup(cg.config(), QStringLiteral("RecentFiles"));
    }

    cg.deleteGroup();

    // write file list
    int i = 1;
    for (const auto &[action, url, shortName, _] : d->m_recentActions) {
        cg.writePathEntry(QStringLiteral("File%1").arg(i), url.toDisplayString(QUrl::PreferLocalFile));
        cg.writePathEntry(QStringLiteral("Name%1").arg(i), shortName);

        ++i;
    }
}

#include "moc_krecentfilesaction.cpp"
