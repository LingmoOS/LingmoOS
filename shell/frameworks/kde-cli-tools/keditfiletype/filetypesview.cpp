/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2000-2008 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Urs Wolfer <uwolfer @ kde.org>
    SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

// Own
#include "filetypesview.h"
#include "mimetypewriter.h"

// Qt
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeDatabase>
#include <QPushButton>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QVBoxLayout>

// KDE
#include <KBuildSycocaProgressDialog>
#include <KLineEdit>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSycoca>

// Local
#include "filegroupdetails.h"
#include "filetypedetails.h"
#include "kservicelistwidget.h"
#include "multiapplydialog.h"
#include "newtypedlg.h"
#include "typeslisttreewidget.h"

K_PLUGIN_CLASS_WITH_JSON(FileTypesView, "kcm_filetypes.json")

FileTypesView::FileTypesView(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    m_fileTypesConfig = KSharedConfig::openConfig(QStringLiteral("filetypesrc"), KConfig::NoGlobals);

    setButtons(Help | Apply);
    QString wtstr;

    QHBoxLayout *l = new QHBoxLayout(widget());
    QVBoxLayout *leftLayout = new QVBoxLayout();
    l->addLayout(leftLayout);

    patternFilterLE = new KLineEdit(widget());
    patternFilterLE->setClearButtonEnabled(true);
    patternFilterLE->setTrapReturnKey(true);
    patternFilterLE->setPlaceholderText(i18n("Search for file type or filename pattern..."));
    leftLayout->addWidget(patternFilterLE);

    connect(patternFilterLE, &QLineEdit::textChanged, this, &FileTypesView::slotFilter);

    wtstr = i18n(
        "Enter a part of a filename pattern, and only file types with a "
        "matching file pattern will appear in the list. Alternatively, enter "
        "a part of a file type name as it appears in the list.");

    patternFilterLE->setWhatsThis(wtstr);

    typesLV = new TypesListTreeWidget(widget());

    typesLV->setHeaderLabel(i18n("Known Types"));
    leftLayout->addWidget(typesLV);
    connect(typesLV, &QTreeWidget::currentItemChanged, this, &FileTypesView::updateDisplay);
    connect(typesLV, &QTreeWidget::itemDoubleClicked, this, &FileTypesView::slotDoubleClicked);

    typesLV->setWhatsThis(
        i18n("Here you can see a hierarchical list of"
             " the file types which are known on your system. Click on the '+' sign"
             " to expand a category, or the '-' sign to collapse it. Select a file type"
             " (e.g. text/html for HTML files) to view/edit the information for that"
             " file type using the controls on the right."));

    QHBoxLayout *btnsLay = new QHBoxLayout();
    leftLayout->addLayout(btnsLay);
    btnsLay->addStretch(1);
    QPushButton *addTypeB = new QPushButton(i18n("Add..."), widget());
    addTypeB->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    connect(addTypeB, &QAbstractButton::clicked, this, &FileTypesView::addType);
    btnsLay->addWidget(addTypeB);

    addTypeB->setWhatsThis(i18n("Click here to add a new file type."));

    m_removeTypeB = new QPushButton(i18n("&Remove"), widget());
    m_removeTypeB->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    connect(m_removeTypeB, &QAbstractButton::clicked, this, &FileTypesView::removeType);
    btnsLay->addWidget(m_removeTypeB);
    m_removeTypeB->setEnabled(false);
    m_removeButtonSaysRevert = false;

    // For the right panel, prepare a widget stack
    m_widgetStack = new QStackedWidget(widget());

    l->addWidget(m_widgetStack);

    // File Type Details
    m_details = new FileTypeDetails(m_widgetStack);
    m_details->allowMultiApply(true);
    connect(m_details, &FileTypeDetails::changed, this, &FileTypesView::setDirty);
    connect(m_details, &FileTypeDetails::embedMajor, this, &FileTypesView::slotEmbedMajor);
    connect(m_details, &FileTypeDetails::multiApply, this, &FileTypesView::multiApply);
    m_widgetStack->insertWidget(1, m_details /*id*/);

    // File Group Details
    m_groupDetails = new FileGroupDetails(m_widgetStack);
    connect(m_groupDetails, &FileGroupDetails::changed, this, &FileTypesView::setDirty);
    m_widgetStack->insertWidget(2, m_groupDetails /*id*/);

    // Widget shown on startup
    m_emptyWidget = new QLabel(i18n("Select a file type by name or by extension"), m_widgetStack);
    m_emptyWidget->setAlignment(Qt::AlignCenter);
    m_widgetStack->insertWidget(3, m_emptyWidget);

    m_widgetStack->setCurrentWidget(m_emptyWidget);

    connect(KSycoca::self(), qOverload<>(&KSycoca::databaseChanged), this, &FileTypesView::slotDatabaseChanged);
}

FileTypesView::~FileTypesView()
{
}

void FileTypesView::setDirty(bool state)
{
    setNeedsSave(state);
    m_dirty = state;
}

// To order the mimetype list
static bool mimeTypeLessThan(const QMimeType &m1, const QMimeType &m2)
{
    return m1.name() < m2.name();
}

// Note that this method loses any newly-added (and not saved yet) mimetypes.
// So this is really only for load().
void FileTypesView::readFileTypes()
{
    typesLV->clear();
    m_majorMap.clear();
    m_itemList.clear();

    QMimeDatabase db;
    QList<QMimeType> mimetypes = db.allMimeTypes();
    std::sort(mimetypes.begin(), mimetypes.end(), mimeTypeLessThan);
    auto it2(mimetypes.constBegin());
    for (; it2 != mimetypes.constEnd(); ++it2) {
        const QString mimetype = (*it2).name();
        const int index = mimetype.indexOf(QLatin1Char('/'));
        const QString maj = mimetype.left(index);

        TypesListItem *groupItem = m_majorMap.value(maj);
        if (!groupItem) {
            groupItem = new TypesListItem(typesLV, maj);
            m_majorMap.insert(maj, groupItem);
        }

        TypesListItem *item = new TypesListItem(groupItem, (*it2));
        m_itemList.append(item);
    }
    updateDisplay(nullptr);
}

void FileTypesView::slotEmbedMajor(const QString &major, bool &embed)
{
    TypesListItem *groupItem = m_majorMap.value(major);
    if (!groupItem) {
        return;
    }

    embed = (groupItem->mimeTypeData().autoEmbed() == MimeTypeData::Yes);
}

void FileTypesView::multiApply(int type)
{
    auto *current = static_cast<TypesListItem *>(typesLV->currentItem());
    MultiApplyDialog d(current, m_itemList, widget());

    if (d.exec()) {
        bool changed = false;

        for (auto *item : d.selected()) {
            if (type == KServiceListWidget::SERVICELIST_APPLICATIONS) {
                if (item->mimeTypeData().appServices() != current->mimeTypeData().appServices()) {
                    item->mimeTypeData().setAppServices(current->mimeTypeData().appServices());
                    changed = true;
                }
            } else if (type == KServiceListWidget::SERVICELIST_SERVICES) {
                if (item->mimeTypeData().embedParts() != current->mimeTypeData().embedParts()) {
                    item->mimeTypeData().setEmbedParts(current->mimeTypeData().embedParts());
                    changed = true;
                }
            }
        }

        if (changed) {
            setDirty(true);
        }
    }
}

void FileTypesView::slotFilter(const QString &patternFilter)
{
    for (int i = 0; i < typesLV->topLevelItemCount(); ++i) {
        typesLV->topLevelItem(i)->setHidden(true);
    }

    // insert all items and their group that match the filter
    for (TypesListItem *it : std::as_const(m_itemList)) {
        const MimeTypeData &mimeTypeData = it->mimeTypeData();
        if (patternFilter.isEmpty() || mimeTypeData.matchesFilter(patternFilter)) {
            TypesListItem *group = m_majorMap.value(mimeTypeData.majorType());
            Q_ASSERT(group);
            if (group) {
                group->setHidden(false);
                it->setHidden(false);
            }
        } else {
            it->setHidden(true);
        }
    }
}

void FileTypesView::addType()
{
    const QStringList allGroups = m_majorMap.keys();

    NewTypeDialog dialog(allGroups, widget());

    if (dialog.exec()) {
        const QString newMimeType = dialog.group() + QLatin1Char('/') + dialog.text();

        TypesListItem *group = m_majorMap.value(dialog.group());
        if (!group) {
            group = new TypesListItem(typesLV, dialog.group());
            m_majorMap.insert(dialog.group(), group);
        }

        // find out if our group has been filtered out -> insert if necessary
        QTreeWidgetItem *item = typesLV->topLevelItem(0);
        bool insert = true;
        while (item) {
            if (item == group) {
                insert = false;
                break;
            }
            item = typesLV->itemBelow(item);
        }
        if (insert) {
            typesLV->addTopLevelItem(group);
        }

        TypesListItem *tli = new TypesListItem(group, newMimeType);
        m_itemList.append(tli);

        group->setExpanded(true);
        tli->setSelected(true);

        setDirty(true);
    }
}

void FileTypesView::removeType()
{
    TypesListItem *current = static_cast<TypesListItem *>(typesLV->currentItem());

    if (!current) {
        return;
    }

    const MimeTypeData &mimeTypeData = current->mimeTypeData();

    // Can't delete groups nor essential mimetypes (but the button should be
    // disabled already in these cases, so this is just extra safety).
    if (mimeTypeData.isMeta() || mimeTypeData.isEssential()) {
        return;
    }

    if (!mimeTypeData.isNew()) {
        removedList.append(mimeTypeData.name());
    }
    if (m_removeButtonSaysRevert) {
        // Nothing else to do for now, until saving
        updateDisplay(current);
    } else {
        QTreeWidgetItem *li = typesLV->itemAbove(current);
        if (!li) {
            li = typesLV->itemBelow(current);
        }
        if (!li) {
            li = current->parent();
        }

        current->parent()->takeChild(current->parent()->indexOfChild(current));
        m_itemList.removeAll(current);
        if (li) {
            li->setSelected(true);
        }
    }
    setDirty(true);
}

void FileTypesView::slotDoubleClicked(QTreeWidgetItem *item)
{
    if (!item) {
        return;
    }
    item->setExpanded(!item->isExpanded());
}

void FileTypesView::updateDisplay(QTreeWidgetItem *item)
{
    TypesListItem *tlitem = static_cast<TypesListItem *>(item);
    updateRemoveButton(tlitem);

    if (!item) {
        m_widgetStack->setCurrentWidget(m_emptyWidget);
        return;
    }

    const bool wasDirty = m_dirty;

    MimeTypeData &mimeTypeData = tlitem->mimeTypeData();

    if (mimeTypeData.isMeta()) { // is a group
        m_widgetStack->setCurrentWidget(m_groupDetails);
        m_groupDetails->setMimeTypeData(&mimeTypeData);
    } else {
        m_widgetStack->setCurrentWidget(m_details);
        m_details->setMimeTypeData(&mimeTypeData);
    }

    // Updating the display indirectly called change(true)
    if (!wasDirty) {
        setDirty(false);
    }
}

void FileTypesView::updateRemoveButton(TypesListItem *tlitem)
{
    bool canRemove = false;
    m_removeButtonSaysRevert = false;

    if (tlitem) {
        const MimeTypeData &mimeTypeData = tlitem->mimeTypeData();
        if (!mimeTypeData.isMeta() && !mimeTypeData.isEssential()) {
            if (mimeTypeData.isNew()) {
                canRemove = true;
            } else {
                // We can only remove mimetypes that we defined ourselves, not those from freedesktop.org
                const QString mimeType = mimeTypeData.name();
                qDebug() << mimeType << "hasDefinitionFile:" << MimeTypeWriter::hasDefinitionFile(mimeType);
                if (MimeTypeWriter::hasDefinitionFile(mimeType)) {
                    canRemove = true;

                    // Is there a global definition for it?
                    const QStringList mimeFiles = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, //
                                                                            QLatin1String("mime/") + mimeType + QStringLiteral(".xml"));
                    qDebug() << mimeFiles;
                    if (mimeFiles.count() >= 2 /*a local and a global*/) {
                        m_removeButtonSaysRevert = true;
                        qDebug() << removedList;
                        if (removedList.contains(mimeType)) {
                            canRemove = false; // already on the "to be reverted" list, user needs to save now
                        }
                    }
                }
            }
        }
    }

    if (m_removeButtonSaysRevert) {
        m_removeTypeB->setText(i18n("&Revert"));
        m_removeTypeB->setToolTip(i18n("Revert this file type to its initial system-wide definition"));
        m_removeTypeB->setWhatsThis(
            i18n("Click here to revert this file type to its initial system-wide definition, which undoes any changes made to the file type. Note that "
                 "system-wide file types cannot be deleted. You can however empty their pattern list, to "
                 "minimize the chances of them being used (but the file type determination from file contents can still end up using them)."));
    } else {
        m_removeTypeB->setText(i18n("&Remove"));
        m_removeTypeB->setToolTip(i18n("Delete this file type definition completely"));
        m_removeTypeB->setWhatsThis(
            i18n("Click here to delete this file type definition completely. This is only possible for user-defined file types. System-wide file types cannot "
                 "be deleted. You can however empty their pattern list, to minimize the chances of "
                 "them being used (but the file type determination from file contents can still end up using them)."));
    }

    m_removeTypeB->setEnabled(canRemove);
}

void FileTypesView::save()
{
    bool needUpdateMimeDb = false;
    bool needUpdateSycoca = false;
    bool didIt = false;
    // first, remove those items which we are asked to remove.
    for (const QString &mime : std::as_const(removedList)) {
        MimeTypeWriter::removeOwnMimeType(mime);
        didIt = true;
        needUpdateMimeDb = true;
        needUpdateSycoca = true; // remove offers for this mimetype
    }
    removedList.clear();

    // now go through all entries and sync those which are dirty.
    // don't use typesLV, it may be filtered
    for (auto it = m_majorMap.cbegin(); it != m_majorMap.cend(); ++it) {
        TypesListItem *tli = it.value();
        if (tli->mimeTypeData().isDirty()) {
            qDebug() << "Entry " << tli->name() << " is dirty. Saving.";
            if (tli->mimeTypeData().sync()) {
                needUpdateMimeDb = true;
            }
            didIt = true;
        }
    }

    for (TypesListItem *tli : std::as_const(m_itemList)) {
        if (tli->mimeTypeData().isDirty()) {
            if (tli->mimeTypeData().isServiceListDirty()) {
                needUpdateSycoca = true;
            }
            qDebug() << "Entry " << tli->name() << " is dirty. Saving.";
            if (tli->mimeTypeData().sync()) {
                needUpdateMimeDb = true;
            }
            didIt = true;
        }
    }

    m_fileTypesConfig->sync();

    setDirty(false);

    if (needUpdateMimeDb) {
        MimeTypeWriter::runUpdateMimeDatabase();
    }
    if (needUpdateSycoca) {
        KBuildSycocaProgressDialog::rebuildKSycoca(widget());
    }

    if (didIt) { // TODO make more specific: only if autoEmbed changed? Well, maybe this is useful for icon and glob changes too...
        // Trigger reparseConfiguration of filetypesrc in konqueror
        // TODO: the same for dolphin. Or we should probably define a global signal for this.
        // Or a KGlobalSettings thing.
        QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KonqMain"), //
                                                          QStringLiteral("org.kde.Konqueror.Main"),
                                                          QStringLiteral("reparseConfiguration"));
        QDBusConnection::sessionBus().send(message);
    }

    updateDisplay(typesLV->currentItem());
}

void FileTypesView::load()
{
    widget()->setEnabled(false);
    widget()->setCursor(Qt::WaitCursor);

    readFileTypes();

    widget()->unsetCursor();
    setDirty(false);
    widget()->setEnabled(true);
}

void FileTypesView::slotDatabaseChanged()
{
    m_details->refresh();

    // ksycoca has new KMimeTypes objects for us, make sure to update
    // our 'copies' to be in sync with it. Not important for OK, but
    // important for Apply (how to differentiate those 2?).
    // See BR 35071.

    for (TypesListItem *tli : std::as_const(m_itemList)) {
        tli->mimeTypeData().refresh();
    }
}

void FileTypesView::defaults()
{
}

#include "filetypesview.moc"

#include "moc_filetypesview.cpp"
