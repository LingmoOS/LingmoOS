/*
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knumbermodel.h"

#include <QtMath>

#include <cmath>

class KNumberModelPrivate
{
public:
    qreal minimumValue = 0.0;
    qreal maximumValue = 0.0;
    qreal stepSize = 1.0;
    QLocale::NumberOptions formattingOptions = QLocale::DefaultNumberOptions;
};

KNumberModel::KNumberModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new KNumberModelPrivate)
{
}

KNumberModel::~KNumberModel()
{
}

void KNumberModel::setMinimumValue(qreal minimumValue)
{
    if (minimumValue == d->minimumValue) {
        return;
    }
    beginResetModel();
    d->minimumValue = minimumValue;
    endResetModel();
    Q_EMIT minimumValueChanged();
}

qreal KNumberModel::minimumValue() const
{
    return d->minimumValue;
}

void KNumberModel::setMaximumValue(qreal maximumValue)
{
    if (maximumValue == d->maximumValue) {
        return;
    }
    beginResetModel();
    d->maximumValue = maximumValue;
    endResetModel();
    Q_EMIT maximumValueChanged();
}

qreal KNumberModel::maximumValue() const
{
    return d->maximumValue;
}

void KNumberModel::setStepSize(qreal stepSize)
{
    if (stepSize == d->stepSize) {
        return;
    }
    beginResetModel();
    d->stepSize = stepSize;
    endResetModel();
    Q_EMIT stepSizeChanged();
}

qreal KNumberModel::stepSize() const
{
    return d->stepSize;
}

void KNumberModel::setFormattingOptions(QLocale::NumberOptions formattingOptions)
{
    if (d->formattingOptions == formattingOptions) {
        return;
    }
    d->formattingOptions = formattingOptions;

    if (rowCount() == 0) {
        return;
    }
    dataChanged(index(0, 0, QModelIndex()), index(rowCount(), 0, QModelIndex()), QList<int>{DisplayRole});
    Q_EMIT formattingOptionsChanged();
}

QLocale::NumberOptions KNumberModel::formattingOptions() const
{
    return d->formattingOptions;
}

qreal KNumberModel::value(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0.0;
    }
    return d->minimumValue + d->stepSize * index.row();
}

int KNumberModel::rowCount(const QModelIndex &index) const
{
    if (index.parent().isValid()) {
        return 0;
    }
    if (d->stepSize == 0) {
        return 1;
    }
    // 1 initial entry (the minimumValue) + the number of valid steps afterwards
    return 1 + std::max(0, qFloor((d->maximumValue - d->minimumValue) / d->stepSize));
}

QVariant KNumberModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case KNumberModel::DisplayRole: {
        auto locale = QLocale();
        locale.setNumberOptions(d->formattingOptions);
        return QVariant(locale.toString(value(index)));
    }
    case KNumberModel::ValueRole:
        return QVariant(value(index));
    }
    return QVariant();
}

QHash<int, QByteArray> KNumberModel::roleNames() const
{
    return {{KNumberModel::DisplayRole, QByteArrayLiteral("display")}, {KNumberModel::ValueRole, QByteArrayLiteral("value")}};
}

#include "moc_knumbermodel.cpp"
