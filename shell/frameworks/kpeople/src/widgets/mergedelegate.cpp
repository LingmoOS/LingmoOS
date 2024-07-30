/*
    KPeople - Duplicates
    SPDX-FileCopyrightText: 2013 Franck Arrecot <franck.arrecot@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "mergedelegate.h"
#include "mergedialog.h"
#include "personsmodel.h"
#include <match_p.h>

#include "kpeople_widgets_debug.h"
#include <KLocalizedString>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QIcon>
#include <QLabel>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>

#define MAX_MATCHING_CONTACTS_ICON 5
#define SIZE_STANDARD_PIXMAP 35

using namespace KPeople;

// TODO: use proper, runtime values there
QSize MergeDelegate::s_decorationSize(SIZE_STANDARD_PIXMAP, SIZE_STANDARD_PIXMAP);
QSize MergeDelegate::s_arrowSize(15, 15);

QSize MergeDelegate::pictureSize()
{
    return s_decorationSize;
}

MergeDelegate::MergeDelegate(QAbstractItemView *parent)
    : KExtendableItemDelegate(parent)
{
    static QIcon arrowD = QIcon::fromTheme(QStringLiteral("arrow-down"));
    setContractPixmap(arrowD.pixmap(s_arrowSize));

    static QIcon arrowR = QIcon::fromTheme(QStringLiteral("arrow-right"));
    setExtendPixmap(arrowR.pixmap(s_arrowSize));
}

MergeDelegate::~MergeDelegate()
{
}

void MergeDelegate::onClickContactParent(const QModelIndex &parent)
{
    if (isExtended(parent)) {
        contractItem(parent);
    } else {
        QItemSelection item = QItemSelection(parent, parent);
        onSelectedContactsChanged(item, QItemSelection());
    }
}

void MergeDelegate::onSelectedContactsChanged(const QItemSelection &now, const QItemSelection &old)
{
    if (!old.indexes().isEmpty()) {
        QModelIndex oldIdx = old.indexes().first();

        if (isExtended(oldIdx)) {
            contractItem(oldIdx);
        }
    }
    if (!now.indexes().isEmpty()) {
        QModelIndex idx = now.indexes().first();
        extendItem(buildMultipleLineLabel(idx), idx);
    }
}

QWidget *MergeDelegate::buildMultipleLineLabel(const QModelIndex &idx)
{
    QString contents;
    const QAbstractItemModel *model = idx.model();
    const int rows = model->rowCount(idx);
    for (int i = 0; i < rows; ++i) {
        const QModelIndex child = model->index(i, 0, idx);
        Match m = child.data(MergeDialog::MergeReasonRole).value<Match>();

        QString name = m.indexB.data(Qt::DisplayRole).toString();
        QString display = i18nc("name: merge reasons", "%1: %2", name, m.matchReasons().join(i18nc("reasons join", ", ")));
        contents += display + QLatin1String("<p/>");
    }
    QLabel *childDisplay = new QLabel(contents, dynamic_cast<QWidget *>(parent()));
    childDisplay->setAlignment(Qt::AlignRight);
    childDisplay->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    return childDisplay;
}

void MergeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &optionOld, const QModelIndex &index) const
{
    QStyleOptionViewItem option(optionOld);
    QStyleOptionViewItem opt(option);
    KExtendableItemDelegate::paint(painter, option, index);

    const int separation = 5;

    const QAbstractItemModel *model = index.model();
    int facesRows = qMin(model->rowCount(index), MAX_MATCHING_CONTACTS_ICON);
    for (int i = 0; i < facesRows; i++) { // Children Icon Displaying Loop
        const QModelIndex child = model->index(i, 0, index);

        QVariant decoration = child.data(Qt::DecorationRole);
        Q_ASSERT(decoration.userType() == QMetaType::QIcon);

        QIcon pix = decoration.value<QIcon>();
        QPoint pixmapPoint = {option.rect.width() / 2 + i * (s_decorationSize.width() + separation), option.rect.top()};
        painter->drawPixmap(pixmapPoint, pix.pixmap(s_decorationSize));
    }
    // draw a vertical line to separate the original person and the merging contacts
    int midWidth = option.rect.width() / 2;
    painter->setPen(opt.palette.color(QPalette::Window));
    painter->drawLine(option.rect.left() + midWidth - SIZE_STANDARD_PIXMAP,
                      option.rect.bottom() - 5,
                      option.rect.left() + midWidth - SIZE_STANDARD_PIXMAP,
                      option.rect.top() + 5);
}

#include "moc_mergedelegate.cpp"
