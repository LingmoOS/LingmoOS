/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "activitieslinkingwidget.h"

#include "ui_activitieslinkingwidget.h"

#include <QItemDelegate>

#include <PlasmaActivities/ActivitiesModel>

class ActivitiesLinkingWidget::Private
{
public:
    Ui::ActivitiesLinkingWidget ui;
};

namespace
{
class CheckboxDelegate : public QItemDelegate
{ //_
public:
    CheckboxDelegate(QObject *parent)
        : QItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        auto wholeRect = option.rect;

        // Drawing the checkbox
        auto checkRect = wholeRect;
        checkRect.setWidth(checkRect.height());
        drawCheck(painter, option, checkRect, option.state & QStyle::State_Selected ? Qt::Checked : Qt::Unchecked);

        // Drawing the text
        auto textRect = wholeRect;
        textRect.setLeft(textRect.left() + 8 + textRect.height());
        drawDisplay(painter, option, textRect, index.data(Qt::DisplayRole).toString());
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        return QSize(100, 22);
    }
}; //^
}

ActivitiesLinkingWidget::ActivitiesLinkingWidget()
    : DialogDsl::DialogModule(true)
    , d(new Private())
{
    d->ui.setupUi(this);

    d->ui.listActivities->setModel(new KActivities::ActivitiesModel(this));
    d->ui.listActivities->setItemDelegate(new CheckboxDelegate(this));
}

ActivitiesLinkingWidget::~ActivitiesLinkingWidget()
{
}

PlasmaVault::Vault::Payload ActivitiesLinkingWidget::fields() const
{
    const auto selection = d->ui.listActivities->selectionModel();
    QStringList selectedActivities;
    for (const auto &selectedIndex : selection->selectedIndexes()) {
        selectedActivities << selectedIndex.data(KActivities::ActivitiesModel::ActivityId).toString();
    }

    return {{KEY_ACTIVITIES, selectedActivities}};
}

void ActivitiesLinkingWidget::init(const PlasmaVault::Vault::Payload &payload)
{
    const auto activities = payload[KEY_ACTIVITIES].toStringList();

    d->ui.checkLimitActivities->setChecked(false);
    d->ui.listActivities->setEnabled(false);

    auto model = d->ui.listActivities->model();
    auto selection = d->ui.listActivities->selectionModel();
    selection->clearSelection();

    if (activities.size() > 0) {
        d->ui.checkLimitActivities->setChecked(true);
        d->ui.listActivities->setEnabled(true);

        for (int row = 0; row < d->ui.listActivities->model()->rowCount(); ++row) {
            const auto index = model->index(row, 0);
            const auto activity = model->data(index, KActivities::ActivitiesModel::ActivityId).toString();

            if (activities.contains(activity)) {
                selection->select(index, QItemSelectionModel::Select);
            }
        }
    }
}

#include "moc_activitieslinkingwidget.cpp"
