/*
 * SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "outputsmodel.h"
#include <KLocalizedString>
#include <QGuiApplication>
#include <QIcon>

OutputsModel::OutputsModel(Options o, QObject *parent)
    : QAbstractListModel(parent)
{
    const auto screens = qGuiApp->screens();

    // Only show the full workspace if there's several outputs
    if (screens.count() > 1 && (o & WorkspaceIncluded)) {
        m_outputs << Output{Output::Workspace, nullptr, i18n("Full Workspace"), "Workspace", {}};
    }

    if (o & VirtualIncluded) {
        static quint64 i = 0;
        m_outputs << Output{Output::Virtual, nullptr, i18n("New Virtual Output"), QStringLiteral("Virtual%1").arg(i++), {}};
    }

    if (o & RegionIncluded) {
        m_outputs << Output{Output::Region, nullptr, i18n("Rectangular Region"), "Region", {}};
    }

    for (const auto screen : screens) {
        Output::OutputType type = Output::Unknown;

        static const auto embedded = {
            QLatin1String("LVDS"),
            QLatin1String("IDP"),
            QLatin1String("EDP"),
            QLatin1String("LCD"),
        };

        for (const auto &prefix : embedded) {
            if (screen->name().startsWith(prefix, Qt::CaseInsensitive)) {
                type = Output::OutputType::Laptop;
                break;
            }
        }

        if (type == Output::OutputType::Unknown) {
            if (screen->name().contains(QLatin1String("TV"))) {
                type = Output::OutputType::Television;
            } else {
                type = Output::OutputType::Monitor;
            }
        }

        QString displayText;
        if (type == Output::OutputType::Laptop) {
            displayText = i18n("Laptop screen");
        } else {
            QStringList parts;
            if (!screen->manufacturer().isEmpty()) {
                parts.append(screen->manufacturer());

                if (!screen->model().isEmpty()) {
                    QString part = screen->model();
                    if (!screen->serialNumber().isEmpty()) {
                        part += QLatin1Char('/') + screen->serialNumber();
                    }
                    parts.append(part);
                } else if (!screen->serialNumber().isEmpty()) {
                    parts.append(screen->serialNumber());
                }

                parts.append(QLatin1Char('(') + screen->name() + QLatin1Char(')'));
            } else {
                parts.append(screen->name());
            }

            displayText = parts.join(QLatin1Char(' '));
        }

        const QPoint pos = screen->geometry().topLeft();
        m_outputs << Output(type, screen, displayText, QStringLiteral("%1x%2").arg(pos.x()).arg(pos.y()), screen->name());
    }
}

OutputsModel::~OutputsModel() = default;

int OutputsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_outputs.count();
}

QHash<int, QByteArray> OutputsModel::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, "display"},
        {Qt::DecorationRole, "decoration"},
        {Qt::CheckStateRole, "checked"},
        {ScreenRole, "screen"},
        {NameRole, "name"},
    };
}

QVariant OutputsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const auto &output = m_outputs[index.row()];
    switch (role) {
    case ScreenRole:
        return QVariant::fromValue(output.screen());
        return 0;
    case NameRole:
        return output.name();
    case Qt::DecorationRole:
        return QIcon::fromTheme(output.iconName());
    case Qt::DisplayRole:
        return output.display();
    case Qt::CheckStateRole:
        return m_selectedRows.contains(index.row()) ? Qt::Checked : Qt::Unchecked;
    }
    return {};
}

bool OutputsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid) || role != Qt::CheckStateRole) {
        return false;
    }

    if (index.data(Qt::CheckStateRole) == value) {
        return true;
    }

    if (value == Qt::Checked) {
        m_selectedRows.insert(index.row());
    } else {
        m_selectedRows.remove(index.row());
    }
    Q_EMIT dataChanged(index, index, {role});
    if (m_selectedRows.count() <= 1) {
        Q_EMIT hasSelectionChanged();
    }
    return true;
}

const Output &OutputsModel::outputAt(int row) const
{
    return m_outputs[row];
}

void OutputsModel::clearSelection()
{
    if (m_selectedRows.isEmpty())
        return;

    auto selected = m_selectedRows;
    m_selectedRows.clear();
    for (int i = 0, c = rowCount({}); i < c; ++i) {
        if (selected.contains(i)) {
            const auto idx = index(i, 0);
            Q_EMIT dataChanged(idx, idx, {Qt::CheckStateRole});
        }
    }
    Q_EMIT hasSelectionChanged();
}

QList<Output> OutputsModel::selectedOutputs() const
{
    QList<Output> ret;
    ret.reserve(m_selectedRows.count());
    for (auto x : std::as_const(m_selectedRows)) {
        ret << m_outputs[x];
    }
    return ret;
}
