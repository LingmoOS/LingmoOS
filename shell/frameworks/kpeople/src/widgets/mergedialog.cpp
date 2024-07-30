/*
    KPeople - Duplicates
    SPDX-FileCopyrightText: 2013 Franck Arrecot <franck.arrecot@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "mergedialog.h"
#include "duplicatesfinder_p.h"
#include "matchessolver_p.h"
#include "mergedelegate.h"
#include "persondata.h"
#include "personsmodel.h"

#include "kpeople_widgets_debug.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QListView>
#include <QPushButton>
#include <QStandardItemModel>

#include <KLocalizedString>
#include <KPixmapSequence>
#include <KPixmapSequenceWidget>

using namespace KPeople;

class MergeDialogPrivate
{
public:
    PersonsModel *personsModel;
    QListView *view;
    MergeDelegate *delegate;

    QStandardItemModel *model;
    DuplicatesFinder *duplicatesFinder;
    KPixmapSequenceWidget *sequence;
};

MergeDialog::MergeDialog(QWidget *parent)
    : QDialog(parent)
    , d_ptr(new MergeDialogPrivate)
{
    Q_D(MergeDialog);

    d_ptr->personsModel = nullptr;
    d_ptr->delegate = nullptr;
    d_ptr->duplicatesFinder = nullptr;

    setWindowTitle(i18n("Duplicates Manager"));

    auto *layout = new QVBoxLayout(this);

    setMinimumSize(450, 350);

    d->model = new QStandardItemModel(this);
    d->view = new QListView(this);
    d->view->setModel(d->model);
    d->view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QLabel *topLabel = new QLabel(i18n("Select contacts to be merged"));

    QDialogButtonBox *buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), SLOT(onMergeButtonClicked()));
    connect(buttons, SIGNAL(rejected()), SLOT(reject()));

    d->sequence = new KPixmapSequenceWidget(this);
    d->sequence->setSequence(KPixmapSequence(QStringLiteral("process-working"), 22));
    d->sequence->setInterval(100);
    d->sequence->setVisible(false);

    layout->addWidget(topLabel);
    layout->addWidget(d->view);
    layout->addWidget(d->sequence);
    layout->addWidget(buttons);
}

MergeDialog::~MergeDialog()
{
    delete d_ptr;
}

void MergeDialog::setPersonsModel(PersonsModel *model)
{
    Q_D(MergeDialog);
    d->personsModel = model;
    if (d->personsModel) {
        searchForDuplicates();
        connect(d->personsModel, SIGNAL(modelInitialized(bool)), SLOT(searchForDuplicates()));
    }
}

void MergeDialog::searchForDuplicates()
{
    Q_D(MergeDialog);
    if (!d->personsModel || !d->personsModel->rowCount() || d->duplicatesFinder) {
        qCWarning(KPEOPLE_WIDGETS_LOG) << "MergeDialog failed to launch the duplicates research";
        return;
    }
    d->duplicatesFinder = new DuplicatesFinder(d->personsModel);
    connect(d->duplicatesFinder, SIGNAL(result(KJob *)), SLOT(searchForDuplicatesFinished(KJob *)));
    d->duplicatesFinder->start();
}

void MergeDialog::onMergeButtonClicked()
{
    Q_D(MergeDialog);
    QList<Match> matches;
    for (int i = 0, rows = d->model->rowCount(); i < rows; i++) {
        QStandardItem *item = d->model->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            for (int j = 0, contactsCount = item->rowCount(); j < contactsCount; ++j) {
                QStandardItem *matchItem = item->child(j);
                matches << matchItem->data(MergeDialog::MergeReasonRole).value<Match>();
            }
        }
    }

    MatchesSolver *solverJob = new MatchesSolver(matches, d->personsModel, this);
    solverJob->start();
    d->sequence->setVisible(true);
    d->view->setEnabled(false);
    connect(solverJob, SIGNAL(finished(KJob *)), this, SLOT(accept()));
}

void MergeDialog::searchForDuplicatesFinished(KJob *)
{
    Q_D(MergeDialog);
    feedDuplicateModelFromMatches(d->duplicatesFinder->results());

    d->delegate = new MergeDelegate(d->view);
    d->view->setItemDelegate(d->delegate);

    // To extend the selected item
    connect(d->view->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            d->delegate,
            SLOT(onSelectedContactsChanged(QItemSelection, QItemSelection)));
    // To contract an already selected item
    connect(d->view, SIGNAL(doubleClicked(QModelIndex)), d->delegate, SLOT(onClickContactParent(QModelIndex)));
}

void MergeDialog::feedDuplicateModelFromMatches(const QList<Match> &matches)
{
    Q_D(MergeDialog);
    QHash<QPersistentModelIndex, QList<Match>> compareTable;
    QHash<QPersistentModelIndex, QPersistentModelIndex> doneIndexes;

    for (const Match &match : matches) {
        QPersistentModelIndex destination = doneIndexes.value(match.indexA, match.indexA);
        QHash<QPersistentModelIndex, QList<Match>>::iterator currentValue = compareTable.find(destination);

        if (currentValue == compareTable.end()) { // new parent, create it
            compareTable[match.indexA] = QList<Match>() << match;
        } else { // know parent, add child
            currentValue->append(match);
        }
        doneIndexes[match.indexB] = destination;
    }
    // now build the model : 1st dimension = person candidate, 2nd dimension = match
    QStandardItem *rootItem = d->model->invisibleRootItem();
    QHash<QPersistentModelIndex, QList<Match>>::const_iterator i;

    for (i = compareTable.constBegin(); i != compareTable.constEnd(); ++i) {
        // Build the merge Contact in the model
        QStandardItem *parent = itemMergeContactFromMatch(true, i->first());
        rootItem->appendRow(parent);

        for (const Match &matchChild : std::as_const(*i)) {
            parent->appendRow(itemMergeContactFromMatch(false, matchChild));
        }
    }

    rootItem->sortChildren(0);
}

QStandardItem *MergeDialog::itemMergeContactFromMatch(bool isParent, const Match &match)
{
    QStandardItem *item = new QStandardItem;

    item->setCheckable(true);
    item->setCheckState(Qt::Unchecked);
    item->setSizeHint(MergeDelegate::pictureSize());
    item->setData(true, KExtendableItemDelegate::ShowExtensionIndicatorRole);

    QVariant deco;
    if (!isParent) { // child
        QString uri = match.indexB.data(PersonsModel::PersonUriRole).toString();
        item->setData(uri, UriRole);

        item->setData(QVariant::fromValue<Match>(match), MergeReasonRole);
        item->setText(match.indexB.data(Qt::DisplayRole).toString());
        deco = match.indexB.data(Qt::DecorationRole);

    } else { // parent
        QString uri = match.indexA.data(PersonsModel::PersonUriRole).toString();
        item->setData(uri, UriRole);

        item->setText(match.indexA.data(Qt::DisplayRole).toString());
        deco = match.indexA.data(Qt::DecorationRole);
    }

    QIcon icon;
    if (deco.userType() == QMetaType::QIcon) {
        icon = deco.value<QIcon>();
    } else if (deco.userType() == QMetaType::QPixmap) {
        icon = QIcon(deco.value<QPixmap>());
    } else if (deco.userType() == QMetaType::QImage) {
        icon = QIcon(QPixmap::fromImage(deco.value<QImage>()));
    } else {
        qCWarning(KPEOPLE_WIDGETS_LOG) << "unknown decoration type" << deco.typeName();
    }
    item->setIcon(icon);
    return item;
}

#include "moc_mergedialog.cpp"
