/*
    Persons Model Example
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QApplication>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include <QPushButton>
#include <QVBoxLayout>

#include <QPainter>
#include <QStyledItemDelegate>

#include <personsmodel.h>

using namespace KPeople;

const int SPACING = 8;
const int PHOTO_SIZE = 32;

class PersonsDelegate : public QStyledItemDelegate
{
public:
    PersonsDelegate(QObject *parent = nullptr);
    ~PersonsDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

PersonsDelegate::PersonsDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

PersonsDelegate::~PersonsDelegate()
{
}

void PersonsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optV4 = option;
    initStyleOption(&optV4, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QRect contactPhotoRect = optV4.rect;
    contactPhotoRect.adjust(SPACING, SPACING, SPACING, SPACING);
    contactPhotoRect.setWidth(PHOTO_SIZE);
    contactPhotoRect.setHeight(PHOTO_SIZE);

    QImage avatar = index.data(Qt::DecorationRole).value<QImage>();
    painter->drawImage(contactPhotoRect, avatar);

    painter->drawRect(contactPhotoRect);

    QRect nameRect = optV4.rect;
    nameRect.adjust(SPACING + PHOTO_SIZE + SPACING, SPACING, 0, 0);

    painter->drawText(nameRect, index.data(Qt::DisplayRole).toString());

    QRect idRect = optV4.rect;
    idRect.adjust(SPACING + PHOTO_SIZE + SPACING, SPACING + 15, 0, 0);
    painter->drawText(idRect, index.data(PersonsModel::PersonUriRole).toString());

    painter->restore();
}

QSize PersonsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(128, 48);
}

class ContactListApp : public QWidget
{
    Q_OBJECT
public:
    ContactListApp();
private Q_SLOTS:
    void onMergeClicked();
    void onUnmergeClicked();

private:
    PersonsModel *m_model;
    QTreeView *m_view;
};

ContactListApp::ContactListApp()
{
    m_view = new QTreeView(this);
    m_model = new PersonsModel(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QSortFilterProxyModel *sortFilter = new QSortFilterProxyModel(m_view);
    sortFilter->setDynamicSortFilter(true);
    sortFilter->setSourceModel(m_model);
    sortFilter->sort(0);
    m_view->setRootIsDecorated(false);
    m_view->setModel(sortFilter);
    m_view->setItemDelegate(new PersonsDelegate(this));
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);

    layout->addWidget(m_view);
    QPushButton *mergeButton = new QPushButton(QStringLiteral("Merge"), this);
    connect(mergeButton, SIGNAL(released()), SLOT(onMergeClicked()));
    layout->addWidget(mergeButton);

    QPushButton *unmergeButton = new QPushButton(QStringLiteral("Unmerge"), this);
    connect(unmergeButton, SIGNAL(released()), SLOT(onUnmergeClicked()));
    layout->addWidget(unmergeButton);
}

void ContactListApp::onMergeClicked()
{
    const QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
    QStringList ids;
    for (const QModelIndex &index : indexes) {
        ids << index.data(PersonsModel::PersonUriRole).toString();
    }

    if (!ids.isEmpty()) {
        KPeople::mergeContacts(ids);
    }
}

void ContactListApp::onUnmergeClicked()
{
    QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        QString id = indexes.first().data(PersonsModel::PersonUriRole).toString();
        KPeople::unmergeContact(id);
    }
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ContactListApp widget;
    widget.show();
    return app.exec();
}

#include "contactlistwidgets.moc"
