// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processinfomodel.h"

#include <QDebug>
#include <QIcon>
#include <QFile>
#include <QRegularExpression>

inline QString formatMem(const unsigned mem_bytes)
{
    return QString("%1M").arg(std::max(10u, mem_bytes / 1024 / 1024));
}

inline QString appName(const QString &desktop)
{
    if (!desktop.endsWith(".desktop"))
        return QString();

    const int start = desktop.lastIndexOf('/') + 1;
    const int end = desktop.lastIndexOf(".desktop");

    return desktop.midRef(start, end - start).toString();
}

QPixmap appIcon(const int size, const QString &desktop)
{
    static QIcon defaultIcon = QIcon::fromTheme("application-x-desktop");

    QFile file(desktop);

    if (file.open(QIODevice::ReadOnly)) {
        const QString &content = file.readAll();
        const QStringList &lines = content.split('\n');

        auto it = std::find_if(lines.begin(), lines.end(), [](const QString &line) { return line.split('=').first() == "Icon"; });
        if (it != lines.end()) {
            const QString icon = it->split('=').last();
            return QIcon::fromTheme(icon, defaultIcon).pixmap(size, size);
        }
    }

    return QIcon::fromTheme(appName(desktop), defaultIcon).pixmap(size, size);
}

ProcessInfoModel::ProcessInfoModel(QObject *parent)
    : QAbstractItemModel(parent)

    , m_processInfos(new ProcessInfoManager)
{
    connect(m_processInfos, &ProcessInfoManager::processInfoListChanged, this, [=] { emit layoutChanged(); });
}

int ProcessInfoModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return m_processInfos->processInfoList.size();

    if (!parent.parent().isValid())
        return m_processInfos->processInfoList[parent.row()].sub_procs.size() - 1;

    return 0;
}

int ProcessInfoModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 2;
    else
        return COLUMN_NUMS;
}

QVariant ProcessInfoModel::data(const QModelIndex &index, int role) const
{
    switch (role)
    {
    case Qt::SizeHintRole:
        if (index.parent().isValid())
            return QSize(0, 24);
        return QSize(0, 40);
    case DisplayRole:
        switch (index.column())
        {
        case COLUMN_FREE_BTN:
            return QString(tr("End"));
        case COLUMN_MEM:
            return formatMem(m_processInfos->processInfoList[index.row()].total_mem_bytes);
        case COLUMN_NAME:
            if (!index.parent().isValid())
            {
                if (m_processInfos->processInfoList[index.row()].sub_procs.isEmpty())
                    return m_processInfos->processInfoList[index.row()].app_name;
                else
                    return m_processInfos->processInfoList[index.row()].sub_procs.first().app_name;
            }
            return m_processInfos->processInfoList[index.parent().row()].sub_procs[index.row() + 1].app_name;
        default:;
        }
        break;
    case IconRole:
    {
        if (index.parent().isValid())
            return QIcon::fromTheme("google-chrome").pixmap(24, 24);

        const QString &desktop = m_processInfos->processInfoList[index.row()].desktop;
        switch (index.column())
        {
        case COLUMN_ICON:
            return appIcon(24, desktop);
        case COLUMN_FREE_BTN:
            return appIcon(48, desktop);
        default:;
        }
    }
        break;
    case TextAlignmentRole:
        switch (index.column())
        {
        case COLUMN_MEM:    return int(Qt::AlignRight | Qt::AlignVCenter);
        default:            return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
    case StateRole:
        return m_pressedIndex == index;
    case PidListRole:
        return m_processInfos->processInfoList[index.row()].pid_list;
    case TabListRole:
    {
        QList<int> ret;
        for (const auto &tab : m_processInfos->processInfoList[index.row()].sub_procs)
            ret << tab.id;
        return QVariant::fromValue(ret);
    }
    default:;
    }

    return QVariant();
}

QModelIndex ProcessInfoModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, &m_processInfos->processInfoList[row]);

    if (!parent.parent().isValid())
        return createIndex(row, column, &m_processInfos->processInfoList[parent.row()].sub_procs[row + 1]);

    return QModelIndex();
}

QModelIndex ProcessInfoModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    ProcessInfo *info = reinterpret_cast<ProcessInfo *>(child.internalId());

    if (info == nullptr)
        return QModelIndex();

    for (int i(0); i != m_processInfos->processInfoList.size(); ++i)
    {
        if (info->id == m_processInfos->processInfoList[i].id)
            return QModelIndex();

        for (int j(0); j != m_processInfos->processInfoList[i].sub_procs.size(); ++j)
        {
            if (info->id == m_processInfos->processInfoList[i].sub_procs[j].id)
                return createIndex(i, 0, &m_processInfos->processInfoList[i]);
        }
    }

    return QModelIndex();
}

void ProcessInfoModel::clearPressed()
{
    const QModelIndex idx = m_pressedIndex;
    m_pressedIndex = QModelIndex();

    emit dataChanged(idx, idx);
}

bool ProcessInfoModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role)
    Q_UNUSED(value)

    m_pressedIndex = index;

    return true;
}
