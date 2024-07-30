// This file is part of the KDE libraries
// SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "krecentfilesmenu.h"

#include <QGuiApplication>
#include <QIcon>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>

class RecentFilesEntry
{
public:
    QUrl url;
    QString displayName;
    QAction *action = nullptr;

    QString titleWithSensibleWidth(QWidget *widget) const
    {
        const QString urlString = url.toDisplayString(QUrl::PreferLocalFile);
        // Calculate 3/4 of screen geometry, we do not want
        // action titles to be bigger than that
        // Since we do not know in which screen we are going to show
        // we choose the min of all the screens
        int maxWidthForTitles = INT_MAX;
        const auto screens = QGuiApplication::screens();
        for (QScreen *screen : screens) {
            maxWidthForTitles = qMin(maxWidthForTitles, screen->availableGeometry().width() * 3 / 4);
        }

        const QFontMetrics fontMetrics = widget->fontMetrics();

        QString title = displayName + QLatin1String(" [") + urlString + QLatin1Char(']');
        const int nameWidth = fontMetrics.boundingRect(title).width();
        if (nameWidth > maxWidthForTitles) {
            // If it does not fit, try to cut only the whole path, though if the
            // name is too long (more than 3/4 of the whole text) we cut it a bit too
            const int displayNameMaxWidth = maxWidthForTitles * 3 / 4;
            QString cutNameValue;
            QString cutValue;
            if (nameWidth > displayNameMaxWidth) {
                cutNameValue = fontMetrics.elidedText(displayName, Qt::ElideMiddle, displayNameMaxWidth);
                cutValue = fontMetrics.elidedText(urlString, Qt::ElideMiddle, maxWidthForTitles - displayNameMaxWidth);
            } else {
                cutNameValue = displayName;
                cutValue = fontMetrics.elidedText(urlString, Qt::ElideMiddle, maxWidthForTitles - nameWidth);
            }
            title = cutNameValue + QLatin1String(" [") + cutValue + QLatin1Char(']');
        }
        return title;
    }

    explicit RecentFilesEntry(const QUrl &_url, const QString &_displayName, KRecentFilesMenu *menu)
        : url(_url)
        , displayName(_displayName)
    {
        action = new QAction(titleWithSensibleWidth(menu));
        QObject::connect(action, &QAction::triggered, action, [this, menu]() {
            Q_EMIT menu->urlTriggered(url);
        });
    }

    ~RecentFilesEntry()
    {
        delete action;
    }
};

class KRecentFilesMenuPrivate
{
public:
    explicit KRecentFilesMenuPrivate(KRecentFilesMenu *q_ptr);

    std::vector<RecentFilesEntry *>::iterator findEntry(const QUrl &url);
    void recentFilesChanged() const;

    KRecentFilesMenu *const q;
    QString m_group = QStringLiteral("RecentFiles");
    std::vector<RecentFilesEntry *> m_entries;
    QSettings *m_settings;
    size_t m_maximumItems = 10;
    QAction *m_noEntriesAction;
    QAction *m_clearAction;
};

KRecentFilesMenuPrivate::KRecentFilesMenuPrivate(KRecentFilesMenu *q_ptr)
    : q(q_ptr)
{}

std::vector<RecentFilesEntry *>::iterator KRecentFilesMenuPrivate::findEntry(const QUrl &url)
{
    return std::find_if(m_entries.begin(), m_entries.end(), [url](RecentFilesEntry *entry) {
        return entry->url == url;
    });
}

void KRecentFilesMenuPrivate::recentFilesChanged() const
{
    q->rebuildMenu();
    Q_EMIT q->recentFilesChanged();
}

KRecentFilesMenu::KRecentFilesMenu(const QString &title, QWidget *parent)
    : QMenu(title, parent)
    , d(new KRecentFilesMenuPrivate(this))
{
    setIcon(QIcon::fromTheme(QStringLiteral("document-open-recent")));
    const QString fileName =
        QStringLiteral("%1/%2_recentfiles").arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation), QCoreApplication::applicationName());
    d->m_settings = new QSettings(fileName, QSettings::Format::IniFormat, this);

    d->m_noEntriesAction = new QAction(tr("No Entries"));
    d->m_noEntriesAction->setDisabled(true);

    d->m_clearAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear-history")), tr("Clear List"));

    readFromFile();
}

KRecentFilesMenu::KRecentFilesMenu(QWidget *parent)
    : KRecentFilesMenu(tr("Recent Files"), parent)
{
}

KRecentFilesMenu::~KRecentFilesMenu()
{
    writeToFile();
    qDeleteAll(d->m_entries);
    delete d->m_clearAction;
    delete d->m_noEntriesAction;
}

void KRecentFilesMenu::readFromFile()
{
    qDeleteAll(d->m_entries);
    d->m_entries.clear();

    d->m_settings->beginGroup(d->m_group);
    const int size = d->m_settings->beginReadArray(QStringLiteral("files"));

    d->m_entries.reserve(size);

    for (int i = 0; i < size; ++i) {
        d->m_settings->setArrayIndex(i);

        const QUrl url = d->m_settings->value(QStringLiteral("url")).toUrl();
        RecentFilesEntry *entry = new RecentFilesEntry(url, d->m_settings->value(QStringLiteral("displayName")).toString(), this);
        d->m_entries.push_back(entry);
    }

    d->m_settings->endArray();
    d->m_settings->endGroup();

    d->recentFilesChanged();
}

void KRecentFilesMenu::addUrl(const QUrl &url, const QString &name)
{
    if (d->m_entries.size() == d->m_maximumItems) {
        delete d->m_entries.back();
        d->m_entries.pop_back();
    }

    // If it's already there remove the old one and reinsert so it appears as new
    auto it = d->findEntry(url);
    if (it != d->m_entries.cend()) {
        delete *it;
        d->m_entries.erase(it);
    }

    QString displayName = name;

    if (displayName.isEmpty()) {
        displayName = url.fileName();
    }

    RecentFilesEntry *entry = new RecentFilesEntry(url, displayName, this);
    d->m_entries.insert(d->m_entries.begin(), entry);

    d->recentFilesChanged();
}

void KRecentFilesMenu::removeUrl(const QUrl &url)
{
    auto it = d->findEntry(url);

    if (it == d->m_entries.end()) {
        return;
    }

    delete *it;
    d->m_entries.erase(it);

    d->recentFilesChanged();
}

void KRecentFilesMenu::rebuildMenu()
{
    clear();

    if (d->m_entries.empty()) {
        addAction(d->m_noEntriesAction);
        return;
    }

    for (const RecentFilesEntry *entry : d->m_entries) {
        addAction(entry->action);
    }

    addSeparator();
    addAction(d->m_clearAction);

    // reconnect d->m_clearAction, since it was disconnected in clear()
    connect(d->m_clearAction, &QAction::triggered, this, &KRecentFilesMenu::clearRecentFiles);
}

void KRecentFilesMenu::writeToFile()
{
    d->m_settings->remove(QString());
    d->m_settings->beginGroup(d->m_group);
    d->m_settings->beginWriteArray(QStringLiteral("files"));

    int index = 0;
    for (const RecentFilesEntry *entry : d->m_entries) {
        d->m_settings->setArrayIndex(index);
        d->m_settings->setValue(QStringLiteral("url"), entry->url);
        d->m_settings->setValue(QStringLiteral("displayName"), entry->displayName);
        ++index;
    }

    d->m_settings->endArray();
    d->m_settings->endGroup();
    d->m_settings->sync();
}

QString KRecentFilesMenu::group() const
{
    return d->m_group;
}

void KRecentFilesMenu::setGroup(const QString &group)
{
    d->m_group = group;
    readFromFile();
}

int KRecentFilesMenu::maximumItems() const
{
    return d->m_maximumItems;
}

void KRecentFilesMenu::setMaximumItems(size_t maximumItems)
{
    d->m_maximumItems = maximumItems;

    // Truncate if there are more entries than the new maximum
    if (d->m_entries.size() > maximumItems) {
        qDeleteAll(d->m_entries.begin() + maximumItems, d->m_entries.end());
        d->m_entries.erase(d->m_entries.begin() + maximumItems, d->m_entries.end());

        d->recentFilesChanged();
    }
}

QList<QUrl> KRecentFilesMenu::recentFiles() const
{
    QList<QUrl> urls;
    urls.reserve(d->m_entries.size());
    std::transform(d->m_entries.cbegin(), d->m_entries.cend(), std::back_inserter(urls), [](const RecentFilesEntry *entry) {
        return entry->url;
    });

    return urls;
}

void KRecentFilesMenu::clearRecentFiles()
{
    qDeleteAll(d->m_entries);
    d->m_entries.clear();

    d->recentFilesChanged();
}

#include "moc_krecentfilesmenu.cpp"
