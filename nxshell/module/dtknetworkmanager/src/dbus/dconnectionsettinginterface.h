// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCONNECTIONSETTINGINTERFACE_H
#define DCONNECTIONSETTINGINTERFACE_H

#include "dnetworkmanager_global.h"
#include "dnetworkmanagertypes.h"
#include <QObject>
#include <QString>
#include <DDBusInterface>
#include <QDBusPendingReply>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DConnectionSettingInterface : public QObject
{
    Q_OBJECT
public:
    explicit DConnectionSettingInterface(const QByteArray &path, QObject *parent = nullptr);
    ~DConnectionSettingInterface() override = default;

    Q_PROPERTY(bool unsaved READ unsaved NOTIFY UnsavedChanged)
    Q_PROPERTY(QString filename READ filename NOTIFY FilenameChanged)
    Q_PROPERTY(quint32 flags READ flags NOTIFY FlagsChanged)

    bool unsaved() const;
    QString filename() const;
    quint32 flags() const;

public Q_SLOTS:
    QDBusPendingReply<void> UpdateSetting(const SettingDesc &prop) const;
    QDBusPendingReply<void> deleteSetting() const;
    QDBusPendingReply<SettingDesc> getSettigns() const;

Q_SIGNALS:
    void UnsavedChanged(const bool unsaved);
    void FilenameChanged(const QString &filename);
    void FlagsChanged(const quint32 flags);

    void Removed();
    void Updated();

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
