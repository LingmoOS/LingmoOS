/*
    SPDX-FileCopyrightText: 2001, 2002 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2001 Michael Jarrett <michaelj@corel.com>
    SPDX-FileCopyrightText: 2009 Shaun Reich <shaun.reich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kdirselectdialog_p.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QLayout>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>

#include <KActionCollection>
#include <KAuthorized>
#include <KConfig>
#include <KConfigGroup>
#include <KFileItemDelegate>
#include <KFileUtils>
#include <KFileWidget>
#include <KHistoryComboBox>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KIO/DeleteOrTrashJob>
#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KIO/MkdirJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPropertiesDialog>
#include <KRecentDirs>
#include <KService>
#include <KSharedConfig>
#include <KStandardShortcut>
#include <KToggleAction>
#include <KUrlCompletion>

#include "kfiletreeview_p.h"
#include <KFilePlacesModel>
#include <KFilePlacesView>
// ### add mutator for treeview!

class KDirSelectDialog::Private
{
public:
    Private(bool localOnly, KDirSelectDialog *parent)
        : m_parent(parent)
        , m_localOnly(localOnly)
        , m_comboLocked(false)
    {
    }

    void readConfig(const KSharedConfigPtr &config, const QString &group);
    void saveConfig(KSharedConfigPtr config, const QString &group);
    void slotMkdir();

    void slotCurrentChanged(const QUrl &url);
    void slotExpand(const QModelIndex &);
    void slotUrlActivated(const QString &);
    void slotComboTextChanged(const QString &);
    void slotContextMenuRequested(const QPoint &);
    void slotMoveToTrash();
    void slotDelete();
    void slotProperties();

    KDirSelectDialog *const m_parent;
    bool m_localOnly : 1;
    bool m_comboLocked : 1;
    QUrl m_rootUrl;
    QUrl m_startDir;
    KFileTreeView *m_treeView = nullptr;
    QMenu *m_contextMenu = nullptr;
    KActionCollection *m_actions = nullptr;
    KFilePlacesView *m_placesView = nullptr;
    KHistoryComboBox *m_urlCombo = nullptr;
    QString m_recentDirClass;
    QUrl m_startURL;
    QAction *moveToTrash = nullptr;
    QAction *deleteAction = nullptr;
    QAction *showHiddenFoldersAction = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
};

void KDirSelectDialog::Private::readConfig(const KSharedConfig::Ptr &config, const QString &group)
{
    m_urlCombo->clear();

    KConfigGroup conf(config, group);
    m_urlCombo->setHistoryItems(conf.readPathEntry("History Items", QStringList()));

    const QSize size = conf.readEntry("DirSelectDialog Size", QSize());
    if (size.isValid()) {
        m_parent->resize(size);
    }
}

void KDirSelectDialog::Private::saveConfig(KSharedConfig::Ptr config, const QString &group)
{
    KConfigGroup conf(config, group);
    KConfigGroup::WriteConfigFlags flags(KConfigGroup::Persistent | KConfigGroup::Global);
    conf.writePathEntry("History Items", m_urlCombo->historyItems(), flags);
    conf.writeEntry("DirSelectDialog Size", m_parent->size(), flags);

    config->sync();
}

void KDirSelectDialog::Private::slotMkdir()
{
    bool ok;
    QString where = m_parent->url().toDisplayString(QUrl::PreferLocalFile);
    QString name = i18nc("folder name", "New Folder");
    if (m_parent->url().isLocalFile() && QFileInfo::exists(m_parent->url().toLocalFile() + QLatin1Char('/') + name)) {
        name = KFileUtils::suggestName(m_parent->url(), name);
    }

    const QString directory = QInputDialog::getText(m_parent,
                                                    i18nc("@title:window", "New Folder"),
                                                    i18nc("@label:textbox", "Create new folder in:\n%1", where),
                                                    QLineEdit::Normal,
                                                    name,
                                                    &ok);
    if (!ok) {
        return;
    }

    bool writeOk = false;
    bool exists = false;
    QUrl folderurl(m_parent->url());

    const QStringList dirs = directory.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    QStringList::ConstIterator it = dirs.begin();

    for (; it != dirs.end(); ++it) {
        folderurl.setPath(folderurl.path() + QLatin1Char('/') + *it);
        KIO::StatJob *job = KIO::stat(folderurl);
        KJobWidgets::setWindow(job, m_parent);
        job->setDetails(KIO::StatNoDetails); // We only want to know if it exists
        job->setSide(KIO::StatJob::DestinationSide);
        exists = job->exec();
        if (!exists) {
            KIO::MkdirJob *job = KIO::mkdir(folderurl);
            KJobWidgets::setWindow(job, m_parent);
            writeOk = job->exec();
        }
    }

    if (exists) { // url was already existent
        QString which = folderurl.toDisplayString(QUrl::PreferLocalFile);
        KMessageBox::error(m_parent, i18n("A file or folder named %1 already exists.", which));
        // Select the existing dir (if a file with that name exists, it won't be selected since
        // we only show dirs here, this is cheaper than checking if the existing item is a file
        // or folder).
        m_parent->setCurrentUrl(folderurl);
        return;
    }

    if (!writeOk) {
        KMessageBox::error(m_parent, i18n("You do not have permission to create that folder."));
        return;
    }

    // Select the newly created dir
    m_parent->setCurrentUrl(folderurl);
}

void KDirSelectDialog::Private::slotCurrentChanged(const QUrl &url)
{
    if (m_comboLocked) {
        return;
    }

    if (url.isValid()) {
        m_urlCombo->setEditText(url.toDisplayString(QUrl::PreferLocalFile));
    } else {
        m_urlCombo->setEditText(QString());
    }
}

void KDirSelectDialog::Private::slotUrlActivated(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }

    const QUrl url = QUrl::fromUserInput(text);
    m_urlCombo->addToHistory(url.toDisplayString());

    if (m_parent->localOnly() && !url.isLocalFile()) {
        return; // FIXME: messagebox for the user
    }

    QUrl oldUrl = m_treeView->currentUrl();
    if (oldUrl.isEmpty()) {
        oldUrl = m_startDir;
    }

    m_parent->setCurrentUrl(oldUrl);
}

void KDirSelectDialog::Private::slotComboTextChanged(const QString &text)
{
    m_treeView->blockSignals(true);
    QUrl url = QUrl::fromUserInput(text);
#ifdef Q_OS_WIN
    QUrl rootUrl(m_treeView->rootUrl());
    if (url.isLocalFile() && !rootUrl.isParentOf(url) && !rootUrl.matches(url, QUrl::StripTrailingSlash)) {
        QUrl tmp = KIO::upUrl(url);
        while (tmp.path().length() > 1) {
            url = tmp;
            tmp = KIO::upUrl(url);
        }
        m_treeView->setRootUrl(url);
    }
#endif
    m_treeView->setCurrentUrl(url);
    m_treeView->blockSignals(false);
}

void KDirSelectDialog::Private::slotContextMenuRequested(const QPoint &pos)
{
    m_contextMenu->popup(m_treeView->viewport()->mapToGlobal(pos));
}

void KDirSelectDialog::Private::slotExpand(const QModelIndex &index)
{
    m_treeView->setExpanded(index, !m_treeView->isExpanded(index));
}

void KDirSelectDialog::Private::slotMoveToTrash()
{
    const QUrl url = m_treeView->selectedUrl();
    using Iface = KIO::AskUserActionInterface;
    auto *trashJob = new KIO::DeleteOrTrashJob({url}, Iface::Trash, Iface::DefaultConfirmation, m_parent);
    trashJob->start();
}

void KDirSelectDialog::Private::slotDelete()
{
    const QUrl url = m_treeView->selectedUrl();
    using Iface = KIO::AskUserActionInterface;
    auto *deleteJob = new KIO::DeleteOrTrashJob({url}, Iface::Delete, Iface::DefaultConfirmation, m_parent);
    deleteJob->start();
}

void KDirSelectDialog::Private::slotProperties()
{
    KPropertiesDialog *dialog = new KPropertiesDialog(m_treeView->selectedUrl(), this->m_parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

KDirSelectDialog::KDirSelectDialog(const QUrl &startDir, bool localOnly, QWidget *parent)
    // #ifdef Q_OS_WIN
    //     : QDialog(parent, Qt::WindowMinMaxButtonsHint),
    // #else
    //     : QDialog(parent),
    // #endif
    : d(new Private(localOnly, this))
{
    setWindowTitle(i18nc("@title:window", "Select Folder"));
    setMinimumSize({200, 200});

    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->setContentsMargins({});
    setLayout(topLayout);

    QPushButton *folderButton = new QPushButton(this);
    KGuiItem::assign(folderButton, KGuiItem(i18nc("@action:button", "New Folder..."), QStringLiteral("folder-new")));
    connect(folderButton, &QPushButton::clicked, this, [this]() {
        d->slotMkdir();
    });

    auto splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    topLayout->addWidget(splitter);

    auto mainWidget = new QWidget(splitter);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins({});

    d->m_actions = new KActionCollection(this);
    d->m_actions->addAssociatedWidget(this);
    d->m_placesView = new KFilePlacesView(splitter);
    d->m_placesView->setModel(new KFilePlacesModel(d->m_placesView));
    d->m_placesView->setObjectName(QStringLiteral("speedbar"));
    d->m_placesView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->m_placesView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->m_placesView->setMinimumSize(250, 200);
    connect(d->m_placesView, &KFilePlacesView::urlChanged, this, &KDirSelectDialog::setCurrentUrl);

    splitter->addWidget(d->m_placesView);
    splitter->addWidget(mainWidget);

    d->m_treeView = new KFileTreeView(splitter);
    d->m_treeView->setDirOnlyMode(true);
    d->m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    d->m_treeView->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::TopEdge | Qt::BottomEdge}));

    for (int i = 1; i < d->m_treeView->model()->columnCount(); ++i) {
        d->m_treeView->hideColumn(i);
    }

    auto urlComboWrapper = new QWidget(splitter);
    auto urlComboWrapperLayout = new QVBoxLayout(urlComboWrapper);
    urlComboWrapperLayout->setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin), 0, style()->pixelMetric(QStyle::PM_LayoutRightMargin), 0);

    d->m_urlCombo = new KHistoryComboBox(urlComboWrapper);
    d->m_urlCombo->setContentsMargins({});
    d->m_urlCombo->setLayoutDirection(Qt::LeftToRight);
    d->m_urlCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    d->m_urlCombo->setTrapReturnKey(true);
    d->m_urlCombo->setIconProvider([](const QString &name) {
        return QIcon::fromTheme(KIO::iconNameForUrl(QUrl::fromUserInput(name)));
    });
    KUrlCompletion *comp = new KUrlCompletion();
    comp->setMode(KUrlCompletion::DirCompletion);
    d->m_urlCombo->setCompletionObject(comp, true);
    d->m_urlCombo->setAutoDeleteCompletionObject(true);
    d->m_urlCombo->setDuplicatesEnabled(false);
    urlComboWrapperLayout->addWidget(d->m_urlCombo);

    d->m_contextMenu = new QMenu(this);

    QAction *newFolder = new QAction(i18nc("@action:inmenu", "New Folder..."), this);
    d->m_actions->addAction(newFolder->objectName(), newFolder);
    newFolder->setIcon(QIcon::fromTheme(QStringLiteral("folder-new")));
    newFolder->setShortcuts(KStandardShortcut::createFolder());
    connect(newFolder, &QAction::triggered, this, [this]() {
        d->slotMkdir();
    });
    d->m_contextMenu->addAction(newFolder);

    d->moveToTrash = new QAction(i18nc("@action:inmenu", "Move to Trash"), this);
    d->m_actions->addAction(d->moveToTrash->objectName(), d->moveToTrash);
    d->moveToTrash->setIcon(QIcon::fromTheme(QStringLiteral("user-trash")));
    d->moveToTrash->setShortcut(Qt::Key_Delete);
    connect(d->moveToTrash, &QAction::triggered, this, [this]() {
        d->slotMoveToTrash();
    });
    d->m_contextMenu->addAction(d->moveToTrash);

    d->deleteAction = new QAction(i18nc("@action:inmenu", "Delete"), this);
    d->m_actions->addAction(d->deleteAction->objectName(), d->deleteAction);
    d->deleteAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    d->deleteAction->setShortcut(Qt::SHIFT | Qt::Key_Delete);
    connect(d->deleteAction, &QAction::triggered, this, [this]() {
        d->slotDelete();
    });
    d->m_contextMenu->addAction(d->deleteAction);

    d->m_contextMenu->addSeparator();

    d->showHiddenFoldersAction = new KToggleAction(i18nc("@option:check", "Show Hidden Folders"), this);
    d->m_actions->addAction(d->showHiddenFoldersAction->objectName(), d->showHiddenFoldersAction);
    d->showHiddenFoldersAction->setShortcuts(KStandardShortcut::showHideHiddenFiles());
    connect(d->showHiddenFoldersAction, &QAction::triggered, d->m_treeView, &KFileTreeView::setShowHiddenFiles);
    d->m_contextMenu->addAction(d->showHiddenFoldersAction);
    d->m_contextMenu->addSeparator();

    QAction *propertiesAction = new QAction(i18nc("@action:inmenu", "Properties"), this);
    d->m_actions->addAction(propertiesAction->objectName(), propertiesAction);
    propertiesAction->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")));
    propertiesAction->setShortcut(Qt::ALT | Qt::Key_Return);
    connect(propertiesAction, &QAction::triggered, this, [this]() {
        d->slotProperties();
    });
    d->m_contextMenu->addAction(propertiesAction);

    d->m_startURL = KFileWidget::getStartUrl(startDir, d->m_recentDirClass);
    if (localOnly && !d->m_startURL.isLocalFile()) {
        QString docPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        if (QDir(docPath).exists()) {
            d->m_startURL = QUrl::fromLocalFile(docPath);
        } else {
            d->m_startURL = QUrl::fromLocalFile(QDir::homePath());
        }
    }

    d->m_startDir = d->m_startURL;
    d->m_rootUrl = d->m_treeView->rootUrl();

    d->readConfig(KSharedConfig::openConfig(), QStringLiteral("DirSelect Dialog"));

    d->m_buttons = new QDialogButtonBox(mainWidget);
    d->m_buttons->setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                     0,
                                     style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                     style()->pixelMetric(QStyle::PM_LayoutBottomMargin));
    d->m_buttons->addButton(folderButton, QDialogButtonBox::ActionRole);
    d->m_buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(d->m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(d->m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(d->m_treeView, 1);
    mainLayout->addWidget(urlComboWrapper, 0);
    mainLayout->addWidget(d->m_buttons);

    connect(d->m_treeView, &KFileTreeView::currentUrlChanged, this, [this](const QUrl &url) {
        d->slotCurrentChanged(url);
    });

    connect(d->m_treeView, &QAbstractItemView::activated, this, [this](const QModelIndex &index) {
        d->slotExpand(index);
    });

    connect(d->m_treeView, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        d->slotContextMenuRequested(pos);
    });

    connect(d->m_urlCombo, &QComboBox::editTextChanged, this, [this](const QString &text) {
        d->slotComboTextChanged(text);
    });

    connect(d->m_urlCombo, &QComboBox::textActivated, this, [this](const QString &text) {
        d->slotUrlActivated(text);
    });

    connect(d->m_urlCombo, QOverload<const QString &>::of(&KComboBox::returnPressed), this, [this](const QString &text) {
        d->slotUrlActivated(text);
    });

    setCurrentUrl(d->m_startURL);
}

KDirSelectDialog::~KDirSelectDialog()
{
    delete d;
}

QUrl KDirSelectDialog::url() const
{
    QUrl comboUrl = QUrl::fromUserInput(d->m_urlCombo->currentText());

    if (comboUrl.isValid()) {
        KIO::StatJob *statJob = KIO::stat(comboUrl, KIO::HideProgressInfo);
        KJobWidgets::setWindow(statJob, d->m_parent);
        const bool ok = statJob->exec();
        if (ok && statJob->statResult().isDir()) {
            return comboUrl;
        }
    }

    // qDebug() << comboUrl.path() << " is not an accessible directory";
    return d->m_treeView->currentUrl();
}

QUrl KDirSelectDialog::rootUrl() const
{
    return d->m_rootUrl;
}

QAbstractItemView *KDirSelectDialog::view() const
{
    return d->m_treeView;
}

bool KDirSelectDialog::localOnly() const
{
    return d->m_localOnly;
}

QUrl KDirSelectDialog::startDir() const
{
    return d->m_startDir;
}

void KDirSelectDialog::setCurrentUrl(const QUrl &url)
{
    if (!url.isValid()) {
        return;
    }

    if (url.scheme() != d->m_rootUrl.scheme()) {
        QUrl u(url);
        // We need the url to end with / because some code ahead (kdirmodel) is expecting
        // to find the / separator. It can happen that a valid url like smb: does not have
        // one so we should add it.
        if (!u.toString().endsWith(QLatin1Char('/'))) {
            u.setPath(QStringLiteral("/"));
        }

        d->m_treeView->setRootUrl(u);
        d->m_rootUrl = u;
    }

    // Check if url represents a hidden folder and enable showing them
    QString fileName = url.fileName();
    // TODO a better hidden file check?
    bool isHidden = fileName.length() > 1 && fileName[0] == QLatin1Char('.') && (fileName.length() > 2 ? fileName[1] != QLatin1Char('.') : true);
    bool showHiddenFiles = isHidden && !d->m_treeView->showHiddenFiles();
    if (showHiddenFiles) {
        d->showHiddenFoldersAction->setChecked(true);
        d->m_treeView->setShowHiddenFiles(true);
    }

    d->m_treeView->setCurrentUrl(url);
}

void KDirSelectDialog::accept()
{
    QUrl selectedUrl = url();
    if (!selectedUrl.isValid()) {
        return;
    }

    if (!d->m_recentDirClass.isEmpty()) {
        KRecentDirs::add(d->m_recentDirClass, selectedUrl.toString());
    }

    d->m_urlCombo->addToHistory(selectedUrl.toDisplayString());
    KFileWidget::setStartDir(url());

    QDialog::accept();
}

void KDirSelectDialog::hideEvent(QHideEvent *event)
{
    d->saveConfig(KSharedConfig::openConfig(), QStringLiteral("DirSelect Dialog"));

    QDialog::hideEvent(event);
}

// static
QUrl KDirSelectDialog::selectDirectory(const QUrl &startDir, bool localOnly, QWidget *parent, const QString &caption)
{
    KDirSelectDialog myDialog(startDir, localOnly, parent);

    if (!caption.isNull()) {
        myDialog.setWindowTitle(caption);
    }

    if (myDialog.exec() == QDialog::Accepted) {
        QUrl url = myDialog.url();

        // Returning the most local url
        if (url.isLocalFile()) {
            return url;
        }

        KIO::StatJob *job = KIO::stat(url);
        KJobWidgets::setWindow(job, parent);

        if (!job->exec()) {
            return url;
        }

        KIO::UDSEntry entry = job->statResult();
        const QString path = entry.stringValue(KIO::UDSEntry::UDS_LOCAL_PATH);

        return path.isEmpty() ? url : QUrl::fromLocalFile(path);
    } else {
        return QUrl();
    }
}

QUrl KDirSelectDialog::directory()
{
    return url();
}

QList<QUrl> KDirSelectDialog::selectedFiles()
{
    return QList<QUrl>() << url();
}

void KDirSelectDialog::setOkButtonText(const QString &text)
{
    d->m_buttons->button(QDialogButtonBox::Ok)->setText(text);
}

void KDirSelectDialog::setCancelButtonText(const QString &text)
{
    d->m_buttons->button(QDialogButtonBox::Cancel)->setText(text);
}

void KDirSelectDialog::setDirectory(const QUrl &directory)
{
    setCurrentUrl(directory);
}

QString KDirSelectDialog::selectedMimeTypeFilter()
{
    return QString();
}

QString KDirSelectDialog::selectedNameFilter()
{
    return QString();
}

QString KDirSelectDialog::currentFilterText()
{
    return QString();
}

void KDirSelectDialog::selectFile(const QUrl &filename)
{
    Q_UNUSED(filename)
}

void KDirSelectDialog::selectMimeTypeFilter(const QString &filter)
{
    Q_UNUSED(filter)
}

void KDirSelectDialog::selectNameFilter(const QString &filter)
{
    Q_UNUSED(filter)
}

#include "moc_kdirselectdialog_p.cpp"
