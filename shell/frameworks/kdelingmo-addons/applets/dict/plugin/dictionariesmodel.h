/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef DICTIONARIES_MODEL_H
#define DICTIONARIES_MODEL_H

#include "../../dict/dictengine.h"

#include <unordered_map>

#include <QAbstractListModel>
#include <QAbstractSocket>
#include <vector>

struct AvailableDict {
    QString id;
    QString description;
    bool enabled = false;
};

class EnabledDictModel : public QAbstractListModel
{
public:
    explicit EnabledDictModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    void appendDict(const AvailableDict &dict);
    void removeDict(int _index);

private:
    QList<AvailableDict> m_enabledDicts;
};

class DictionariesModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @return the list of enabled dictionaries
     */
    Q_PROPERTY(QString enabledDicts READ enabledDicts WRITE setEnabledDicts NOTIFY enabledDictsChanged)

    /**
     * @return the model that contains all enabled dictionaries
     */
    Q_PROPERTY(QAbstractListModel *enabledDictModel READ enabledDictModel CONSTANT)

    /**
     * @return @c true if the engine is downloading dict list from
     * the Internet, @c false otherwise.
     */
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

    /**
     * @return the type of the last socket error
     */
    Q_PROPERTY(QAbstractSocket::SocketError errorCode READ errorCode NOTIFY errorCodeChanged)

    /**
     * @return a human-readable description of the last socket error
     */
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    explicit DictionariesModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString enabledDicts() const;
    void setEnabledDicts(const QString &dicts);

    QAbstractListModel *enabledDictModel() const;

    Q_INVOKABLE void setEnabled(const QString &dict);
    Q_INVOKABLE void setDisabled(int _index);
    Q_INVOKABLE void move(int oldIndex, int newIndex);

    bool loading() const;

    QAbstractSocket::SocketError errorCode() const;
    QString errorString() const;

Q_SIGNALS:
    void enabledDictsChanged();
    void loadingChanged();
    void errorCodeChanged();
    void errorStringChanged();

private Q_SLOTS:
    void slotDictErrorOccurred(QAbstractSocket::SocketError socketError, const QString &errorString);
    void slotDictLoadingChanged(bool loading);

private:
    void setAvailableDicts(const QVariantMap &data);

    DictEngine m_engine;
    EnabledDictModel *m_enabledDictModel;

    std::vector<AvailableDict> m_availableDicts;
    std::unordered_map<QString /*id*/, int /*index*/> m_idIndexProxyMap;
    QString m_enabledDicts;
    QStringList m_enabledDictIdList;

    bool m_loading = false;
    QAbstractSocket::SocketError m_errorCode = QAbstractSocket::UnknownSocketError;
    QString m_errorString;
};

#endif
