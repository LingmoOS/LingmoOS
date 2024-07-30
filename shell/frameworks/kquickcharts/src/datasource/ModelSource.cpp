/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ModelSource.h"

#include <QMetaProperty>

#include "charts_datasource_logging.h"

ModelSource::ModelSource(QObject *parent)
    : ChartDataSource(parent)
{
    connect(this, &ModelSource::modelChanged, this, &ModelSource::dataChanged);
    connect(this, &ModelSource::columnChanged, this, &ModelSource::dataChanged);
    connect(this, &ModelSource::roleChanged, this, &ModelSource::dataChanged);
    connect(this, &ModelSource::indexColumnsChanged, this, &ModelSource::dataChanged);
}

int ModelSource::role() const
{
    if (!m_model) {
        return -1;
    }

    if (m_role < 0 && !m_roleName.isEmpty()) {
        m_role = m_model->roleNames().key(m_roleName.toLatin1(), -1);
    }

    return m_role;
}

QString ModelSource::roleName() const
{
    return m_roleName;
}

int ModelSource::column() const
{
    return m_column;
}

QAbstractItemModel *ModelSource::model() const
{
    return m_model;
}

bool ModelSource::indexColumns() const
{
    return m_indexColumns;
}

int ModelSource::itemCount() const
{
    if (!m_model) {
        return 0;
    }

    return m_indexColumns ? m_model->columnCount() : m_model->rowCount();
}

QVariant ModelSource::item(int index) const
{
    if (!m_model) {
        return {};
    }

    // For certain model (QML ListModel for example), the roleNames() are more
    // dynamic and may only be valid when this method gets called. So try and
    // lookup the role first before anything else.
    if (m_role < 0) {
        if (m_roleName.isEmpty()) {
            return QVariant{};
        }

        m_role = m_model->roleNames().key(m_roleName.toLatin1(), -1);
        if (m_role < 0) {
            qCWarning(DATASOURCE) << "ModelSource: Invalid role " << m_role << m_roleName;
            return QVariant{};
        }
    }

    if (!m_indexColumns && (m_column < 0 || m_column > m_model->columnCount())) {
        qCDebug(DATASOURCE) << "ModelSource: Invalid column" << m_column;
        return QVariant{};
    }

    auto modelIndex = m_indexColumns ? m_model->index(0, index) : m_model->index(index, m_column);
    if (modelIndex.isValid()) {
        return m_model->data(modelIndex, m_role);
    }

    return QVariant{};
}

QVariant ModelSource::minimum() const
{
    if (!m_model || itemCount() <= 0) {
        return {};
    }

    if (m_minimum.isValid()) {
        return m_minimum;
    }

    auto minProperty = m_model->property("minimum");
    auto maxProperty = m_model->property("maximum");
    if (minProperty.isValid() && minProperty != maxProperty) {
        return minProperty;
    }

    QVariant result = std::numeric_limits<float>::max();
    for (int i = 0; i < itemCount(); ++i) {
        result = std::min(result, item(i), variantCompare);
    }
    return result;
}

QVariant ModelSource::maximum() const
{
    if (!m_model || itemCount() <= 0) {
        return {};
    }

    if (m_maximum.isValid()) {
        return m_maximum;
    }

    auto minProperty = m_model->property("minimum");
    auto maxProperty = m_model->property("maximum");
    if (maxProperty.isValid() && maxProperty != minProperty) {
        return maxProperty;
    }

    QVariant result = std::numeric_limits<float>::min();
    for (int i = 0; i < itemCount(); ++i) {
        result = std::max(result, item(i), variantCompare);
    }
    return result;
}

void ModelSource::setRole(int role)
{
    if (role == m_role) {
        return;
    }

    m_role = role;
    if (m_model) {
        m_roleName = QString::fromLatin1(m_model->roleNames().value(role));
        Q_EMIT roleNameChanged();
    }
    Q_EMIT roleChanged();
}

void ModelSource::setRoleName(const QString &name)
{
    if (name == m_roleName) {
        return;
    }

    m_roleName = name;
    if (m_model) {
        m_role = m_model->roleNames().key(m_roleName.toLatin1(), -1);
        Q_EMIT roleChanged();
    }
    Q_EMIT roleNameChanged();
}

void ModelSource::setColumn(int column)
{
    if (column == m_column) {
        return;
    }

    m_column = column;
    Q_EMIT columnChanged();
}

void ModelSource::setIndexColumns(bool index)
{
    if (index == m_indexColumns) {
        return;
    }

    m_indexColumns = index;
    Q_EMIT indexColumnsChanged();
}

void ModelSource::setModel(QAbstractItemModel *model)
{
    if (m_model == model) {
        return;
    }

    if (m_model) {
        m_model->disconnect(this);
        m_minimum = QVariant{};
        m_maximum = QVariant{};
    }

    m_model = model;
    if (m_model) {
        connect(m_model, &QAbstractItemModel::rowsInserted, this, &ModelSource::dataChanged);
        connect(m_model, &QAbstractItemModel::rowsRemoved, this, &ModelSource::dataChanged);
        connect(m_model, &QAbstractItemModel::rowsMoved, this, &ModelSource::dataChanged);
        connect(m_model, &QAbstractItemModel::modelReset, this, &ModelSource::dataChanged);
        connect(m_model, &QAbstractItemModel::dataChanged, this, &ModelSource::dataChanged);
        connect(m_model, &QAbstractItemModel::layoutChanged, this, &ModelSource::dataChanged);

        connect(m_model, &QAbstractItemModel::destroyed, this, [this]() {
            m_minimum = QVariant{};
            m_maximum = QVariant{};
            m_model = nullptr;
        });

        auto minimumIndex = m_model->metaObject()->indexOfProperty("minimum");
        if (minimumIndex != -1) {
            auto minimum = m_model->metaObject()->property(minimumIndex);
            if (minimum.hasNotifySignal()) {
                auto slot = metaObject()->method(metaObject()->indexOfSlot("onMinimumChanged()"));
                connect(m_model, minimum.notifySignal(), this, slot);
                m_minimum = minimum.read(m_model);
            }
        }

        auto maximumIndex = m_model->metaObject()->indexOfProperty("maximum");
        if (maximumIndex != -1) {
            auto maximum = m_model->metaObject()->property(maximumIndex);
            if (maximum.hasNotifySignal()) {
                auto slot = metaObject()->method(metaObject()->indexOfSlot("onMaximumChanged()"));
                connect(m_model, maximum.notifySignal(), this, slot);
                m_maximum = maximum.read(m_model);
            }
        }
    }

    Q_EMIT modelChanged();
}

void ModelSource::onMinimumChanged()
{
    auto newMinimum = m_model->property("minimum");
    if (newMinimum.isValid() && newMinimum != m_minimum) {
        m_minimum = newMinimum;
    }
}

void ModelSource::onMaximumChanged()
{
    auto newMaximum = m_model->property("maximum");
    if (newMaximum.isValid() && newMaximum != m_maximum) {
        m_maximum = newMaximum;
    }
}

#include "moc_ModelSource.cpp"
