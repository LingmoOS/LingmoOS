// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APPCHOOSERMODEL_H
#define APPCHOOSERMODEL_H
#include <QAbstractListModel>
#include <QString>
#include <QMap>
#include <QMetaType>
#include <QDBusMetaType>
#include <QDBusObjectPath>

using ObjectInterfaceMap = QMap<QString, QVariantMap>;
using ObjectMap = QMap<QDBusObjectPath, ObjectInterfaceMap>;
using PropMap = QMap<QString, QMap<QString, QString>>;

Q_DECLARE_METATYPE(ObjectMap)
Q_DECLARE_METATYPE(ObjectInterfaceMap)
Q_DECLARE_METATYPE(PropMap)

class AppChooserModel : public QAbstractListModel
{
    Q_OBJECT
public:
    struct DesktopInfo {
      QString appId;
      QString name;
      QString icon;
      bool selected;
    };

    enum Role{
        DataRole,
        NameRole,
        IconRole,
        SelectRole
    };
    explicit AppChooserModel(QObject *parent = nullptr);

    void click(const QModelIndex &index);
    void click(const QString &appId);

    QStringList choices();

private:
    void loadApplications();

protected:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QList<DesktopInfo> m_datas;
};

#endif // APPCHOOSERMODEL_H
