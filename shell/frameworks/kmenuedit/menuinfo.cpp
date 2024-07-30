/*
 *   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "menuinfo.h"

#include <QRegularExpressionMatch>

#include <KDesktopFile>

#include <KConfigGroup>
#include <QStandardPaths>

#include "globalaccel.h"
#include "menufile.h"

//
// MenuFolderInfo
//

static QStringList *s_newShortcuts = nullptr;
static QStringList *s_freeShortcuts = nullptr;
static QStringList *s_deletedApps = nullptr;

// Add separator
MenuFolderInfo::~MenuFolderInfo()
{
    qDeleteAll(subFolders);
    subFolders.clear();
}

void MenuFolderInfo::add(MenuSeparatorInfo *info, bool initial)
{
    if (initial) {
        initialLayout.append(info);
    }
}

// Add sub menu
void MenuFolderInfo::add(MenuFolderInfo *info, bool initial)
{
    subFolders.append(info);
    if (initial) {
        initialLayout.append(info);
    }
}

// Remove sub menu (without deleting it)
void MenuFolderInfo::take(MenuFolderInfo *info)
{
    subFolders.removeAll(info);
}

// Remove sub menu (without deleting it)
bool MenuFolderInfo::takeRecursive(MenuFolderInfo *info)
{
    if (subFolders.removeAll(info) > 0) {
        return true;
    }

    foreach (MenuFolderInfo *subFolderInfo, subFolders) {
        if (subFolderInfo->takeRecursive(info)) {
            return true;
        }
    }
    return false;
}

// Recursively update all fullIds
void MenuFolderInfo::updateFullId(const QString &parentId)
{
    fullId = parentId + id;

    foreach (MenuFolderInfo *subFolderInfo, subFolders) {
        subFolderInfo->updateFullId(fullId);
    }
}

// Add entry
void MenuFolderInfo::add(MenuEntryInfo *entry, bool initial)
{
    entries.append(entry);
    if (initial) {
        initialLayout.append(entry);
    }
}

// Remove entry
void MenuFolderInfo::take(MenuEntryInfo *entry)
{
    entries.removeAll(entry);
}

QString uniqueCaption(const QString &caption)
{
    static const QRegularExpression re(QStringLiteral("(.*)(?=-\\d+)"));
    const QRegularExpressionMatch match = re.match(caption);
    return match.hasMatch() ? match.captured(1) : caption;
}

// Return a unique sub-menu caption inspired by @p caption
QString MenuFolderInfo::uniqueMenuCaption(const QString &caption)
{
    QString cap = uniqueCaption(caption);

    QString result = caption;

    for (int n = 1; ++n;) {
        bool ok = true;
        foreach (MenuFolderInfo *subFolderInfo, subFolders) {
            if (subFolderInfo->caption == result) {
                ok = false;
                break;
            }
        }
        if (ok) {
            return result;
        }

        result = cap + QStringLiteral("-%1").arg(n);
    }
    return QString(); // Never reached
}

// Return a unique item caption inspired by @p caption
QString MenuFolderInfo::uniqueItemCaption(const QString &caption, const QString &exclude)
{
    QString cap = uniqueCaption(caption);

    QString result = caption;

    for (int n = 1; ++n;) {
        bool ok = true;
        if (result == exclude) {
            ok = false;
        }
        foreach (MenuEntryInfo *entryInfo, entries) {
            if (entryInfo->caption == result) {
                ok = false;
                break;
            }
        }
        if (ok) {
            return result;
        }

        result = cap + QStringLiteral("-%1").arg(n);
    }
    return QString(); // Never reached
}

// Return a list of existing submenu ids
QStringList MenuFolderInfo::existingMenuIds()
{
    QStringList result;
    foreach (MenuFolderInfo *subFolderInfo, subFolders) {
        result.append(subFolderInfo->id);
    }
    return result;
}

void MenuFolderInfo::setDirty()
{
    dirty = true;
}

void MenuFolderInfo::save(MenuFile *menuFile)
{
    if (s_deletedApps) {
        // Remove hotkeys for applications that have been deleted
        for (QStringList::ConstIterator it = s_deletedApps->constBegin(); it != s_deletedApps->constEnd(); ++it) {
            // The shortcut is deleted if we set a empty sequence
            GlobalAccel::changeMenuEntryShortcut(KService::serviceByStorageId(*it), QKeySequence());
        }
        delete s_deletedApps;
        s_deletedApps = nullptr;
    }

    if (dirty) {
        QString local = KDesktopFile::locateLocal(directoryFile);

        KDesktopFile *df = nullptr;
        if (directoryFile != local) {
            KDesktopFile orig(QStandardPaths::ApplicationsLocation, directoryFile);
            df = orig.copyTo(local);
        } else {
            df = new KDesktopFile(QStandardPaths::ApplicationsLocation, directoryFile);
        }

        KConfigGroup dg(df->desktopGroup());
        dg.writeEntry("Name", caption);
        dg.writeEntry("GenericName", genericname);
        dg.writeEntry("Comment", comment);
        dg.writeEntry("Icon", icon);
        dg.sync();
        delete df;
        dirty = false;
    }

    // Save sub-menus
    foreach (MenuFolderInfo *subFolderInfo, subFolders) {
        subFolderInfo->save(menuFile);
    }

    // Save entries
    foreach (MenuEntryInfo *entryInfo, entries) {
        if (entryInfo->needInsertion()) {
            menuFile->addEntry(fullId, entryInfo->menuId());
        }
        entryInfo->save();
    }
}

bool MenuFolderInfo::hasDirt()
{
    if (dirty) {
        return true;
    }

    // Check sub-menus
    foreach (MenuFolderInfo *subFolderInfo, subFolders) {
        if (subFolderInfo->hasDirt()) {
            return true;
        }
    }

    // Check entries
    foreach (MenuEntryInfo *entryInfo, entries) {
        if (entryInfo->dirty || entryInfo->shortcutDirty) {
            return true;
        }
    }
    return false;
}

KService::Ptr MenuFolderInfo::findServiceShortcut(const QKeySequence &cut)
{
    KService::Ptr result;
    // Check sub-menus
    foreach (MenuFolderInfo *subFolderInfo, subFolders) {
        result = subFolderInfo->findServiceShortcut(cut);
        if (result) {
            return result;
        }
    }

    // Check entries
    foreach (MenuEntryInfo *entryInfo, entries) {
        if (entryInfo->shortCut == cut) {
            return entryInfo->service;
        }
    }
    return KService::Ptr();
}

void MenuFolderInfo::setInUse(bool inUse)
{
    // Propagate to sub-menus
    foreach (MenuFolderInfo *subFolderInfo, subFolders) {
        subFolderInfo->setInUse(inUse);
    }

    // Propagate to entries
    foreach (MenuEntryInfo *entryInfo, entries) {
        entryInfo->setInUse(inUse);
    }
}

//
// MenuEntryInfo
//

MenuEntryInfo::~MenuEntryInfo()
{
    m_desktopFile->markAsClean();
    delete m_desktopFile;
}

KDesktopFile *MenuEntryInfo::desktopFile()
{
    if (!m_desktopFile) {
        m_desktopFile = new KDesktopFile(service->entryPath());
    }
    return m_desktopFile;
}

void MenuEntryInfo::setDirty()
{
    if (dirty) {
        return;
    }

    dirty = true;

    QString local = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1Char('/') + service->menuId();
    if (local != service->entryPath()) {
        KDesktopFile *oldDf = desktopFile();
        m_desktopFile = oldDf->copyTo(local);
        delete oldDf;
    }
}

bool MenuEntryInfo::needInsertion()
{
    // If entry is dirty and previously stored under applnk, then we need to be added explicitly
    return dirty && !service->entryPath().startsWith(QLatin1Char('/'));
}

void MenuEntryInfo::save()
{
    if (dirty) {
        m_desktopFile->sync();
        dirty = false;
    }
    if (shortcutDirty) {
        GlobalAccel::changeMenuEntryShortcut(service, shortCut);
        shortcutDirty = false;
    }
}

void MenuEntryInfo::setCaption(const QString &_caption)
{
    if (caption == _caption) {
        return;
    }
    caption = _caption;
    setDirty();
    desktopFile()->desktopGroup().writeEntry("Name", caption);
}

void MenuEntryInfo::setDescription(const QString &_description)
{
    if (description == _description) {
        return;
    }
    description = _description;
    setDirty();
    desktopFile()->desktopGroup().writeEntry("GenericName", description);
}

void MenuEntryInfo::setIcon(const QString &_icon)
{
    if (icon == _icon) {
        return;
    }

    icon = _icon;
    setDirty();
    desktopFile()->desktopGroup().writeEntry("Icon", icon);
}

QKeySequence MenuEntryInfo::shortcut()
{
    if (!shortcutLoaded) {
        shortcutLoaded = true;
        shortCut = GlobalAccel::getMenuEntryShortcut(service);
    }
    return shortCut;
}

static void freeShortcut(const QKeySequence &shortCut)
{
    if (!shortCut.isEmpty()) {
        QString shortcutKey = shortCut.toString();
        if (s_newShortcuts) {
            s_newShortcuts->removeAll(shortcutKey);
        }

        if (!s_freeShortcuts) {
            s_freeShortcuts = new QStringList;
        }

        s_freeShortcuts->append(shortcutKey);
    }
}

static void allocateShortcut(const QKeySequence &shortCut)
{
    if (!shortCut.isEmpty()) {
        QString shortcutKey = shortCut.toString();
        if (s_freeShortcuts) {
            s_freeShortcuts->removeAll(shortcutKey);
        }

        if (!s_newShortcuts) {
            s_newShortcuts = new QStringList;
        }

        s_newShortcuts->append(shortcutKey);
    }
}

void MenuEntryInfo::setShortcut(const QKeySequence &_shortcut)
{
    if (shortCut == _shortcut) {
        return;
    }

    freeShortcut(shortCut);
    allocateShortcut(_shortcut);

    shortCut = _shortcut;
    if (shortCut.isEmpty()) {
        shortCut = QKeySequence(); // Normalize
    }
    shortcutLoaded = true;
    shortcutDirty = true;
}

void MenuEntryInfo::setInUse(bool inUse)
{
    if (inUse) {
        QKeySequence temp = shortcut();
        shortCut = QKeySequence();
        if (isShortcutAvailable(temp)) {
            shortCut = temp;
        } else {
            shortcutDirty = true;
        }
        allocateShortcut(shortCut);

        if (s_deletedApps) {
            s_deletedApps->removeAll(service->storageId());
        }
    } else {
        freeShortcut(shortcut());

        // Add to list of deleted apps
        if (!s_deletedApps) {
            s_deletedApps = new QStringList;
        }

        s_deletedApps->append(service->storageId());
    }
}

bool MenuEntryInfo::isShortcutAvailable(const QKeySequence &_shortcut)
{
    // We only have to check agains not saved local shortcuts.
    // KKeySequenceWidget checks against all other registered shortcuts.
    if (shortCut == _shortcut) {
        return true;
    }

    QString shortcutKey = _shortcut.toString();
    bool available = true;
    if (available && s_newShortcuts) {
        available = !s_newShortcuts->contains(shortcutKey);
    }
    if (!available && s_freeShortcuts) {
        available = s_freeShortcuts->contains(shortcutKey);
    }
    return available;
}
