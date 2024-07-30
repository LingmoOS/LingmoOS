/*
 *   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef menuinfo_h
#define menuinfo_h

#include <QList>

#include <KService>
#include <QKeySequence>

class MenuFile;
class MenuEntryInfo;

class MenuInfo
{
public:
    MenuInfo()
    {
    }

    virtual ~MenuInfo()
    {
    }
};

class MenuSeparatorInfo : public MenuInfo
{
public:
    MenuSeparatorInfo()
    {
    }
};

class MenuFolderInfo : public MenuInfo
{
public:
    MenuFolderInfo()
        : dirty(false)
        , hidden(false)
    {
    }

    ~MenuFolderInfo() override;
    // Add separator
    void add(MenuSeparatorInfo *, bool initial = false);

    // Add sub menu
    void add(MenuFolderInfo *, bool initial = false);

    // Remove sub menu (without deleting it)
    void take(MenuFolderInfo *);

    // Remove sub menu (without deleting it)
    // @return true if found
    bool takeRecursive(MenuFolderInfo *info);

    // Add entry
    void add(MenuEntryInfo *, bool initial = false);

    // Remove entry (without deleting it)
    void take(MenuEntryInfo *);

    // Return a unique sub-menu caption inspired by @p caption
    QString uniqueMenuCaption(const QString &caption);

    // Return a unique item caption inspired by @p caption but different
    // from @p exclude
    QString uniqueItemCaption(const QString &caption, const QString &exclude = QString());

    // Update full id's for this item and all submenus
    void updateFullId(const QString &parentId);

    // Return a list of existing submenu ids
    QStringList existingMenuIds();

    void setCaption(const QString &_caption)
    {
        if (_caption == caption) {
            return;
        }
        caption = _caption;
        setDirty();
    }

    void setIcon(const QString &_icon)
    {
        if (_icon == icon) {
            return;
        }
        icon = _icon;
        setDirty();
    }

    void setGenericName(const QString &_description)
    {
        if (_description == genericname) {
            return;
        }
        genericname = _description;
        setDirty();
    }

    void setComment(const QString &_comment)
    {
        if (_comment == comment) {
            return;
        }
        comment = _comment;
        setDirty();
    }

    // Mark menu as dirty
    void setDirty();

    // Return whether this menu or any entry or submenu contained in it is dirty.
    bool hasDirt();

    // Return whether this menu should be explicitly added to its parent menu
    bool needInsertion();

    // Save menu and all its entries and submenus
    void save(MenuFile *);

    // Search service by shortcut
    KService::Ptr findServiceShortcut(const QKeySequence &);

    // Set whether the entry is in active use (as opposed to in the clipboard/deleted)
    void setInUse(bool inUse);

public:
    QString id; // Relative to parent
    QString fullId; // Name in tree
    QString caption; // Visible name
    QString genericname; // Generic description
    QString comment; // Comment
    QString directoryFile; // File describing this folder.
    QString icon; // Icon
    QList<MenuFolderInfo *> subFolders; // Sub menus in this folder
    QList<MenuEntryInfo *> entries; // Menu entries in this folder
    QList<MenuInfo *> initialLayout; // Layout of menu entries according to sycoca
    bool dirty;
    bool hidden;
};

class MenuEntryInfo : public MenuInfo
{
public:
    explicit MenuEntryInfo(const KService::Ptr &_service, KDesktopFile *_df = nullptr)
        : service(_service)
        , m_desktopFile(_df)
        , shortcutLoaded(false)
        , shortcutDirty(false)
        , dirty(_df != 0)
        , hidden(false)
    {
        caption = service->name();
        description = service->genericName();
        icon = service->icon();
    }

    ~MenuEntryInfo() override;

    void setCaption(const QString &_caption);
    void setDescription(const QString &_description);
    void setIcon(const QString &_icon);

    QString menuId() const
    {
        return service->menuId();
    }

    QString file() const
    {
        return service->entryPath();
    }

    QKeySequence shortcut();
    void setShortcut(const QKeySequence &_shortcut);
    bool isShortcutAvailable(const QKeySequence &_shortcut);

    void setDirty();

    // Set whether the entry is in active use (as opposed to in the clipboard/deleted)
    void setInUse(bool inUse);

    // Return whether this menu should be explicitly added to its parent menu
    bool needInsertion();

    void save();

    KDesktopFile *desktopFile();

public:
    QString caption;
    QString description;
    QString icon;
    KService::Ptr service;
    KDesktopFile *m_desktopFile;
    QKeySequence shortCut;
    bool shortcutLoaded;
    bool shortcutDirty;
    bool dirty;
    bool hidden;
};

#endif
