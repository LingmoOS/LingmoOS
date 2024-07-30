/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "dictionariesmodel.h"
#include <QDebug>

EnabledDictModel::EnabledDictModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant EnabledDictModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    const int row = index.row();

    switch (role) {
    case Qt::DisplayRole:
        return m_enabledDicts[row].description;
    case Qt::EditRole:
        return m_enabledDicts[row].id;
    default:
        return {};
    }
}

int EnabledDictModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : m_enabledDicts.size();
}

QHash<int, QByteArray> EnabledDictModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "description"},
        {Qt::EditRole, "id"},
    };
}

bool EnabledDictModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent != destinationParent || sourceParent.isValid()) {
        return false;
    }

    const bool isMoveDown = destinationChild > sourceRow;
    // QAbstractItemModel::beginMoveRows(): when moving rows down in the same parent,
    // the rows will be placed before the destinationChild index.
    if (!beginMoveRows(sourceParent, sourceRow + count - 1, sourceRow, destinationParent, isMoveDown ? destinationChild + 1 : destinationChild)) {
        return false;
    }

    for (int i = 0; i < count; i++) {
        m_enabledDicts.move(isMoveDown ? sourceRow : sourceRow + i, destinationChild);
    }

    endMoveRows();
    return true;
}

void EnabledDictModel::appendDict(const AvailableDict &dict)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    m_enabledDicts.append(dict);

    endInsertRows();
}

void EnabledDictModel::removeDict(int _index)
{
    if (_index < 0 || _index >= rowCount()) {
        return;
    }

    beginRemoveRows(QModelIndex(), _index, _index);

    m_enabledDicts.removeAt(_index);

    endRemoveRows();
}

DictionariesModel::DictionariesModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_enabledDictModel(new EnabledDictModel(this))
{
    connect(&m_engine, &DictEngine::dictErrorOccurred, this, &DictionariesModel::slotDictErrorOccurred);
    connect(&m_engine, &DictEngine::dictsRecieved, this, [this](const QMap<QString, QString> &dicts) {
        beginResetModel();
        m_availableDicts = {};
        m_idIndexProxyMap.clear();
        m_availableDicts.resize(dicts.count());
        m_idIndexProxyMap.reserve(dicts.size());

        int i = 0;
        for (auto it = dicts.begin(), end = dicts.end(); it != end; ++it, ++i) {
            m_availableDicts[i] = AvailableDict{it.key(), it.value()};
            m_idIndexProxyMap.emplace(it.key(), i);
        }
        endResetModel();

        setEnabledDicts(m_enabledDicts);
    });
    connect(&m_engine, &DictEngine::dictLoadingChanged, this, &DictionariesModel::slotDictLoadingChanged);

    m_engine.requestDicts();
}

QVariant DictionariesModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    switch (role) {
    case Qt::DisplayRole:
        return m_availableDicts[row].description;
    case Qt::EditRole:
        return m_availableDicts[row].id;
    case Qt::CheckStateRole:
        return m_availableDicts[row].enabled;
    default:
        break;
    }
    return QVariant();
}

bool DictionariesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index)) {
        return false;
    }

    const int row = index.row();

    switch (role) {
    case Qt::CheckStateRole: {
        if (value.toBool()) {
            setEnabled(m_availableDicts[row].id);
        } else {
            setDisabled(m_enabledDictIdList.indexOf(m_availableDicts[row].id));
        }

        return true;
    }

    default:
        return false;
    }
}

int DictionariesModel::rowCount(const QModelIndex &index) const
{
    if (index.isValid()) {
        return 0; // flat model
    }
    return m_availableDicts.size();
}

QHash<int, QByteArray> DictionariesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "description"},
        {Qt::EditRole, "id"},
        {Qt::CheckStateRole, "checked"},
    };
}

QString DictionariesModel::enabledDicts() const
{
    return m_enabledDictIdList.join(QLatin1Char(','));
}

void DictionariesModel::setEnabledDicts(const QString &dicts)
{
    m_enabledDicts = dicts;
    if (!dicts.isEmpty()) {
        m_enabledDictIdList = dicts.split(QLatin1Char(','), Qt::SkipEmptyParts);
    } else {
        m_enabledDictIdList.clear();
    }
    Q_EMIT enabledDictsChanged();

    if (m_availableDicts.empty()) {
        // Loading
        return;
    }

    std::vector<AvailableDict> enabledDictList;
    enabledDictList.resize(m_enabledDictIdList.size());

    for (std::size_t i = 0; i < m_availableDicts.size(); i++) {
        auto &dict = m_availableDicts.at(i);
        auto it = std::find_if(m_enabledDictIdList.cbegin(), m_enabledDictIdList.cend(), [&dict](const QString &id) {
            return id == dict.id;
        });
        const bool enabled = it != m_enabledDictIdList.cend();

        if (dict.enabled != enabled) {
            dict.enabled = enabled;
            Q_EMIT dataChanged(index(i, 0), index(i, 0), {Qt::CheckStateRole});
        }

        if (enabled) {
            enabledDictList[std::distance(m_enabledDictIdList.cbegin(), it)] = dict;
        }
    }

    for (const auto &dict : std::as_const(enabledDictList)) {
        if (!dict.enabled) {
            continue;
        }

        m_enabledDictModel->appendDict(dict);
    }
}

QAbstractListModel *DictionariesModel::enabledDictModel() const
{
    return m_enabledDictModel;
}

void DictionariesModel::setEnabled(const QString &dict)
{
    const auto it = m_idIndexProxyMap.find(dict);
    if (it == m_idIndexProxyMap.end()) {
        return;
    }

    auto &d = m_availableDicts.at(it->second);
    if (d.enabled) {
        return;
    }

    d.enabled = true;
    Q_EMIT dataChanged(index(it->second, 0), index(it->second, 0), {Qt::CheckStateRole});

    if (!m_enabledDictIdList.contains(d.id)) {
        m_enabledDictIdList.append(d.id);
        m_enabledDictModel->appendDict(d);

        Q_EMIT enabledDictsChanged();
    }
}

void DictionariesModel::setDisabled(int _index)
{
    if (_index < 0 || _index >= m_enabledDictIdList.size()) {
        return;
    }

    m_enabledDictModel->removeDict(_index);

    const QString id = m_enabledDictIdList.takeAt(_index);
    Q_EMIT enabledDictsChanged();

    const auto it = m_idIndexProxyMap.find(id);
    if (it == m_idIndexProxyMap.end()) {
        return;
    }

    auto &d = m_availableDicts.at(it->second);
    d.enabled = false;
    Q_EMIT dataChanged(index(it->second, 0), index(it->second, 0), {Qt::CheckStateRole});
}

void DictionariesModel::move(int oldIndex, int newIndex)
{
    if (oldIndex < 0 || oldIndex >= m_enabledDictIdList.size()) {
        return;
    }
    if (newIndex < 0 || newIndex >= m_enabledDictIdList.size()) {
        return;
    }

    m_enabledDictModel->moveRows(QModelIndex(), oldIndex, 1, QModelIndex(), newIndex);

    m_enabledDictIdList.move(oldIndex, newIndex);
    Q_EMIT enabledDictsChanged();
}

bool DictionariesModel::loading() const
{
    return m_loading;
}

QAbstractSocket::SocketError DictionariesModel::errorCode() const
{
    return m_errorCode;
}

QString DictionariesModel::errorString() const
{
    return m_errorString;
}

void DictionariesModel::slotDictErrorOccurred(QAbstractSocket::SocketError socketError, const QString &errorString)
{
    m_errorCode = socketError;
    m_errorString = errorString;

    Q_EMIT errorCodeChanged();
    Q_EMIT errorStringChanged();
}

void DictionariesModel::slotDictLoadingChanged(bool loading)
{
    m_loading = loading;

    Q_EMIT loadingChanged();
}
