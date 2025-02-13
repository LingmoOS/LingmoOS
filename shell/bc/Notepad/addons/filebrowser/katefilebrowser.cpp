/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
   SPDX-FileCopyrightText: 2009 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

// BEGIN Includes
#include "katefilebrowser.h"

#include "katebookmarkhandler.h"

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <KActionCollection>
#include <KActionMenu>
#include <KApplicationTrader>
#include <KConfigGroup>
#include <KDirOperator>
#include <KFilePlacesModel>
#include <KHistoryComboBox>
#include <KIO/ApplicationLauncherJob>
#include <kio_version.h>
#include <kwidgetsaddons_version.h>
#if KIO_VERSION >= QT_VERSION_CHECK(5, 98, 0)
#include <KIO/JobUiDelegateFactory>
#else
#include <KIO/JobUiDelegate>
#endif
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KToolBar>
#include <KUrlNavigator>
#include <kio_version.h>

#include <QAbstractItemView>
#include <QAction>
#include <QDir>
#include <QLineEdit>
#include <QStyle>
#include <QVBoxLayout>

// END Includes

KateFileBrowser::KateFileBrowser(KTextEditor::MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_toolbar = new KToolBar(this);
    m_toolbar->setMovable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->setContextMenuPolicy(Qt::NoContextMenu);
    m_toolbar->layout()->setContentsMargins(0, 0, 0, 0);

    // ensure reasonable icons sizes, like e.g. the quick-open and co. icons
    // the normal toolbar sizes are TOO large, e.g. for scaled stuff even more!
    const int iconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize, nullptr, this);
    m_toolbar->setIconSize(QSize(iconSize, iconSize));

    mainLayout->addWidget(m_toolbar);

    // includes some actions, but not hooked into the shortcut dialog atm
    m_actionCollection = new KActionCollection(this);
    m_actionCollection->addAssociatedWidget(this);

    KFilePlacesModel *model = new KFilePlacesModel(this);
    m_urlNavigator = new KUrlNavigator(model, QUrl::fromLocalFile(QDir::homePath()), this);
    connect(m_urlNavigator, &KUrlNavigator::urlChanged, this, &KateFileBrowser::updateDirOperator);
    mainLayout->addWidget(m_urlNavigator);

    m_dirOperator = new KDirOperator(QUrl(), this);
    // Default to a view with only one column since columns are auto-sized
    m_dirOperator->setView(KFile::Tree);
    m_dirOperator->view()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_dirOperator->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    mainLayout->addWidget(m_dirOperator);

    // Mime filter for the KDirOperator
    QStringList filter;
    filter << QStringLiteral("text/html") << QStringLiteral("inode/directory");
#if KIO_VERSION < QT_VERSION_CHECK(5, 96, 0)
    filter << QStringLiteral("text/plain");
#else
    filter << QStringLiteral("application/x-zerosize");
#endif
    m_dirOperator->setNewFileMenuSupportedMimeTypes(filter);

    setFocusProxy(m_dirOperator);
    connect(m_dirOperator, &KDirOperator::viewChanged, this, &KateFileBrowser::selectorViewChanged);
    connect(m_urlNavigator, &KUrlNavigator::returnPressed, m_dirOperator, static_cast<void (KDirOperator::*)()>(&KDirOperator::setFocus));

    // now all actions exist in dir operator and we can use them in the toolbar
    setupActions();
    setupToolbar();

    m_filter = new KHistoryComboBox(true, this);
    m_filter->setMaxCount(10);
    m_filter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_filter->lineEdit()->setPlaceholderText(i18n("Search"));
    mainLayout->addWidget(m_filter);

    connect(m_filter, &KHistoryComboBox::editTextChanged, this, &KateFileBrowser::slotFilterChange);
    connect(m_filter, static_cast<void (KHistoryComboBox::*)(const QString &)>(&KHistoryComboBox::returnPressed), m_filter, &KHistoryComboBox::addToHistory);
    connect(m_filter,
            static_cast<void (KHistoryComboBox::*)(const QString &)>(&KHistoryComboBox::returnPressed),
            m_dirOperator,
            static_cast<void (KDirOperator::*)()>(&KDirOperator::setFocus));
    connect(m_dirOperator, &KDirOperator::urlEntered, this, &KateFileBrowser::updateUrlNavigator);

    // Connect the bookmark handler
    connect(m_bookmarkHandler, &KateBookmarkHandler::openUrl, this, static_cast<void (KateFileBrowser::*)(const QString &)>(&KateFileBrowser::setDir));

    m_filter->setWhatsThis(i18n("Enter a name filter to limit which files are displayed."));

    connect(m_dirOperator, &KDirOperator::fileSelected, this, &KateFileBrowser::fileSelected);
    connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &KateFileBrowser::autoSyncFolder);

    connect(m_dirOperator, &KDirOperator::contextMenuAboutToShow, this, &KateFileBrowser::contextMenuAboutToShow);

    // Ensure highlight current document also works after directory change
    connect(m_dirOperator, &KDirOperator::finishedLoading, this, [this] {
        if (m_highlightCurrentFile->isChecked() && m_autoSyncFolder->isChecked()) {
            if (const auto u = activeDocumentUrl(); u.isValid()) {
                m_dirOperator->setCurrentItem(u);
            }
        }
    });
}

KateFileBrowser::~KateFileBrowser()
{
}
// END Constructor/Destructor

// BEGIN Public Methods
void KateFileBrowser::setupToolbar()
{
    KConfigGroup config(KSharedConfig::openConfig(), "filebrowser");
    QStringList actions = config.readEntry("toolbar actions", QStringList());
    if (actions.isEmpty()) { // default toolbar
        actions << QStringLiteral("back") << QStringLiteral("forward") << QStringLiteral("bookmarks") << QStringLiteral("sync_dir")
                << QStringLiteral("configure");
    }

    // remove all actions from the toolbar (there should be none)
    m_toolbar->clear();

    // now add all actions to the toolbar
    for (const QString &it : qAsConst(actions)) {
        QAction *ac = nullptr;
        if (it.isEmpty()) {
            continue;
        }
        if (it == QLatin1String("bookmarks") || it == QLatin1String("sync_dir") || it == QLatin1String("configure")) {
            ac = actionCollection()->action(it);
        } else {
#if KIO_VERSION < QT_VERSION_CHECK(5, 100, 0)
            ac = m_dirOperator->actionCollection()->action(it);
#else
            ac = m_dirOperator->action(actionFromName(it));
#endif
        }

        if (ac) {
            m_toolbar->addAction(ac);
        }
    }
}

void KateFileBrowser::readSessionConfig(const KConfigGroup &cg)
{
    m_dirOperator->readConfig(cg);
    m_dirOperator->setView(KFile::Default);

    m_urlNavigator->setLocationUrl(cg.readEntry("location", QUrl::fromLocalFile(QDir::homePath())));
    setDir(cg.readEntry("location", QUrl::fromLocalFile(QDir::homePath())));
    m_autoSyncFolder->setChecked(cg.readEntry("auto sync folder", true));
    m_highlightCurrentFile->setChecked(cg.readEntry("highlight current file", true));
    m_highlightCurrentFile->setEnabled(m_autoSyncFolder->isChecked());
    m_filter->setHistoryItems(cg.readEntry("filter history", QStringList()), true);
}

void KateFileBrowser::writeSessionConfig(KConfigGroup &cg)
{
    m_dirOperator->writeConfig(cg);

    cg.writeEntry("location", m_urlNavigator->locationUrl().url());
    cg.writeEntry("auto sync folder", m_autoSyncFolder->isChecked());
    cg.writeEntry("auto sync folder", m_autoSyncFolder->isChecked());
    cg.writeEntry("highlight current file", m_highlightCurrentFile->isChecked());
    cg.writeEntry("filter history", m_filter->historyItems());
}

#if KIO_VERSION >= QT_VERSION_CHECK(5, 100, 0)
KDirOperator::Action KateFileBrowser::actionFromName(const QString &name)
{
    if (name == QLatin1String("up")) {
        return KDirOperator::Up;
    } else if (name == QLatin1String("back")) {
        return KDirOperator::Back;
    } else if (name == QLatin1String("forward")) {
        return KDirOperator::Forward;
    } else if (name == QLatin1String("home")) {
        return KDirOperator::Home;
    } else if (name == QLatin1String("reload")) {
        return KDirOperator::Reload;
    } else if (name == QLatin1String("mkdir")) {
        return KDirOperator::NewFolder;
    } else if (name == QLatin1String("delete")) {
        return KDirOperator::Delete;
    } else if (name == QLatin1String("short view")) {
        return KDirOperator::ShortView;
    } else if (name == QLatin1String("detailed view")) {
        return KDirOperator::DetailedView;
    } else if (name == QLatin1String("tree view")) {
        return KDirOperator::TreeView;
    } else if (name == QLatin1String("detailed tree view")) {
        return KDirOperator::DetailedTreeView;
    } else if (name == QLatin1String("show hidden")) {
        return KDirOperator::ShowHiddenFiles;
    } else {
        qWarning() << "Unknown KDirOperator action:" << name;
    }

    return {};
}
#endif

// END Public Methods

// BEGIN Public Slots

void KateFileBrowser::slotFilterChange(const QString &nf)
{
    QString f = nf.trimmed();
    const bool empty = f.isEmpty() || f == QLatin1String("*");

    if (empty) {
        m_dirOperator->clearFilter();
    } else {
        m_dirOperator->setNameFilter(f);
    }

    m_dirOperator->updateDir();
}

bool kateFileSelectorIsReadable(const QUrl &url)
{
    if (!url.isLocalFile()) {
        return true; // what else can we say?
    }

    QDir dir(url.toLocalFile());
    return dir.exists();
}

void KateFileBrowser::setDir(const QUrl &u)
{
    QUrl newurl;

    if (!u.isValid()) {
        newurl = QUrl::fromLocalFile(QDir::homePath());
    } else {
        newurl = u;
    }

    QString path(newurl.path());
    if (!path.endsWith(QLatin1Char('/'))) {
        path += QLatin1Char('/');
    }
    newurl.setPath(path);

    if (!kateFileSelectorIsReadable(newurl)) {
        newurl.setPath(newurl.path() + QStringLiteral("../"));
        newurl = newurl.adjusted(QUrl::NormalizePathSegments);
    }

    if (!kateFileSelectorIsReadable(newurl)) {
        newurl = QUrl::fromLocalFile(QDir::homePath());
    }

    m_dirOperator->setUrl(newurl, true);
}

void KateFileBrowser::contextMenuAboutToShow(const KFileItem &item, QMenu *menu)
{
    if (m_openWithMenu == nullptr) {
        m_openWithMenu = new KateFileBrowserOpenWithMenu(i18nc("@action:inmenu", "Open With"), this);
        m_openWithMenu->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
        menu->insertMenu(menu->actions().at(1), m_openWithMenu);
        menu->insertSeparator(menu->actions().at(2));
        connect(m_openWithMenu, &QMenu::aboutToShow, this, &KateFileBrowser::fixOpenWithMenu);
        connect(m_openWithMenu, &QMenu::triggered, this, &KateFileBrowser::openWithMenuAction);
    }
    m_openWithMenu->setItem(item);
}

void KateFileBrowser::fixOpenWithMenu()
{
    KateFileBrowserOpenWithMenu *menu = static_cast<KateFileBrowserOpenWithMenu *>(sender());
    menu->clear();

    // get a list of appropriate services.
    QMimeType mime = menu->item().determineMimeType();

    QAction *a = nullptr;
    const KService::List offers = KApplicationTrader::queryByMimeType(mime.name());
    // for each one, insert a menu item...
    for (const auto &service : offers) {
        if (service->name() == QLatin1String("Kate")) {
            continue;
        }
        a = menu->addAction(QIcon::fromTheme(service->icon()), service->name());
        a->setData(QVariant(QList<QString>({service->entryPath(), menu->item().url().toString()})));
    }
    // append "Other..." to call the KDE "open with" dialog.
    a = menu->addAction(i18n("&Other..."));
    a->setData(QVariant(QList<QString>({QString(), menu->item().url().toString()})));
}

void KateFileBrowser::openWithMenuAction(QAction *a)
{
    const QString application = a->data().toStringList().first();
    const QString fileName = a->data().toStringList().last();
    const QList<QUrl> list({QUrl(fileName)});

    KService::Ptr app = KService::serviceByDesktopPath(application);
    // If app is null, ApplicationLauncherJob will invoke the open-with dialog
    auto *job = new KIO::ApplicationLauncherJob(app);
    job->setUrls(list);
#if KIO_VERSION >= QT_VERSION_CHECK(5, 98, 0)
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
#else
    job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
#endif
    job->start();
}
// END Public Slots

// BEGIN Private Slots

void KateFileBrowser::fileSelected(const KFileItem & /*file*/)
{
    openSelectedFiles();
}

void KateFileBrowser::openSelectedFiles()
{
    const KFileItemList list = m_dirOperator->selectedItems();

    if (list.count() > 20) {
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
        if (KMessageBox::questionTwoActions(
                this,
#else
        if (KMessageBox::questionYesNo(this,
#endif
                i18np("You are trying to open 1 file, are you sure?", "You are trying to open %1 files, are you sure?", list.count()),
                {},
                KGuiItem(i18nc("@action:button", "Open All Files"), QStringLiteral("document-open")),
                KStandardGuiItem::cancel())
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
            == KMessageBox::SecondaryAction) {
#else
            == KMessageBox::No) {
#endif
            return;
        }
    }

    for (const KFileItem &item : list) {
        m_mainWindow->openUrl(item.url());
    }

    m_dirOperator->view()->selectionModel()->clear();
}

void KateFileBrowser::updateDirOperator(const QUrl &u)
{
    m_dirOperator->setUrl(u, true);
}

void KateFileBrowser::updateUrlNavigator(const QUrl &u)
{
    m_urlNavigator->setLocationUrl(u);
}

void KateFileBrowser::setActiveDocumentDir()
{
    QUrl u = activeDocumentUrl();
    if (!u.isEmpty()) {
        setDir(KIO::upUrl(u));
        if (m_highlightCurrentFile->isChecked() && m_autoSyncFolder->isChecked()) {
            m_dirOperator->setCurrentItem(u);
        }
    }
}

void KateFileBrowser::autoSyncFolder()
{
    if (m_autoSyncFolder->isChecked()) {
        setActiveDocumentDir();
    }
}

void KateFileBrowser::selectorViewChanged(QAbstractItemView *newView)
{
    newView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

// END Private Slots

// BEGIN Protected

QUrl KateFileBrowser::activeDocumentUrl()
{
    KTextEditor::View *v = m_mainWindow->activeView();
    if (v) {
        return v->document()->url();
    }
    return QUrl();
}

void KateFileBrowser::setupActions()
{
    // bookmarks action!
    KActionMenu *acmBookmarks = new KActionMenu(QIcon::fromTheme(QStringLiteral("bookmarks")), i18n("Bookmarks"), this);
    acmBookmarks->setPopupMode(QToolButton::InstantPopup);
    m_bookmarkHandler = new KateBookmarkHandler(this, acmBookmarks->menu());
    acmBookmarks->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    // action for synchronizing the dir operator with the current document path
    QAction *syncFolder = new QAction(this);
    syncFolder->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    syncFolder->setText(i18n("Current Document Folder"));
    syncFolder->setIcon(QIcon::fromTheme(QStringLiteral("system-switch-user")));
    connect(syncFolder, &QAction::triggered, this, &KateFileBrowser::setActiveDocumentDir);

    m_actionCollection->addAction(QStringLiteral("sync_dir"), syncFolder);
    m_actionCollection->addAction(QStringLiteral("bookmarks"), acmBookmarks);

    // section for settings menu
    KActionMenu *optionsMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("configure")), i18n("Options"), this);
    optionsMenu->setPopupMode(QToolButton::InstantPopup);
#if KIO_VERSION < QT_VERSION_CHECK(5, 100, 0)
    optionsMenu->addAction(m_dirOperator->actionCollection()->action(QStringLiteral("short view")));
    optionsMenu->addAction(m_dirOperator->actionCollection()->action(QStringLiteral("detailed view")));
    optionsMenu->addAction(m_dirOperator->actionCollection()->action(QStringLiteral("tree view")));
    optionsMenu->addAction(m_dirOperator->actionCollection()->action(QStringLiteral("detailed tree view")));
    optionsMenu->addSeparator();
    optionsMenu->addAction(m_dirOperator->actionCollection()->action(QStringLiteral("show hidden")));
#else
    optionsMenu->addAction(m_dirOperator->action(KDirOperator::ShortView));
    optionsMenu->addAction(m_dirOperator->action(KDirOperator::DetailedView));
    optionsMenu->addAction(m_dirOperator->action(KDirOperator::TreeView));
    optionsMenu->addAction(m_dirOperator->action(KDirOperator::DetailedTreeView));
    optionsMenu->addSeparator();
    optionsMenu->addAction(m_dirOperator->action(KDirOperator::ShowHiddenFiles));
#endif

    // action for synchronising the dir operator with the current document path...
    m_autoSyncFolder = new QAction(this);
    m_autoSyncFolder->setCheckable(true);
    m_autoSyncFolder->setText(i18n("Automatically synchronize with current document"));
    m_autoSyncFolder->setChecked(true);
    m_autoSyncFolder->setIcon(QIcon::fromTheme(QStringLiteral("system-switch-user")));
    optionsMenu->addAction(m_autoSyncFolder);
    // ...and his buddy who depend on him...
    m_highlightCurrentFile = new QAction(this);
    m_highlightCurrentFile->setCheckable(true);
    m_highlightCurrentFile->setText(i18n("Highlight current file"));
    m_highlightCurrentFile->setChecked(true);
    optionsMenu->addAction(m_highlightCurrentFile);
    // ...needs some special handling in case of user action
    connect(m_highlightCurrentFile, &QAction::triggered, this, [this] {
        m_dirOperator->view()->clearSelection();
        autoSyncFolder();
    });
    connect(m_autoSyncFolder, &QAction::triggered, this, [this](bool enabled) {
        m_dirOperator->view()->clearSelection();
        m_highlightCurrentFile->setEnabled(enabled);
        autoSyncFolder();
    });

    m_actionCollection->addAction(QStringLiteral("configure"), optionsMenu);

    //
    // Remove all shortcuts due to shortcut clashes (e.g. F5: reload, Ctrl+B: bookmark)
    // BUGS: #188954, #236368
    //
    const auto actions = m_actionCollection->actions();
    for (QAction *a : actions) {
        a->setShortcut(QKeySequence());
    }
#if KIO_VERSION < QT_VERSION_CHECK(5, 100, 0)
    const auto dirActions = m_dirOperator->actionCollection()->actions();
#else
    const auto dirActions = m_dirOperator->allActions();
#endif
    for (QAction *a : dirActions) {
        a->setShortcut(QKeySequence());
    }
}
// END Protected
