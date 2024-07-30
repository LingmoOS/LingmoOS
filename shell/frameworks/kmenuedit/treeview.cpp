/*
 *   SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
 *   SPDX-FileCopyrightText: 2001-2002 Raffaele Sandrini <sandrini@kde.org>
 *   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
 *   SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "treeview.h"

#include <unistd.h>

#include <QAction>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <QDrag>
#include <QDropEvent>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QMenu>
#include <QPainter>
#include <QRegularExpressionMatch>
#include <QUrl>

#include "kmenuedit_debug.h"
#include <KActionCollection>
#include <KBuildSycocaProgressDialog>
#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStringHandler>
#include <KUrlMimeData>
#include <QStandardPaths>

#include "menufile.h"
#include "menuinfo.h"

#define MOVE_FOLDER 'M'
#define COPY_FOLDER 'C'
#define MOVE_FILE 'm'
#define COPY_FILE 'c'
#define COPY_SEPARATOR 'S'

static const char *s_internalMimeType = "application/x-kmenuedit-internal";

class SeparatorWidget : public QWidget
{
public:
    SeparatorWidget()
        : QWidget(nullptr)
    {
    }

protected:
    void paintEvent(QPaintEvent * /*event*/) override
    {
        QPainter p(this);
        // Draw Separator
        int h = (height() / 2) - 1;
        //        if (isSelected()) {
        //            p->setPen( palette().color( QPalette::HighlightedText ) );
        //        } else {
        //            p->setPen( palette().color( QPalette::Text ) );
        //        }

        p.drawLine(2, h, width() - 4, h);
    }
};

TreeItem::TreeItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, const QString &menuId, bool _m_init)
    : QTreeWidgetItem(parent, after)
    , m_hidden(false)
    , m_init(_m_init)
    , m_layoutDirty(false)
    , m_menuId(menuId)
    , m_folderInfo(nullptr)
    , m_entryInfo(nullptr)
{
}

TreeItem::TreeItem(QTreeWidget *parent, QTreeWidgetItem *after, const QString &menuId, bool _m_init)
    : QTreeWidgetItem(parent, after)
    , m_hidden(false)
    , m_init(_m_init)
    , m_layoutDirty(false)
    , m_menuId(menuId)
    , m_folderInfo(nullptr)
    , m_entryInfo(nullptr)
{
    load();
}

TreeItem::~TreeItem()
{
}

/**
 * @brief Return the description.
 * @return Description, or an empty string if none.
 */
QString TreeItem::description() const
{
    QString description;
    if (isEntry()) {
        description = entryInfo()->description;
    }
    return description;
}

/**
 * @brief Compare two items using their names.
 * @param item1 First item.
 * @param item2 Second item.
 * @return Integer less than, equal to, or greater than zero if item1 is less than, equal to, or greater than item2.
 */
bool TreeItem::itemNameLessThan(QTreeWidgetItem *item1, QTreeWidgetItem *item2)
{
    TreeItem *treeItem1 = static_cast<TreeItem *>(item1);
    TreeItem *treeItem2 = static_cast<TreeItem *>(item2);
    return QString::localeAwareCompare(treeItem1->name(), treeItem2->name()) < 0;
}

/**
 * @brief Compare two items using their descriptions. If both are empty, sort them by name.
 * @param item1 First item.
 * @param item2 Second item.
 * @return Integer less than, equal to, or greater than zero if item1 is less than, equal to, or greater than item2.
 */
bool TreeItem::itemDescriptionLessThan(QTreeWidgetItem *item1, QTreeWidgetItem *item2)
{
    // extract descriptions in lower case
    TreeItem *treeItem1 = static_cast<TreeItem *>(item1);
    TreeItem *treeItem2 = static_cast<TreeItem *>(item2);
    const QString description1 = treeItem1->description();
    const QString description2 = treeItem2->description();

    // if description is missing for both items, sort them using their names
    if (description1.isEmpty() && description2.isEmpty()) {
        return itemNameLessThan(item1, item2);
    } else {
        return QString::localeAwareCompare(description1, description2) < 0;
    }
}

void TreeItem::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    update();
}

void TreeItem::setHiddenInMenu(bool b)
{
    if (m_hidden == b) {
        return;
    }

    m_hidden = b;
    update();
}

void TreeItem::update()
{
    QString s = m_name;
    if (m_hidden) {
        s += i18n(" [Hidden]");
    }

    setText(0, s);
}

void TreeItem::load()
{
    if (m_folderInfo && !m_init) {
        m_init = true;
        TreeView *tv = static_cast<TreeView *>(treeWidget());
        tv->fillBranch(m_folderInfo, this);
    }
}

bool TreeItem::isLayoutDirty() const
{
    if (m_layoutDirty) {
        return true;
    }

    for (int i = 0; i < childCount(); ++i) {
        TreeItem *item = dynamic_cast<TreeItem *>(child(i));
        if (!item) {
            continue;
        }

        if (item->isLayoutDirty()) {
            return true;
        }
    }

    return false;
}

TreeView::TreeView(KActionCollection *ac, QWidget *parent)
    : QTreeWidget(parent)
    , m_ac(ac)
    , m_popupMenu(nullptr)
    , m_clipboard(0)
    , m_clipboardFolderInfo(nullptr)
    , m_clipboardEntryInfo(nullptr)
    , m_layoutDirty(false)
    , m_detailedMenuEntries(true)
    , m_detailedEntriesNamesFirst(true)
{
    m_dropMimeTypes << QLatin1String(s_internalMimeType) << KUrlMimeData::mimeDataTypes();
    qRegisterMetaType<TreeItem *>("TreeItem");
    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);
    setSortingEnabled(false);
    setDragEnabled(true);
    setAcceptDrops(true);
    setMinimumWidth(240);

    setHeaderLabels(QStringList() << QLatin1String(""));
    header()->hide();

    // listen for creation
    connect(m_ac->action(NEW_ITEM_ACTION_NAME), &QAction::triggered, this, &TreeView::newitem);
    connect(m_ac->action(NEW_SUBMENU_ACTION_NAME), &QAction::triggered, this, &TreeView::newsubmenu);
    connect(m_ac->action(NEW_SEPARATOR_ACTION_NAME), &QAction::triggered, this, &TreeView::newsep);

    // listen for copy
    connect(m_ac->action(CUT_ACTION_NAME), &QAction::triggered, this, &TreeView::cut);
    connect(m_ac->action(COPY_ACTION_NAME), SIGNAL(triggered()), SLOT(copy()));
    connect(m_ac->action(PASTE_ACTION_NAME), &QAction::triggered, this, &TreeView::paste);

    // listen for deleting
    connect(m_ac->action(DELETE_ACTION_NAME), SIGNAL(triggered()), SLOT(del()));

    // listen for sorting
    QAction *action = m_ac->action(SORT_BY_NAME_ACTION_NAME);
    connect(action, &QAction::triggered, this, [this]() {
        sort(SortByName);
    });
    action = m_ac->action(SORT_BY_DESCRIPTION_ACTION_NAME);
    connect(action, &QAction::triggered, this, [this]() {
        sort(SortByDescription);
    });
    action = m_ac->action(SORT_ALL_BY_NAME_ACTION_NAME);
    connect(action, &QAction::triggered, this, [this]() {
        sort(SortAllByName);
    });
    action = m_ac->action(SORT_ALL_BY_DESCRIPTION_ACTION_NAME);
    connect(action, &QAction::triggered, this, [this]() {
        sort(SortAllByDescription);
    });

    // connect moving up/down actions
    connect(m_ac->action(MOVE_UP_ACTION_NAME), &QAction::triggered, this, &TreeView::moveUpItem);
    connect(m_ac->action(MOVE_DOWN_ACTION_NAME), &QAction::triggered, this, &TreeView::moveDownItem);

    // listen for selection
    connect(this, &QTreeWidget::currentItemChanged, this, &TreeView::itemSelected);

    m_menuFile = new MenuFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/menus/") +
                              QStringLiteral("applications-kmenuedit.menu"));
    m_rootFolder = new MenuFolderInfo;
    m_separator = new MenuSeparatorInfo;
}

TreeView::~TreeView()
{
    cleanupClipboard();
    delete m_rootFolder;
    delete m_separator;
    delete m_menuFile;
}

void TreeView::setViewMode(bool showHidden)
{
    // setup popup menu
    delete m_popupMenu;
    m_popupMenu = new QMenu(this);

    // creation
    m_popupMenu->addAction(m_ac->action(NEW_ITEM_ACTION_NAME));
    m_popupMenu->addAction(m_ac->action(NEW_SUBMENU_ACTION_NAME));
    m_popupMenu->addAction(m_ac->action(NEW_SEPARATOR_ACTION_NAME));
    m_popupMenu->addSeparator();

    // copy
    m_popupMenu->addAction(m_ac->action(CUT_ACTION_NAME));
    m_popupMenu->addAction(m_ac->action(COPY_ACTION_NAME));
    m_popupMenu->addAction(m_ac->action(PASTE_ACTION_NAME));
    m_popupMenu->addSeparator();

    // delete
    m_popupMenu->addAction(m_ac->action(DELETE_ACTION_NAME));
    m_popupMenu->addSeparator();

    // move
    m_popupMenu->addAction(m_ac->action(MOVE_UP_ACTION_NAME));
    m_popupMenu->addAction(m_ac->action(MOVE_DOWN_ACTION_NAME));
    m_popupMenu->addSeparator();

    // sort
    m_popupMenu->addAction(m_ac->action(SORT_ACTION_NAME));

    m_showHidden = showHidden;
    readMenuFolderInfo();
    fill();
}

void TreeView::readMenuFolderInfo(MenuFolderInfo *folderInfo, KServiceGroup::Ptr folder, const QString &prefix)
{
    if (!folderInfo) {
        folderInfo = m_rootFolder;
        folder = KServiceGroup::root();
    }

    if (!folder || !folder->isValid()) {
        return;
    }

    folderInfo->caption = folder->caption();
    folderInfo->comment = folder->comment();

    // Item names may contain ampersands. To avoid them being converted
    // to accelerators, replace them with two ampersands.
    folderInfo->hidden = folder->noDisplay();
    folderInfo->directoryFile = folder->directoryEntryPath();
    folderInfo->icon = folder->icon();
    QString id = folder->relPath();
    int i = id.lastIndexOf(QLatin1Char('/'), -2);
    id = id.mid(i + 1);
    folderInfo->id = id;
    folderInfo->fullId = prefix + id;

    for (const KSycocaEntry::Ptr &e : folder->entries(true, !m_showHidden, true, m_detailedMenuEntries && !m_detailedEntriesNamesFirst)) {
        if (e->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr serviceGroup(static_cast<KServiceGroup *>(e.data()));
            MenuFolderInfo *subFolderInfo = new MenuFolderInfo();
            readMenuFolderInfo(subFolderInfo, serviceGroup, folderInfo->fullId);
            folderInfo->add(subFolderInfo, true);
        } else if (e->isType(KST_KService)) {
            const KService::Ptr service(static_cast<KService *>(e.data()));
            folderInfo->add(new MenuEntryInfo(service), true);
        } else if (e->isType(KST_KServiceSeparator)) {
            folderInfo->add(m_separator, true);
        }
    }
}

void TreeView::fill()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    clear();
    fillBranch(m_rootFolder, nullptr);
    QApplication::restoreOverrideCursor();
}

QString TreeView::findName(KDesktopFile *df, bool deleted)
{
    QString name = df->readName();
    if (deleted) {
        if (name == QLatin1String("empty")) {
            name.clear();
        }
        if (name.isEmpty()) {
            bool isLocal = true;
            const QStringList files = QStandardPaths::locateAll(df->locationType(), df->fileName(), QStandardPaths::LocateFile);
            for (QStringList::ConstIterator it = files.constBegin(); it != files.constEnd(); ++it) {
                if (isLocal) {
                    isLocal = false;
                    continue;
                }

                KDesktopFile df2(*it);
                name = df2.readName();

                if (!name.isEmpty() && (name != QLatin1String("empty"))) {
                    return name;
                }
            }
        }
    }
    return name;
}

TreeItem *TreeView::createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuFolderInfo *folderInfo, bool m_init)
{
    TreeItem *item;
    if (parent) {
        item = new TreeItem(parent, after, QString(), m_init);
    } else {
        item = new TreeItem(this, after, QString(), m_init);
    }

    item->setMenuFolderInfo(folderInfo);
    item->setName(folderInfo->caption);
    item->setIcon(0, QIcon::fromTheme(folderInfo->icon));
    item->setDirectoryPath(folderInfo->fullId);
    item->setHiddenInMenu(folderInfo->hidden);
    item->load();
    return item;
}

TreeItem *TreeView::createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuEntryInfo *entryInfo, bool m_init)
{
    bool hidden = entryInfo->hidden;

    TreeItem *item;
    if (parent) {
        item = new TreeItem(parent, after, entryInfo->menuId(), m_init);
    } else {
        item = new TreeItem(this, after, entryInfo->menuId(), m_init);
    }

    QString name;

    if (m_detailedMenuEntries && entryInfo->description.length() != 0) {
        if (m_detailedEntriesNamesFirst) {
            name = entryInfo->caption + QStringLiteral(" (") + entryInfo->description + QLatin1Char(')');
        } else {
            name = entryInfo->description + QStringLiteral(" (") + entryInfo->caption + QLatin1Char(')');
        }
    } else {
        name = entryInfo->caption;
    }

    ////qCDebug(KMENUEDIT_LOG) << parent << after << name;
    item->setMenuEntryInfo(entryInfo);
    item->setName(name);
    item->setIcon(0, QIcon::fromTheme(entryInfo->icon));
    item->setHiddenInMenu(hidden);
    item->load();

    return item;
}

TreeItem *TreeView::createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuSeparatorInfo *, bool init)
{
    TreeItem *item;
    if (parent) {
        item = new TreeItem(parent, after, QString(), init);
    } else {
        item = new TreeItem(this, after, QString(), init);
    }

    setItemWidget(item, 0, new SeparatorWidget);
    return item;
}

void TreeView::fillBranch(MenuFolderInfo *folderInfo, TreeItem *parent)
{
    QString relPath = parent ? parent->directory() : QString();
    TreeItem *after = nullptr;
    foreach (MenuInfo *info, folderInfo->initialLayout) {
        MenuEntryInfo *entry = dynamic_cast<MenuEntryInfo *>(info);
        if (entry) {
            after = createTreeItem(parent, after, entry);
            continue;
        }

        MenuFolderInfo *subFolder = dynamic_cast<MenuFolderInfo *>(info);
        if (subFolder) {
            after = createTreeItem(parent, after, subFolder);
            continue;
        }
        MenuSeparatorInfo *separator = dynamic_cast<MenuSeparatorInfo *>(info);
        if (separator) {
            after = createTreeItem(parent, after, separator);
            continue;
        }
    }
}
TreeItem *TreeView::expandPath(TreeItem *item, const QString &path)
{
    int i = path.indexOf(QLatin1String("/"));
    QString subMenu = path.left(i + 1);
    QString restMenu = path.mid(i + 1);

    for (int i = 0; i < item->childCount(); ++i) {
        TreeItem *childItem = dynamic_cast<TreeItem *>(item->child(i));
        if (!childItem) {
            continue;
        }

        MenuFolderInfo *folderInfo = childItem->folderInfo();
        if (folderInfo && (folderInfo->id == subMenu)) {
            childItem->setExpanded(true);
            if (!restMenu.isEmpty()) {
                return expandPath(childItem, restMenu);
            } else {
                return childItem;
            }
        }
    }

    return nullptr;
}

void TreeView::selectMenu(const QString &menu)
{
    // close all parent expansions and deselect everything
    collapseAll();
    setCurrentIndex(rootIndex());

    if (menu.length() <= 1) {
        setCurrentItem(topLevelItem(0));
        clearSelection();
        return; // Root menu
    }

    QString restMenu = menu;
    if (menu.startsWith(QLatin1Char('/'))) {
        restMenu = menu.mid(1);
    }
    if (!restMenu.endsWith(QLatin1Char('/'))) {
        restMenu += QLatin1Char('/');
    }

    TreeItem *item = nullptr;
    int i = restMenu.indexOf(QLatin1String("/"));
    QString subMenu = restMenu.left(i + 1);
    restMenu = restMenu.mid(i + 1);

    for (int i = 0; i < topLevelItemCount(); ++i) {
        item = dynamic_cast<TreeItem *>(topLevelItem(i));
        if (!item) {
            continue;
        }

        MenuFolderInfo *folderInfo = item->folderInfo();
        if (folderInfo && (folderInfo->id == subMenu)) {
            if (!restMenu.isEmpty()) {
                item = expandPath(item, restMenu);
            }
            break;
        }
    }

    if (item) {
        setCurrentItem(item);
        scrollToItem(item);
    }
}

void TreeView::selectMenuEntry(const QString &menuEntry)
{
    TreeItem *item = static_cast<TreeItem *>(selectedItem());
    if (!item) {
        item = static_cast<TreeItem *>(currentItem());
    }

    if (!item) {
        return;
    }

    QTreeWidgetItem *parent = item->parent();
    if (parent) {
        for (int i = 0; i < parent->childCount(); ++i) {
            TreeItem *item = dynamic_cast<TreeItem *>(parent->child(i));
            if (!item || item->isDirectory()) {
                continue;
            }

            MenuEntryInfo *entry = item->entryInfo();
            if (entry && entry->menuId() == menuEntry) {
                setCurrentItem(item);
                scrollToItem(item);
                return;
            }
        }
    } else {
        // top level
        for (int i = 0; i < topLevelItemCount(); ++i) {
            TreeItem *item = dynamic_cast<TreeItem *>(topLevelItem(i));
            if (!item || item->isDirectory()) {
                continue;
            }

            MenuEntryInfo *entry = item->entryInfo();
            if (entry && entry->menuId() == menuEntry) {
                setCurrentItem(item);
                scrollToItem(item);
                return;
            }
        }
    }
}

void TreeView::itemSelected(QTreeWidgetItem *item)
{
    if (item) {
        // ensure the item is visible as selected
        item->setSelected(true);
    }

    TreeItem *_item = item ? static_cast<TreeItem *>(item) : nullptr;
    TreeItem *parentItem = nullptr;
    bool selected = false;
    bool dselected = false;
    if (_item) {
        selected = true;
        dselected = _item->isHiddenInMenu();
        parentItem = getParentItem(_item);
    }

    // change actions activation
    m_ac->action(CUT_ACTION_NAME)->setEnabled(selected);
    m_ac->action(COPY_ACTION_NAME)->setEnabled(selected);
    m_ac->action(PASTE_ACTION_NAME)->setEnabled(m_clipboard != 0);

    if (m_ac->action(DELETE_ACTION_NAME)) {
        m_ac->action(DELETE_ACTION_NAME)->setEnabled(selected && !dselected);
    }

    m_ac->action(SORT_BY_NAME_ACTION_NAME)->setEnabled(selected && _item && _item->isDirectory() && (_item->childCount() > 0));
    m_ac->action(SORT_BY_DESCRIPTION_ACTION_NAME)->setEnabled(m_ac->action(SORT_BY_NAME_ACTION_NAME)->isEnabled());

    m_ac->action(MOVE_UP_ACTION_NAME)->setEnabled(_item && selected && (parentItem->indexOfChild(_item) > 0));
    m_ac->action(MOVE_DOWN_ACTION_NAME)->setEnabled(_item && selected && (parentItem->indexOfChild(_item) < parentItem->childCount() - 1));

    if (!item) {
        emit disableAction();
        return;
    }

    if (_item) {
        if (_item->isDirectory()) {
            emit entrySelected(_item->folderInfo());
        } else {
            emit entrySelected(_item->entryInfo());
        }
    }
}

void TreeView::currentDataChanged(MenuFolderInfo *folderInfo)
{
    TreeItem *item = (TreeItem *)selectedItem();
    if (!item || !folderInfo) {
        return;
    }

    item->setName(folderInfo->caption);
    item->setIcon(0, QIcon::fromTheme(folderInfo->icon));
}

void TreeView::currentDataChanged(MenuEntryInfo *entryInfo)
{
    TreeItem *item = (TreeItem *)selectedItem();
    if (!item || !entryInfo) {
        return;
    }

    QString name;

    if (m_detailedMenuEntries && !entryInfo->description.isEmpty()) {
        if (m_detailedEntriesNamesFirst) {
            name = entryInfo->caption + QStringLiteral(" (") + entryInfo->description + QLatin1Char(')');
        } else {
            name = entryInfo->description + QStringLiteral(" (") + entryInfo->caption + QLatin1Char(')');
        }
    } else {
        name = entryInfo->caption;
    }

    item->setName(name);
    item->setIcon(0, QIcon::fromTheme(entryInfo->icon));
}

Qt::DropActions TreeView::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList TreeView::mimeTypes() const
{
    return m_dropMimeTypes;
}

void TreeView::startDrag(Qt::DropActions supportedActions)
{
    QList<QTreeWidgetItem *> items;
    items << selectedItem();
    QMimeData *data = mimeData(items);
    if (!data) {
        return;
    }

    QDrag *drag = new QDrag(this);
    const int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
    drag->setPixmap(selectedItem()->icon(0).pixmap(iconSize, iconSize));
    drag->setMimeData(data);
    drag->exec(supportedActions, Qt::MoveAction);
}

QMimeData *TreeView::mimeData(const QList<QTreeWidgetItem *> &items) const
{
    if (items.isEmpty()) {
        return nullptr;
    }

    return new MenuItemMimeData(dynamic_cast<TreeItem *>(items.first()));
}

static QString createDesktopFile(const QString &file, QString *menuId, QStringList *excludeList)
{
    QString base = file.mid(file.lastIndexOf(QLatin1Char('/')) + 1);
    base = base.left(base.lastIndexOf(QLatin1Char('.')));

    const QRegularExpression re(QStringLiteral("(.*)(?=-\\d+)"));
    const QRegularExpressionMatch match = re.match(base);
    base = match.hasMatch() ? match.captured(1) : base;

    QString result = KService::newServicePath(true, base, menuId, excludeList);
    excludeList->append(*menuId);
    // Todo for Undo-support: Undo menuId allocation:

    return result;
}

static KDesktopFile *copyDesktopFile(MenuEntryInfo *entryInfo, QString *menuId, QStringList *excludeList)
{
    QString result = createDesktopFile(entryInfo->file(), menuId, excludeList);
    KDesktopFile *df = entryInfo->desktopFile()->copyTo(result);
    df->desktopGroup().deleteEntry("Categories"); // Don't set any categories!

    return df;
}

static QString createDirectoryFile(const QString &file, QStringList *excludeList)
{
    QString base = file.mid(file.lastIndexOf(QLatin1Char('/')) + 1);
    base = base.left(base.lastIndexOf(QLatin1Char('.')));

    QString result;
    int i = 1;
    while (true) {
        if (i == 1) {
            result = base + QStringLiteral(".directory");
        } else {
            result = base + QStringLiteral("-%1.directory").arg(i);
        }

        if (!excludeList->contains(result)) {
            if (QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("desktop-directories/") + result).isEmpty()) {
                break;
            }
        }
        i++;
    }
    excludeList->append(result);
    result = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/desktop-directories/") + result;
    return result;
}

bool TreeView::dropMimeData(QTreeWidgetItem *item, int index, const QMimeData *data, Qt::DropAction action)
{
    // get destination folder
    TreeItem *titem = item ? dynamic_cast<TreeItem *>(item) : nullptr;
    if (item && !titem) {
        return false;
    }

    TreeItem *parentItem = nullptr;
    QTreeWidgetItem *after = titem;
    // find the parent item and which item the dropped item should go after
    if (titem) {
        if (titem->isDirectory()) {
            parentItem = titem;
            after = titem->child(index);
            if (!after) {
                after = titem->child(titem->childCount() - 1);
            }
        } else {
            parentItem = dynamic_cast<TreeItem *>(titem->parent());
            if (titem->parent() && !parentItem) {
                return false;
            }
        }
    } else if (index > 0) {
        after = topLevelItem(index);
        if (!after) {
            after = topLevelItem(topLevelItemCount() - 1);
        }
    }

    QString folder = parentItem ? parentItem->directory() : QStringLiteral("/");
    MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
    // qCDebug(KMENUEDIT_LOG) << "think we're dropping on" << (parentItem ? parentItem->text(0) : "Top Level") <<  index;

    if (!data->hasFormat(QLatin1String(s_internalMimeType))) {
        // External drop
        if (!data->hasUrls()) {
            return false;
        }

        QList<QUrl> urls = KUrlMimeData::urlsFromMimeData(data);
        if (urls.isEmpty() || !urls[0].isLocalFile()) {
            return false;
        }

        // FIXME: this should really support multiple DnD
        QString path = urls[0].path();
        if (!path.endsWith(QLatin1String(".desktop"))) {
            return false;
        }

        QString menuId;
        QString result = createDesktopFile(path, &menuId, &m_newMenuIds);
        KDesktopFile orig_df(path);
        KDesktopFile *df = orig_df.copyTo(result);
        df->desktopGroup().deleteEntry("Categories"); // Don't set any categories!

        KService::Ptr s(new KService(df));
        s->setMenuId(menuId);

        MenuEntryInfo *entryInfo = new MenuEntryInfo(s, df);

        QString oldCaption = entryInfo->caption;
        QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption, oldCaption);
        entryInfo->setCaption(newCaption);

        // Add file to menu
        // m_menuFile->addEntry(folder, menuId);
        m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

        // create the TreeItem
        if (parentItem) {
            parentItem->setExpanded(true);
        }

        // update fileInfo data
        parentFolderInfo->add(entryInfo);

        TreeItem *newItem = createTreeItem(parentItem, after, entryInfo, true);
        setCurrentItem(newItem);

        setLayoutDirty(parentItem);
        return true;
    }

    QVariant p(data->data(QLatin1String(s_internalMimeType)));
    const MenuItemMimeData *itemData = dynamic_cast<const MenuItemMimeData *>(data);
    if (!itemData) {
        return false;
    }

    TreeItem *dragItem = itemData->item();
    if (!dragItem || dragItem == after) {
        return false; // Nothing to do
    }

    ////qCDebug(KMENUEDIT_LOG) << "an internal drag of" << dragItem->text(0) << (parentItem ? parentItem->text(0) : "Top level");
    if (dragItem->isDirectory()) {
        MenuFolderInfo *folderInfo = dragItem->folderInfo();
        if (action == Qt::CopyAction) {
            // FIXME:
            // * Create new .directory file
        } else {
            TreeItem *tmpItem = static_cast<TreeItem *>(parentItem);
            while (tmpItem) {
                if (tmpItem == dragItem) {
                    return false;
                }

                tmpItem = static_cast<TreeItem *>(tmpItem->parent());
            }

            // Remove MenuFolderInfo
            TreeItem *oldParentItem = static_cast<TreeItem *>(dragItem->parent());
            MenuFolderInfo *oldParentFolderInfo = oldParentItem ? oldParentItem->folderInfo() : m_rootFolder;
            oldParentFolderInfo->take(folderInfo);

            // Move menu
            QString oldFolder = folderInfo->fullId;
            QString folderName = folderInfo->id;
            QString newFolder = m_menuFile->uniqueMenuName(folder, folderName, parentFolderInfo->existingMenuIds());
            folderInfo->id = newFolder;

            // Add file to menu
            // m_menuFile->moveMenu(oldFolder, folder + newFolder);
            // qCDebug(KMENUEDIT_LOG) << "moving" << dragItem->text(0) << "to" << folder + newFolder;
            m_menuFile->pushAction(MenuFile::MOVE_MENU, oldFolder, folder + newFolder);

            // Make sure caption is unique
            QString newCaption = parentFolderInfo->uniqueMenuCaption(folderInfo->caption);
            if (newCaption != folderInfo->caption) {
                folderInfo->setCaption(newCaption);
            }

            // create the TreeItem
            if (parentItem) {
                parentItem->setExpanded(true);
            }

            // update fileInfo data
            folderInfo->updateFullId(parentFolderInfo->fullId);
            folderInfo->setInUse(true);
            parentFolderInfo->add(folderInfo);

            if (parentItem != oldParentItem) {
                if (oldParentItem) {
                    oldParentItem->takeChild(oldParentItem->indexOfChild(dragItem));
                } else {
                    takeTopLevelItem(indexOfTopLevelItem(dragItem));
                }
            }

            if (parentItem) {
                parentItem->insertChild(after ? parentItem->indexOfChild(after) + 1 : parentItem->childCount(), dragItem);
            } else {
                insertTopLevelItem(after ? indexOfTopLevelItem(after) : topLevelItemCount(), dragItem);
            }

            dragItem->setName(folderInfo->caption);
            dragItem->setDirectoryPath(folderInfo->fullId);
            setCurrentItem(dragItem);
        }
    } else if (dragItem->isEntry()) {
        MenuEntryInfo *entryInfo = dragItem->entryInfo();
        QString menuId = entryInfo->menuId();

        if (action == Qt::CopyAction) {
            // Need to copy file and then add it
            KDesktopFile *df = copyDesktopFile(entryInfo, &menuId, &m_newMenuIds); // Duplicate
            // UNDO-ACTION: NEW_MENU_ID (menuId)

            KService::Ptr s(new KService(df));
            s->setMenuId(menuId);

            entryInfo = new MenuEntryInfo(s, df);

            QString oldCaption = entryInfo->caption;
            QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption, oldCaption);
            entryInfo->setCaption(newCaption);
        } else {
            del(dragItem, false);
            QString oldCaption = entryInfo->caption;
            QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption);
            entryInfo->setCaption(newCaption);
            entryInfo->setInUse(true);
        }

        // Add file to menu
        // m_menuFile->addEntry(folder, menuId);
        m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

        // create the TreeItem
        if (parentItem) {
            parentItem->setExpanded(true);
        }

        // update fileInfo data
        parentFolderInfo->add(entryInfo);

        TreeItem *newItem = createTreeItem(parentItem, after, entryInfo);
        setCurrentItem(newItem);

        // clipboard no longer owns entryInfo
        if (m_clipboardEntryInfo == entryInfo) {
            m_clipboard = COPY_FILE;
        }
    } else {
        // copying a separator
        if (action != Qt::CopyAction) {
            del(dragItem, false);
        }

        TreeItem *newItem = createTreeItem(parentItem, after, m_separator);
        setCurrentItem(newItem);
    }

    // qCDebug(KMENUEDIT_LOG) << "setting the layout to be dirty at" << parentItem;
    setLayoutDirty(parentItem);
    return true;
}

QTreeWidgetItem *TreeView::selectedItem()
{
    QList<QTreeWidgetItem *> selection = selectedItems();

    if (selection.isEmpty()) {
        return nullptr;
    }

    return selection.first();
}

void TreeView::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_popupMenu && itemAt(event->pos())) {
        m_popupMenu->exec(event->globalPos());
    }
}

void TreeView::dropEvent(QDropEvent *event)
{
    // this prevents QTreeWidget from interfering with our moves
    QTreeView::dropEvent(event);
}

void TreeView::newsubmenu()
{
    TreeItem *parentItem = nullptr;
    TreeItem *item = (TreeItem *)selectedItem();

    bool ok;
    QString caption = QInputDialog::getText(this, i18n("New Submenu"), i18n("Submenu name:"), QLineEdit::Normal, QString(), &ok);

    if (!ok) {
        return;
    }

    QString file = caption;
    file.replace(QLatin1Char('/'), QLatin1Char('-'));

    file = createDirectoryFile(file, &m_newDirectoryList); // Create

    // get destination folder
    QString folder;

    if (!item) {
        parentItem = nullptr;
        folder.clear();
    } else if (item->isDirectory()) {
        parentItem = item;
        item = nullptr;
        folder = parentItem->directory();
    } else {
        parentItem = static_cast<TreeItem *>(item->parent());
        folder = parentItem ? parentItem->directory() : QString();
    }

    MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
    MenuFolderInfo *folderInfo = new MenuFolderInfo();
    folderInfo->caption = parentFolderInfo->uniqueMenuCaption(caption);
    folderInfo->id = m_menuFile->uniqueMenuName(folder, caption, parentFolderInfo->existingMenuIds());
    folderInfo->directoryFile = file;
    folderInfo->icon = QStringLiteral("package");
    folderInfo->hidden = false;
    folderInfo->setDirty();

    KDesktopFile *df = new KDesktopFile(file);
    KConfigGroup desktopGroup = df->desktopGroup();
    desktopGroup.writeEntry("Name", folderInfo->caption);
    desktopGroup.writeEntry("Icon", folderInfo->icon);
    df->sync();
    delete df;
    // Add file to menu
    // m_menuFile->addMenu(folder + folderInfo->id, file);
    m_menuFile->pushAction(MenuFile::ADD_MENU, folder + folderInfo->id, file);

    folderInfo->fullId = parentFolderInfo->fullId + folderInfo->id;

    // create the TreeItem
    if (parentItem) {
        parentItem->setExpanded(true);
    }

    // update fileInfo data
    parentFolderInfo->add(folderInfo);

    TreeItem *newItem = createTreeItem(parentItem, item, folderInfo, true);

    setCurrentItem(newItem);
    setLayoutDirty(parentItem);
}

void TreeView::newitem()
{
    TreeItem *parentItem = nullptr;
    TreeItem *item = (TreeItem *)selectedItem();

    bool ok;
    QString caption = QInputDialog::getText(this, i18n("New Item"), i18n("Item name:"), QLineEdit::Normal, QString(), &ok);

    if (!ok) {
        return;
    }

    QString menuId;
    QString file = caption;
    file.replace(QLatin1Char('/'), QLatin1Char('-'));

    file = createDesktopFile(file, &menuId, &m_newMenuIds); // Create

    KDesktopFile *df = new KDesktopFile(file);
    KConfigGroup desktopGroup = df->desktopGroup();
    desktopGroup.writeEntry("Name", caption);
    desktopGroup.writeEntry("Type", "Application");

    // get destination folder
    QString folder;

    if (!item) {
        parentItem = nullptr;
        folder.clear();
    } else if (item->isDirectory()) {
        parentItem = item;
        item = nullptr;
        folder = parentItem->directory();
    } else {
        parentItem = static_cast<TreeItem *>(item->parent());
        folder = parentItem ? parentItem->directory() : QString();
    }

    MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;

    // Add file to menu
    // m_menuFile->addEntry(folder, menuId);
    m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

    KService::Ptr s(new KService(df));
    s->setMenuId(menuId);

    MenuEntryInfo *entryInfo = new MenuEntryInfo(s, df);

    // create the TreeItem
    if (parentItem) {
        parentItem->setExpanded(true);
    }

    // update fileInfo data
    parentFolderInfo->add(entryInfo);

    TreeItem *newItem = createTreeItem(parentItem, item, entryInfo, true);

    setCurrentItem(newItem);
    setLayoutDirty(parentItem);
}

void TreeView::newsep()
{
    TreeItem *parentItem = nullptr;
    TreeItem *item = (TreeItem *)selectedItem();

    if (!item) {
        parentItem = nullptr;
    } else if (item->isDirectory()) {
        parentItem = item;
        item = nullptr;
    } else {
        parentItem = static_cast<TreeItem *>(item->parent());
    }

    // create the TreeItem
    if (parentItem) {
        parentItem->setExpanded(true);
    }

    TreeItem *newItem = createTreeItem(parentItem, item, m_separator, true);

    setCurrentItem(newItem);
    setLayoutDirty(parentItem);
}

void TreeView::cut()
{
    copy(true);

    // Select new current item
    // TODO: is this completely redundant?
    setCurrentItem(currentItem());
}

void TreeView::copy()
{
    copy(false);
}

void TreeView::copy(bool cutting)
{
    TreeItem *item = (TreeItem *)selectedItem();

    // nil selected? -> nil to copy
    if (item == nullptr) {
        return;
    }

    if (cutting) {
        setLayoutDirty((TreeItem *)item->parent());
    }

    // clean up old stuff
    cleanupClipboard();

    // is item a folder or a file?
    if (item->isDirectory()) {
        QString folder = item->directory();
        if (cutting) {
            // Place in clipboard
            m_clipboard = MOVE_FOLDER;
            m_clipboardFolderInfo = item->folderInfo();

            del(item, false);
        } else {
            // Place in clipboard
            m_clipboard = COPY_FOLDER;
            m_clipboardFolderInfo = item->folderInfo();
        }
    } else if (item->isEntry()) {
        if (cutting) {
            // Place in clipboard
            m_clipboard = MOVE_FILE;
            m_clipboardEntryInfo = item->entryInfo();

            del(item, false);
        } else {
            // Place in clipboard
            m_clipboard = COPY_FILE;
            m_clipboardEntryInfo = item->entryInfo();
        }
    } else {
        // Place in clipboard
        m_clipboard = COPY_SEPARATOR;
        if (cutting) {
            del(item, false);
        }
    }

    m_ac->action(PASTE_ACTION_NAME)->setEnabled(true);
}

void TreeView::paste()
{
    TreeItem *parentItem = nullptr;
    TreeItem *item = (TreeItem *)selectedItem();

    // nil selected? -> nil to paste to
    if (item == nullptr) {
        return;
    }

    // is there content in the clipboard?
    if (!m_clipboard) {
        return;
    }

    // get destination folder
    QString folder;

    if (item->isDirectory()) {
        parentItem = item;
        item = nullptr;
        folder = parentItem->directory();
    } else {
        parentItem = static_cast<TreeItem *>(item->parent());
        folder = parentItem ? parentItem->directory() : QString();
    }

    MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
    int command = m_clipboard;
    if ((command == COPY_FOLDER) || (command == MOVE_FOLDER)) {
        MenuFolderInfo *folderInfo = m_clipboardFolderInfo;
        if (command == COPY_FOLDER) {
            // Ugh.. this is hard :)
            // * Create new .directory file
            // Add
        } else if (command == MOVE_FOLDER) {
            // Move menu
            QString oldFolder = folderInfo->fullId;
            QString folderName = folderInfo->id;
            QString newFolder = m_menuFile->uniqueMenuName(folder, folderName, parentFolderInfo->existingMenuIds());
            folderInfo->id = newFolder;

            // Add file to menu
            // m_menuFile->moveMenu(oldFolder, folder + newFolder);
            m_menuFile->pushAction(MenuFile::MOVE_MENU, oldFolder, folder + newFolder);

            // Make sure caption is unique
            QString newCaption = parentFolderInfo->uniqueMenuCaption(folderInfo->caption);
            if (newCaption != folderInfo->caption) {
                folderInfo->setCaption(newCaption);
            }
            // create the TreeItem
            if (parentItem) {
                parentItem->setExpanded(true);
            }

            // update fileInfo data
            folderInfo->fullId = parentFolderInfo->fullId + folderInfo->id;
            folderInfo->setInUse(true);
            parentFolderInfo->add(folderInfo);

            TreeItem *newItem = createTreeItem(parentItem, item, folderInfo);

            setCurrentItem(newItem);
        }

        m_clipboard = COPY_FOLDER; // Next one copies.
    } else if ((command == COPY_FILE) || (command == MOVE_FILE)) {
        MenuEntryInfo *entryInfo = m_clipboardEntryInfo;
        QString menuId;

        if (command == COPY_FILE) {
            // Need to copy file and then add it
            KDesktopFile *df = copyDesktopFile(entryInfo, &menuId, &m_newMenuIds); // Duplicate

            KService::Ptr s(new KService(df));
            s->setMenuId(menuId);
            entryInfo = new MenuEntryInfo(s, df);

            QString oldCaption = entryInfo->caption;
            QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption, oldCaption);
            entryInfo->setCaption(newCaption);
        } else if (command == MOVE_FILE) {
            menuId = entryInfo->menuId();
            m_clipboard = COPY_FILE; // Next one copies.

            QString oldCaption = entryInfo->caption;
            QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption);
            entryInfo->setCaption(newCaption);
            entryInfo->setInUse(true);
        }
        // Add file to menu
        // m_menuFile->addEntry(folder, menuId);
        m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

        // create the TreeItem
        if (parentItem) {
            parentItem->setExpanded(true);
        }

        // update fileInfo data
        parentFolderInfo->add(entryInfo);

        TreeItem *newItem = createTreeItem(parentItem, item, entryInfo, true);

        setCurrentItem(newItem);
    } else {
        // create separator
        if (parentItem) {
            parentItem->setExpanded(true);
        }

        TreeItem *newItem = createTreeItem(parentItem, item, m_separator, true);

        setCurrentItem(newItem);
    }
    setLayoutDirty(parentItem);
}

/**
 * This slot is called from the signal mapper to sort children contained in an item.
 * This item is determined according to the chosen sort type.
 *
 * @brief Determine which item is to sort, and do it.
 * @param sortCmd Sort type.
 */
void TreeView::sort(const int sortCmd)
{
    // determine the chosen sort type and the selected item
    SortType sortType = static_cast<SortType>(sortCmd);
    TreeItem *itemToSort;
    if (sortType == SortByName || sortType == SortByDescription) {
        itemToSort = static_cast<TreeItem *>(selectedItem());
        sortItem(itemToSort, sortType);
    } else if (sortType == SortAllByDescription) {
        sortType = SortByDescription;
        for (int i = 0; i < topLevelItemCount(); ++i) {
            itemToSort = static_cast<TreeItem *>(topLevelItem(i));
            sortItem(itemToSort, sortType);
        }
    } else { /* if (sortType == SortAllByName) */
        sortType = SortByName;
        for (int i = 0; i < topLevelItemCount(); ++i) {
            itemToSort = static_cast<TreeItem *>(topLevelItem(i));
            sortItem(itemToSort, sortType);
        }
    }
}

/**
 * Sort children of the given item, according to the sort type.
 * The sorting is done on children groups, splited by separator items.
 *
 * @brief Sort item children.
 * @param item Item to sort.
 * @param sortType Sort type.
 */
void TreeView::sortItem(TreeItem *item, SortType sortType)
{
    // sort the selected item only if contains children
    if ((!item->isDirectory()) || (item->childCount() == 0)) {
        return;
    }

    // remove contained children
    QList<QTreeWidgetItem *> children = item->takeChildren();

    // sort children groups, splited by separator items
    QList<QTreeWidgetItem *>::iterator startIt = children.begin();
    QList<QTreeWidgetItem *>::iterator currentIt = children.begin();
    while (currentIt != children.end()) {
        TreeItem *child = static_cast<TreeItem *>(*currentIt);
        // if it's a separator, sort previous items and continue on following items
        if (child->isSeparator() && startIt != currentIt) {
            sortItemChildren(startIt, currentIt, sortType);
            startIt = currentIt + 1;
        }
        ++currentIt;
    }
    sortItemChildren(startIt, currentIt, sortType);

    // insert sorted children in the tree
    item->addChildren(children);
    foreach (QTreeWidgetItem *child, children) {
        // recreate item widget for separators
        TreeItem *treeItem = static_cast<TreeItem *>(child);
        if (treeItem->isSeparator()) {
            setItemWidget(treeItem, 0, new SeparatorWidget);
        }

        // try to sort sub-children
        sortItem(static_cast<TreeItem *>(child), sortType);
    }

    // flag current item as dirty
    TreeItem *itemToFlagAsDirty = item;
    // if tree root item, set the entire layout as dirty
    if (item == invisibleRootItem()) {
        itemToFlagAsDirty = nullptr;
    }
    setLayoutDirty(itemToFlagAsDirty);
}

/**
 * Sort a children range defined with two list iterators, according to the sort type.
 *
 * @brief Sort a children range.
 * @param begin First child iterator.
 * @param end Last child iterator (exclusive, pointed child won't be affected).
 * @param sortType Sort type.
 */
void TreeView::sortItemChildren(const QList<QTreeWidgetItem *>::iterator &begin, const QList<QTreeWidgetItem *>::iterator &end, SortType sortType)
{
    // sort by name
    if (sortType == SortByName) {
        std::sort(begin, end, TreeItem::itemNameLessThan);
    }
    // sort by description
    else if (sortType == SortByDescription) {
        std::sort(begin, end, TreeItem::itemDescriptionLessThan);
    }
}

/**
 * @brief Move up the selected item.
 */
void TreeView::moveUpItem()
{
    moveUpOrDownItem(true);
}

/**
 * @brief Move down the selected item.
 */
void TreeView::moveDownItem()
{
    moveUpOrDownItem(false);
}

/**
 * Move the selected item on desired direction (up or down).
 *
 * @brief Move up/down the selected item.
 * @param isMovingUpAction True to move up, false to move down.
 */
void TreeView::moveUpOrDownItem(bool isMovingUpAction)
{
    // get the selected item and its parent
    TreeItem *sourceItem = static_cast<TreeItem *>(selectedItem());
    if (!sourceItem) {
        return;
    }
    TreeItem *parentItem = getParentItem(sourceItem);

    // get selected item index
    int sourceItemIndex = parentItem->indexOfChild(sourceItem);

    // find the second item to swap
    TreeItem *destItem = nullptr;
    int destIndex;
    if (isMovingUpAction) {
        destIndex = sourceItemIndex - 1;
        destItem = static_cast<TreeItem *>(parentItem->child(destIndex));
    } else {
        destIndex = sourceItemIndex + 1;
        destItem = static_cast<TreeItem *>(parentItem->child(destIndex));
    }

    // swap items
    parentItem->removeChild(sourceItem);
    parentItem->insertChild(destIndex, sourceItem);

    // recreate item widget for separators
    if (sourceItem->isSeparator()) {
        setItemWidget(sourceItem, 0, new SeparatorWidget);
    }
    if (destItem->isSeparator()) {
        setItemWidget(destItem, 0, new SeparatorWidget);
    }

    // set the focus on the source item
    setCurrentItem(sourceItem);

    // flag parent item as dirty (if the parent is the root item, set the entire layout as dirty)
    if (parentItem == invisibleRootItem()) {
        parentItem = nullptr;
    }
    setLayoutDirty(parentItem);
}

/**
 * For a given item, return its parent. For top items, return the invisible root item.
 *
 * @brief Get the parent item.
 * @param item Item.
 * @return Parent item.
 */
TreeItem *TreeView::getParentItem(QTreeWidgetItem *item) const
{
    QTreeWidgetItem *parentItem = item->parent();
    if (!parentItem) {
        parentItem = invisibleRootItem();
    }
    return static_cast<TreeItem *>(parentItem);
}

void TreeView::del()
{
    TreeItem *item = (TreeItem *)selectedItem();

    // nil selected? -> nil to delete
    if (item == nullptr) {
        return;
    }

    del(item, true);

    // Select new current item
    // TODO: is this completely redundant?
    setCurrentItem(currentItem());
}

void TreeView::del(TreeItem *item, bool deleteInfo)
{
    TreeItem *parentItem = static_cast<TreeItem *>(item->parent());
    // is file a .directory or a .desktop file
    if (item->isDirectory()) {
        MenuFolderInfo *folderInfo = item->folderInfo();

        // Remove MenuFolderInfo
        MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
        parentFolderInfo->take(folderInfo);
        folderInfo->setInUse(false);

        if (m_clipboard == COPY_FOLDER && (m_clipboardFolderInfo == folderInfo)) {
            // Copy + Del == Cut
            m_clipboard = MOVE_FOLDER; // Clipboard now owns folderInfo
        } else {
            if (folderInfo->takeRecursive(m_clipboardFolderInfo)) {
                m_clipboard = MOVE_FOLDER; // Clipboard now owns m_clipboardFolderInfo
            }
            if (deleteInfo) {
                delete folderInfo; // Delete folderInfo
            }
        }

        // Remove from menu
        // m_menuFile->removeMenu(item->directory());
        m_menuFile->pushAction(MenuFile::REMOVE_MENU, item->directory(), QString());

        // Remove tree item
        delete item;
    } else if (item->isEntry()) {
        MenuEntryInfo *entryInfo = item->entryInfo();
        QString menuId = entryInfo->menuId();

        // Remove MenuFolderInfo
        MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
        parentFolderInfo->take(entryInfo);
        entryInfo->setInUse(false);

        if (m_clipboard == COPY_FILE && (m_clipboardEntryInfo == entryInfo)) {
            // Copy + Del == Cut
            m_clipboard = MOVE_FILE; // Clipboard now owns entryInfo
        } else {
            if (deleteInfo) {
                delete entryInfo; // Delete entryInfo
            }
        }

        // Remove from menu
        QString folder = parentItem ? parentItem->directory() : QString();
        // m_menuFile->removeEntry(folder, menuId);
        m_menuFile->pushAction(MenuFile::REMOVE_ENTRY, folder, menuId);

        // Remove tree item
        delete item;
    } else {
        // Remove separator
        delete item;
    }

    setLayoutDirty(parentItem);
}

void TreeView::cleanupClipboard()
{
    if (m_clipboard == MOVE_FOLDER) {
        delete m_clipboardFolderInfo;
    }
    m_clipboardFolderInfo = nullptr;

    if (m_clipboard == MOVE_FILE) {
        delete m_clipboardEntryInfo;
    }
    m_clipboardEntryInfo = nullptr;

    m_clipboard = 0;
}

static QStringList extractLayout(QTreeWidget *tree, QTreeWidgetItem *parent)
{
    QStringList layout;
    if (!parent && !tree) {
        return layout;
    }

    bool firstFolder = true;
    bool firstEntry = true;
    int max = parent ? parent->childCount() : tree->topLevelItemCount();
    for (int i = 0; i < max; ++i) {
        TreeItem *item = dynamic_cast<TreeItem *>(parent ? parent->child(i) : tree->topLevelItem(i));
        if (!item) {
            continue;
        }

        if (item->isDirectory()) {
            if (firstFolder) {
                firstFolder = false;
                layout << QStringLiteral(":M"); // Add new folders here...
            }
            layout << (item->folderInfo()->id);
        } else if (item->isEntry()) {
            if (firstEntry) {
                firstEntry = false;
                layout << QStringLiteral(":F"); // Add new entries here...
            }
            layout << (item->entryInfo()->menuId());
        } else {
            layout << QStringLiteral(":S");
        }
    }

    return layout;
}

void TreeItem::saveLayout(MenuFile *menuFile)
{
    if (m_layoutDirty) {
        QStringList layout = extractLayout(nullptr, this);
        menuFile->setLayout(folderInfo()->fullId, layout);
        m_layoutDirty = false;
    }

    for (int i = 0; i < childCount(); ++i) {
        TreeItem *item = dynamic_cast<TreeItem *>(child(i));
        if (item) {
            item->saveLayout(menuFile);
        }
    }
}

void TreeView::saveLayout()
{
    if (m_layoutDirty) {
        QStringList layout = extractLayout(this, nullptr);
        m_menuFile->setLayout(m_rootFolder->fullId, layout);
        m_layoutDirty = false;
    }

    for (int i = 0; i < topLevelItemCount(); ++i) {
        TreeItem *item = dynamic_cast<TreeItem *>(topLevelItem(i));
        if (item) {
            item->saveLayout(m_menuFile);
        }
    }
}

bool TreeView::save()
{
    saveLayout();
    m_rootFolder->save(m_menuFile);

    bool success = m_menuFile->performAllActions();

    m_newMenuIds.clear();
    m_newDirectoryList.clear();

    if (success) {
        KBuildSycocaProgressDialog::rebuildKSycoca(this);
    } else {
        KMessageBox::error(this,
                           QStringLiteral("<qt>") + i18n("Menu changes could not be saved because of the following problem:") + QStringLiteral("<br><br>")
                               + m_menuFile->error() + QStringLiteral("</qt>"));
    }

    sendReloadMenu();

    return success;
}

void TreeView::setLayoutDirty(TreeItem *parentItem)
{
    if (parentItem) {
        parentItem->setLayoutDirty();
    } else {
        m_layoutDirty = true;
    }
}

bool TreeView::isLayoutDirty()
{
    for (int i = 0; i < topLevelItemCount(); ++i) {
        TreeItem *item = dynamic_cast<TreeItem *>(topLevelItem(i));
        if (!item) {
            continue;
        }

        if (item->isLayoutDirty()) {
            return true;
        }
    }

    return false;
}

bool TreeView::dirty()
{
    return m_layoutDirty || m_rootFolder->hasDirt() || m_menuFile->dirty() || isLayoutDirty();
}

void TreeView::findServiceShortcut(const QKeySequence &cut, KService::Ptr &service)
{
    service = m_rootFolder->findServiceShortcut(cut);
}

void TreeView::restoreMenuSystem()
{
    if (KMessageBox::questionTwoActions(this,
                                        i18n("Do you want to restore the system menu? Warning: This will remove all custom menus."),
                                        i18nc("@title:window", "Restore Menu System"),
                                        KStandardGuiItem::reset(),
                                        KStandardGuiItem::cancel())
        == KMessageBox::SecondaryAction) {
        return;
    }
    const QString kmenueditfile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/menus/applications-kmenuedit.menu");
    if (QFile::exists(kmenueditfile)) {
        if (!QFile::remove(kmenueditfile)) {
            qCWarning(KMENUEDIT_LOG) << "Could not delete " << kmenueditfile;
        }
    }

    const QString xdgappsdir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/applications");
    if (QFileInfo(xdgappsdir).isDir()) {
        if (!QDir(xdgappsdir).removeRecursively()) {
            qCWarning(KMENUEDIT_LOG) << "Could not delete dir :" << xdgappsdir;
        }
    }
    const QString xdgdesktopdir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/desktop-directories");
    if (QFileInfo(xdgdesktopdir).isDir()) {
        if (!QDir(xdgdesktopdir).removeRecursively()) {
            qCWarning(KMENUEDIT_LOG) << "Could not delete dir :" << xdgdesktopdir;
        }
    }

    KBuildSycocaProgressDialog::rebuildKSycoca(this);
    clear();
    cleanupClipboard();
    delete m_rootFolder;
    delete m_separator;

    m_layoutDirty = false;
    m_newMenuIds.clear();
    m_newDirectoryList.clear();
    m_menuFile->restoreMenuSystem(kmenueditfile);

    m_rootFolder = new MenuFolderInfo;
    m_separator = new MenuSeparatorInfo;

    readMenuFolderInfo();
    fill();
    sendReloadMenu();
    emit disableAction();
    emit entrySelected((MenuEntryInfo *)nullptr);
}

void TreeView::updateTreeView(bool showHidden)
{
    m_showHidden = showHidden;
    clear();
    cleanupClipboard();
    delete m_rootFolder;
    delete m_separator;

    m_layoutDirty = false;
    m_newMenuIds.clear();
    m_newDirectoryList.clear();

    m_rootFolder = new MenuFolderInfo;
    m_separator = new MenuSeparatorInfo;

    readMenuFolderInfo();
    fill();
    sendReloadMenu();
    emit disableAction();
    emit entrySelected((MenuEntryInfo *)nullptr);
}

void TreeView::sendReloadMenu()
{
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/kickoff"), QStringLiteral("org.kde.plasma"), QStringLiteral("reloadMenu"));
    QDBusConnection::sessionBus().send(message);
}

// Slot to expand or retract items in tree based on length of search string
void TreeView::searchUpdated(const QString &searchString)
{
    // expand all categories if we typed more than a few characters, otherwise collapse and un-select everything
    // use logicalLength for CJK users
    if (KStringHandler::logicalLength(searchString) > 2) {
        expandAll();
    } else {
        collapseAll();
        setCurrentIndex(rootIndex());
    }
}

MenuItemMimeData::MenuItemMimeData(TreeItem *item)
    : QMimeData()
    , m_item(item)
{
}

TreeItem *MenuItemMimeData::item() const
{
    return m_item;
}

QStringList MenuItemMimeData::formats() const
{
    QStringList formats;
    if (!m_item) {
        return formats;
    }

    formats << QLatin1String(s_internalMimeType);
    return formats;
}

bool MenuItemMimeData::hasFormat(const QString &mimeType) const
{
    return m_item && mimeType == QLatin1String(s_internalMimeType);
}

QVariant MenuItemMimeData::retrieveData(const QString &mimeType, QMetaType type) const
{
    Q_UNUSED(type);

    if (m_item && mimeType == QLatin1String(s_internalMimeType)) {
        return QVariant::fromValue<TreeItem *>(m_item);
    }

    return QVariant();
}
