/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#include "multiapplydialog.h"

#include "typeslistitem.h"
#include "typeslistproxyitem.h"
#include "typeslisttreewidget.h"
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QVBoxLayout>

MultiApplyDialog::MultiApplyDialog(const TypesListItem *source, const QList<TypesListItem *> &sourceItemList, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Apply To..."));
    resize(400, 500);

    QVBoxLayout *l = new QVBoxLayout(this);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_mimeTree = new TypesListTreeWidget(this);
    m_mimeTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_mimeTree->setHeaderLabel(i18n("Known Types"));

    l->addWidget(m_mimeTree);
    l->addWidget(buttons);

    connect(m_mimeTree, &QTreeWidget::itemChanged, this, &MultiApplyDialog::onItemChanged);
    connect(buttons, &QDialogButtonBox::accepted, this, &MultiApplyDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    load(source, sourceItemList);
}

MultiApplyDialog::~MultiApplyDialog()
{
}

const QList<TypesListItem *> MultiApplyDialog::selected() const
{
    return m_selected;
}

void MultiApplyDialog::onItemChanged(QTreeWidgetItem *qitem, int column)
{
    auto *item = static_cast<TypesListProxyItem *>(qitem);

    if (column != 0) {
        return;
    }

    QTreeWidgetItem *parent = item->parent();

    Qt::CheckState checkState = item->checkState(0);

    switch (checkState) {
    case Qt::Unchecked:
    case Qt::Checked:
        if (parent) {
            if (checkState == Qt::Unchecked) {
                m_selected.removeAll(item->inner());
            } else {
                if (!m_selected.contains(item->inner())) {
                    m_selected.append(item->inner());
                }
            }

            if (parent->childCount() > 1) {
                bool hasOtherState = false;

                for (int i = 0; i < parent->childCount(); i += 1) {
                    auto *child = parent->child(i);

                    if (child != m_source && child->checkState(0) != checkState) {
                        hasOtherState = true;
                        break;
                    }
                }

                if (hasOtherState) {
                    parent->setCheckState(0, Qt::PartiallyChecked);
                } else {
                    parent->setCheckState(0, checkState);
                }
            } else {
                parent->setCheckState(0, checkState);
            }
        }

        for (int i = 0; i < item->childCount(); i += 1) {
            auto *child = item->child(i);

            if (child != m_source) {
                child->setCheckState(0, checkState);
            }
        }

        break;
    case Qt::PartiallyChecked:
        break;
    }
}

void MultiApplyDialog::load(const TypesListItem *source, const QList<TypesListItem *> &sourceItemList)
{
    QMap<QString, TypesListProxyItem *> majorMap;
    m_mimeTree->clear();
    m_itemList.clear();
    m_selected.clear();
    m_source = nullptr;

    for (TypesListItem *type : sourceItemList) {
        const QString mimetype = type->name();
        const int index = mimetype.indexOf(QLatin1Char('/'));
        const QString maj = mimetype.left(index);

        TypesListProxyItem *groupItem = majorMap.value(maj);

        if (!groupItem) {
            groupItem = new TypesListProxyItem(static_cast<TypesListItem *>(type->parent()), m_mimeTree);
            groupItem->setCheckState(0, Qt::Unchecked);
            majorMap.insert(maj, groupItem);
        }

        TypesListProxyItem *item = new TypesListProxyItem(type, groupItem);
        item->setCheckState(0, Qt::Unchecked);

        if (type == source) {
            item->setFlags(Qt::ItemFlags());
            item->setCheckState(0, Qt::Checked);
            m_source = item;
        }

        m_itemList.append(item);
    }
}

#include "moc_multiapplydialog.cpp"
