/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001-2004 Anders Lund <anders@alweb.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kmimetypechooser.h"

#include "kmimetypeeditor.h"
#include <qmimedatabase.h>

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTreeView>
#include <QVBoxLayout>

// BEGIN KMimeTypeChooserPrivate
class KMimeTypeChooserPrivate
{
public:
    KMimeTypeChooserPrivate(KMimeTypeChooser *parent)
        : q(parent)
    {
    }

    void loadMimeTypes(const QStringList &selected = QStringList());
    QList<const QStandardItem *> getCheckedItems();

    void editMimeType();
    void slotCurrentChanged(const QModelIndex &index);
    void slotSycocaDatabaseChanged(const QStringList &);

    KMimeTypeChooser *const q;
    QTreeView *mimeTypeTree = nullptr;
    QStandardItemModel *m_model = nullptr;
    QSortFilterProxyModel *m_proxyModel = nullptr;
    QLineEdit *m_filterLineEdit = nullptr;
    QPushButton *btnEditMimeType = nullptr;

    QString defaultgroup;
    QStringList groups;
    int visuals;
};
// END

static const char s_keditfiletypeExecutable[] = "keditfiletype";

// BEGIN KMimeTypeChooser
KMimeTypeChooser::KMimeTypeChooser(const QString &text,
                                   const QStringList &selMimeTypes,
                                   const QString &defaultGroup,
                                   const QStringList &groupsToShow,
                                   int visuals,
                                   QWidget *parent)
    : QWidget(parent)
    , d(new KMimeTypeChooserPrivate(this))
{
    d->defaultgroup = defaultGroup;
    d->groups = groupsToShow;
    if (visuals & EditButton) {
        if (QStandardPaths::findExecutable(QString::fromLatin1(s_keditfiletypeExecutable)).isEmpty()) {
            visuals &= ~EditButton;
        }
    }
    d->visuals = visuals;

    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    vboxLayout->setContentsMargins(0, 0, 0, 0);
    if (!text.isEmpty()) {
        vboxLayout->addWidget(new QLabel(text, this));
    }

    d->mimeTypeTree = new QTreeView(this);
    d->m_model = new QStandardItemModel(d->mimeTypeTree);
    d->m_proxyModel = new QSortFilterProxyModel(d->mimeTypeTree);
    d->m_proxyModel->setRecursiveFilteringEnabled(true);
    d->m_proxyModel->setFilterKeyColumn(-1);
    d->m_proxyModel->setSourceModel(d->m_model);
    d->mimeTypeTree->setModel(d->m_proxyModel);

    d->m_filterLineEdit = new QLineEdit(this);
    d->m_filterLineEdit->setPlaceholderText(tr("Search for file type or filename pattern…", "@info:placeholder"));
    QLabel *filterLabel = new QLabel(tr("&Filter:", "@label:textbox"));
    filterLabel->setBuddy(d->m_filterLineEdit);
    connect(d->m_filterLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        d->m_proxyModel->setFilterRegularExpression(
            QRegularExpression(text, QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption));
    });

    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(d->m_filterLineEdit);
    vboxLayout->addLayout(filterLayout);
    d->m_filterLineEdit->setFocus();

    vboxLayout->addWidget(d->mimeTypeTree);
    QStringList headerLabels({tr("MIME Type", "@title:column")});

    if (visuals & Comments) {
        headerLabels.append(tr("Comment", "@title:column"));
    }

    if (visuals & Patterns) {
        headerLabels.append(tr("Patterns", "@title:column"));
    }

    d->m_model->setColumnCount(headerLabels.count());
    d->m_model->setHorizontalHeaderLabels(headerLabels);
    QFontMetrics fm(d->mimeTypeTree->fontMetrics());
    // big enough for most names/comments, but not for the insanely long ones
    const int optWidth = 20 * fm.averageCharWidth();
    d->mimeTypeTree->setColumnWidth(0, optWidth);
    d->mimeTypeTree->setColumnWidth(1, optWidth);

    d->loadMimeTypes(selMimeTypes);

    if (visuals & EditButton) {
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch(1);
        d->btnEditMimeType = new QPushButton(tr("&Edit…", "@action:button"), this);
        buttonLayout->addWidget(d->btnEditMimeType);
        d->btnEditMimeType->setEnabled(false);

        connect(d->btnEditMimeType, &QPushButton::clicked, this, [this]() {
            d->editMimeType();
        });
        connect(d->mimeTypeTree, &QAbstractItemView::doubleClicked, this, [this]() {
            d->editMimeType();
        });

        connect(d->mimeTypeTree, &QTreeView::activated, this, [this](const QModelIndex &index) {
            d->slotCurrentChanged(index);
        });

        d->btnEditMimeType->setToolTip(tr("Launch the MIME type editor", "@info:tooltip"));

        vboxLayout->addLayout(buttonLayout);
    }
}

KMimeTypeChooser::~KMimeTypeChooser() = default;

void KMimeTypeChooserPrivate::loadMimeTypes(const QStringList &_selectedMimeTypes)
{
    QStringList selMimeTypes;

    if (!_selectedMimeTypes.isEmpty()) {
        selMimeTypes = _selectedMimeTypes;
    } else {
        selMimeTypes = q->mimeTypes();
    }

    std::vector<QStandardItem *> parentGroups;
    QMimeDatabase db;
    const QList<QMimeType> mimetypes = db.allMimeTypes();

    bool agroupisopen = false;
    QStandardItem *idefault = nullptr; // open this, if all other fails
    QStandardItem *firstChecked = nullptr; // make this one visible after the loop

    for (const QMimeType &mt : mimetypes) {
        const QString mimetype = mt.name();
        const int index = mimetype.indexOf(QLatin1Char('/'));
        // e.g. "text", "audio", "inode"
        const QString maj = mimetype.left(index);

        if (!groups.isEmpty() && !groups.contains(maj)) {
            continue;
        }

        QStandardItem *groupItem;

        auto it = std::find_if(parentGroups.cbegin(), parentGroups.cend(), [maj](const QStandardItem *item) {
            return maj == item->text();
        });

        if (it == parentGroups.cend()) {
            groupItem = new QStandardItem(maj);
            groupItem->setFlags(Qt::ItemIsEnabled);
            // a dud item to fill the patterns column next to "groupItem" and setFlags() on it
            QStandardItem *secondColumn = new QStandardItem();
            secondColumn->setFlags(Qt::NoItemFlags);
            QStandardItem *thirdColumn = new QStandardItem();
            thirdColumn->setFlags(Qt::NoItemFlags);
            m_model->appendRow({groupItem, secondColumn, thirdColumn});
            parentGroups.push_back(groupItem);
            if (maj == defaultgroup) {
                idefault = groupItem;
            }
        } else {
            groupItem = *it;
        }

        // e.g. "html", "plain", "mp4"
        const QString min = mimetype.mid(index + 1);
        QStandardItem *mime = new QStandardItem(QIcon::fromTheme(mt.iconName()), min);
        mime->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

        QStandardItem *comments = nullptr;
        if (visuals & KMimeTypeChooser::Comments) {
            comments = new QStandardItem(mt.comment());
            comments->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }

        QStandardItem *patterns = nullptr;

        if (visuals & KMimeTypeChooser::Patterns) {
            patterns = new QStandardItem(mt.globPatterns().join(QLatin1String("; ")));
            patterns->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }

        groupItem->appendRow(QList<QStandardItem *>({mime, comments, patterns}));

        if (selMimeTypes.contains(mimetype)) {
            mime->setCheckState(Qt::Checked);
            const QModelIndex index = m_proxyModel->mapFromSource(m_model->indexFromItem(groupItem));
            mimeTypeTree->expand(index);
            agroupisopen = true;
            if (!firstChecked) {
                firstChecked = mime;
            }
        } else {
            mime->setCheckState(Qt::Unchecked);
        }
    }

    m_model->sort(0);

    if (firstChecked) {
        const QModelIndex index = m_proxyModel->mapFromSource(m_model->indexFromItem(firstChecked));
        mimeTypeTree->scrollTo(index);
    }

    if (!agroupisopen && idefault) {
        const QModelIndex index = m_proxyModel->mapFromSource(m_model->indexFromItem(idefault));
        mimeTypeTree->expand(index);
        mimeTypeTree->scrollTo(index);
    }
}

void KMimeTypeChooserPrivate::editMimeType()
{
    QModelIndex mimeIndex = m_proxyModel->mapToSource(mimeTypeTree->currentIndex());

    // skip parent (non-leaf) nodes
    if (m_model->hasChildren(mimeIndex)) {
        return;
    }

    if (mimeIndex.column() > 0) { // we need the item from column 0 to concatenate "mt" below
        mimeIndex = m_model->sibling(mimeIndex.row(), 0, mimeIndex);
    }

    const QStandardItem *item = m_model->itemFromIndex(mimeIndex);
    const QString mt = (item->parent())->text() + QLatin1Char('/') + item->text();
    KMimeTypeEditor::editMimeType(mt, q);

    // TODO: use a QFileSystemWatcher on one of the shared-mime-info generated files, instead.
    // q->connect( KSycoca::self(), SIGNAL(databaseChanged(QStringList)),
    //            q, SLOT(slotSycocaDatabaseChanged(QStringList)) );
    // TODO: use QFileSystemWatcher to be told when keditfiletype changed a MIME type
    // or a better idea: a QMimeDatabaseWatcher class in Qt itself
}

void KMimeTypeChooserPrivate::slotCurrentChanged(const QModelIndex &index)
{
    if (btnEditMimeType) {
        const QModelIndex srcIndex = m_proxyModel->mapToSource(index);
        const QStandardItem *currentItem = m_model->itemFromIndex(srcIndex);
        btnEditMimeType->setEnabled(currentItem && currentItem->parent());
    }
}

// TODO: see editMimeType
void KMimeTypeChooserPrivate::slotSycocaDatabaseChanged(const QStringList &changedResources)
{
    if (changedResources.contains(QLatin1String("xdgdata-mime"))) {
        loadMimeTypes();
    }
}

QList<const QStandardItem *> KMimeTypeChooserPrivate::getCheckedItems()
{
    QList<const QStandardItem *> lst;
    const int rowCount = m_model->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        const QStandardItem *groupItem = m_model->item(i);
        const int childCount = groupItem->rowCount();
        for (int j = 0; j < childCount; ++j) {
            const QStandardItem *child = groupItem->child(j);
            if (child->checkState() == Qt::Checked) {
                lst.append(child);
            }
        }
    }
    return lst;
}

QStringList KMimeTypeChooser::mimeTypes() const
{
    QStringList mimeList;
    const QList<const QStandardItem *> checkedItems = d->getCheckedItems();
    mimeList.reserve(checkedItems.size());
    for (const QStandardItem *item : checkedItems) {
        mimeList.append(item->parent()->text() + QLatin1Char('/') + item->text());
    }
    return mimeList;
}

QStringList KMimeTypeChooser::patterns() const
{
    QStringList patternList;
    const QList<const QStandardItem *> checkedItems = d->getCheckedItems();
    QMimeDatabase db;
    for (const QStandardItem *item : checkedItems) {
        QMimeType mime = db.mimeTypeForName(item->parent()->text() + QLatin1Char('/') + item->text());
        Q_ASSERT(mime.isValid());
        patternList += mime.globPatterns();
    }
    return patternList;
}
// END

// BEGIN KMimeTypeChooserDialogPrivate

class KMimeTypeChooserDialogPrivate
{
public:
    KMimeTypeChooserDialogPrivate(KMimeTypeChooserDialog *parent)
        : q(parent)
    {
    }

    void init();

    KMimeTypeChooserDialog *q;
    KMimeTypeChooser *m_chooser;
};

// END

// BEGIN KMimeTypeChooserDialog
KMimeTypeChooserDialog::KMimeTypeChooserDialog(const QString &title,
                                               const QString &text,
                                               const QStringList &selMimeTypes,
                                               const QString &defaultGroup,
                                               const QStringList &groupsToShow,
                                               int visuals,
                                               QWidget *parent)
    : QDialog(parent)
    , d(new KMimeTypeChooserDialogPrivate(this))
{
    setWindowTitle(title);

    d->m_chooser = new KMimeTypeChooser(text, selMimeTypes, defaultGroup, groupsToShow, visuals, this);
    d->init();
}

KMimeTypeChooserDialog::KMimeTypeChooserDialog(const QString &title,
                                               const QString &text,
                                               const QStringList &selMimeTypes,
                                               const QString &defaultGroup,
                                               QWidget *parent)
    : QDialog(parent)
    , d(new KMimeTypeChooserDialogPrivate(this))
{
    setWindowTitle(title);

    d->m_chooser = new KMimeTypeChooser(text,
                                        selMimeTypes,
                                        defaultGroup,
                                        QStringList(),
                                        KMimeTypeChooser::Comments | KMimeTypeChooser::Patterns | KMimeTypeChooser::EditButton,
                                        this);
    d->init();
}

KMimeTypeChooser *KMimeTypeChooserDialog::chooser()
{
    return d->m_chooser;
}

void KMimeTypeChooserDialogPrivate::init()
{
    QVBoxLayout *layout = new QVBoxLayout(q);

    layout->addWidget(m_chooser);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(q);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, q, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, q, &QDialog::reject);
    layout->addWidget(buttonBox);
}

KMimeTypeChooserDialog::~KMimeTypeChooserDialog() = default;

QSize KMimeTypeChooserDialog::sizeHint() const
{
    QFontMetrics fm(fontMetrics());
    const int viewableSize = fm.averageCharWidth() * 60;
    return QSize(viewableSize, viewableSize);
}

// END KMimeTypeChooserDialog

#include "moc_kmimetypechooser.cpp"
