/*
 *   SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
 *   SPDX-FileCopyrightText: 2001-2002 Raffaele Sandrini <sandrini@kde.org>
 *   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
 *   SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef treeview_h
#define treeview_h

#include <QMimeData>
#include <QTreeWidget>

#include <KService>
#include <KServiceGroup>

class QMenu;
class QDropEvent;

class KActionCollection;
class KDesktopFile;
class MenuFile;
class MenuFolderInfo;
class MenuEntryInfo;
class MenuSeparatorInfo;
class QKeySequence;

static const QString SAVE_ACTION_NAME = QStringLiteral("file_save");
static const QString NEW_ITEM_ACTION_NAME = QStringLiteral("new_item");
static const QString NEW_SUBMENU_ACTION_NAME = QStringLiteral("new_submenu");
static const QString NEW_SEPARATOR_ACTION_NAME = QStringLiteral("new_separator");
static const QString CUT_ACTION_NAME = QStringLiteral("edit_cut");
static const QString COPY_ACTION_NAME = QStringLiteral("edit_copy");
static const QString PASTE_ACTION_NAME = QStringLiteral("edit_paste");
static const QString DELETE_ACTION_NAME = QStringLiteral("delete");
static const QString SORT_ACTION_NAME = QStringLiteral("sort");
static const QString SORT_BY_NAME_ACTION_NAME = QStringLiteral("sort_by_name");
static const QString SORT_BY_DESCRIPTION_ACTION_NAME = QStringLiteral("sort_by_description");
static const QString SORT_ALL_BY_NAME_ACTION_NAME = QStringLiteral("sort_all_by_name");
static const QString SORT_ALL_BY_DESCRIPTION_ACTION_NAME = QStringLiteral("sort_all_by_description");
static const QString MOVE_UP_ACTION_NAME = QStringLiteral("move_up");
static const QString MOVE_DOWN_ACTION_NAME = QStringLiteral("move_down");

class TreeItem : public QTreeWidgetItem
{
public:
    TreeItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, const QString &menuId, bool __init = false);
    TreeItem(QTreeWidget *parent, QTreeWidgetItem *after, const QString &menuId, bool __init = false);
    ~TreeItem() override;
    static bool itemNameLessThan(QTreeWidgetItem *item1, QTreeWidgetItem *item2);
    static bool itemDescriptionLessThan(QTreeWidgetItem *item1, QTreeWidgetItem *item2);

    QString menuId() const
    {
        return m_menuId;
    }

    QString directory() const
    {
        return m_directoryPath;
    }

    void setDirectoryPath(const QString &path)
    {
        m_directoryPath = path;
    }

    MenuFolderInfo *folderInfo()
    {
        return m_folderInfo;
    }

    void setMenuFolderInfo(MenuFolderInfo *folderInfo)
    {
        m_folderInfo = folderInfo;
    }

    MenuEntryInfo *entryInfo() const
    {
        return m_entryInfo;
    }

    void setMenuEntryInfo(MenuEntryInfo *entryInfo)
    {
        m_entryInfo = entryInfo;
    }

    QString name() const
    {
        return m_name;
    }

    void setName(const QString &name);

    QString description() const;

    bool isDirectory() const
    {
        return m_folderInfo;
    }

    bool isEntry() const
    {
        return m_entryInfo;
    }

    bool isSeparator() const
    {
        return !isDirectory() && !isEntry();
    }

    bool isHiddenInMenu() const
    {
        return m_hidden;
    }

    void setHiddenInMenu(bool b);

    bool isLayoutDirty() const;
    void setLayoutDirty()
    {
        m_layoutDirty = true;
    }

    void saveLayout(MenuFile *menuFile);

    void load();

    // virtual void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);

private:
    void update();

    bool m_hidden : 1;
    bool m_init : 1;
    bool m_layoutDirty : 1;
    QString m_menuId;
    QString m_name;
    QString m_directoryPath;
    MenuFolderInfo *m_folderInfo;
    MenuEntryInfo *m_entryInfo;
};

class TreeView : public QTreeWidget
{
    friend class TreeItem;
    Q_OBJECT
public:
    explicit TreeView(KActionCollection *ac, QWidget *parent = nullptr);
    ~TreeView() override;

    void readMenuFolderInfo(MenuFolderInfo *folderInfo = 0, KServiceGroup::Ptr folder = KServiceGroup::Ptr(), const QString &prefix = QString());
    void setViewMode(bool showHidden);
    bool save();

    bool dirty();

    void selectMenu(const QString &menu);
    void selectMenuEntry(const QString &menuEntry);

    void restoreMenuSystem();

    void updateTreeView(bool showHidden);

public Q_SLOTS:
    void currentDataChanged(MenuFolderInfo *folderInfo);
    void currentDataChanged(MenuEntryInfo *entryInfo);
    void findServiceShortcut(const QKeySequence &, KService::Ptr &);
    void searchUpdated(const QString &searchString);

Q_SIGNALS:
    void entrySelected(MenuFolderInfo *folderInfo);
    void entrySelected(MenuEntryInfo *entryInfo);
    void disableAction();

protected Q_SLOTS:
    void itemSelected(QTreeWidgetItem *);
    bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action) override;

    void newsubmenu();
    void newitem();
    void newsep();

    void cut();
    void copy();
    void paste();
    void del();
    void sort(const int sortCmd);
    void moveUpItem();
    void moveDownItem();

protected:
    enum SortType {
        SortByName = 0,
        SortByDescription,
        SortAllByName,
        SortAllByDescription,
    };

    void contextMenuEvent(QContextMenuEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;
    QTreeWidgetItem *selectedItem();
    TreeItem *createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuFolderInfo *folderInfo, bool _init = false);
    TreeItem *createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuEntryInfo *entryInfo, bool _init = false);
    TreeItem *createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuSeparatorInfo *sepInfo, bool _init = false);

    void del(TreeItem *, bool deleteInfo);
    void fill();
    void fillBranch(MenuFolderInfo *folderInfo, TreeItem *parent);
    QString findName(KDesktopFile *df, bool deleted);
    void sortItem(TreeItem *item, SortType sortType);
    void sortItemChildren(const QList<QTreeWidgetItem *>::iterator &begin, const QList<QTreeWidgetItem *>::iterator &end, SortType sortType);
    TreeItem *getParentItem(QTreeWidgetItem *item) const;
    void moveUpOrDownItem(bool isMovingUpAction);

    TreeItem *expandPath(TreeItem *item, const QString &path);

    // moving = src will be removed later
    void copy(bool moving);

    void cleanupClipboard();

    bool isLayoutDirty();
    void setLayoutDirty(TreeItem *);
    void saveLayout();

    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QList<QTreeWidgetItem *> &items) const override;
    Qt::DropActions supportedDropActions() const override;

    void sendReloadMenu();

private:
    KActionCollection *m_ac = nullptr;
    QMenu *m_popupMenu = nullptr;
    int m_clipboard;
    MenuFolderInfo *m_clipboardFolderInfo = nullptr;
    MenuEntryInfo *m_clipboardEntryInfo = nullptr;
    bool m_showHidden;
    MenuFile *m_menuFile = nullptr;
    MenuFolderInfo *m_rootFolder = nullptr;
    MenuSeparatorInfo *m_separator = nullptr;
    QStringList m_newMenuIds;
    QStringList m_newDirectoryList;
    bool m_layoutDirty;
    bool m_detailedMenuEntries;
    bool m_detailedEntriesNamesFirst;
    QStringList m_dropMimeTypes;
};

class MenuItemMimeData : public QMimeData
{
    Q_OBJECT
public:
    explicit MenuItemMimeData(TreeItem *item);
    QStringList formats() const override;
    bool hasFormat(const QString &mimeType) const override;
    TreeItem *item() const;

protected:
    QVariant retrieveData(const QString &mimeType, QMetaType type) const override;

private:
    TreeItem *m_item = nullptr;
};

Q_DECLARE_METATYPE(TreeItem *)

#endif
