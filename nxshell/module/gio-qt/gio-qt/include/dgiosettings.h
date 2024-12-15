// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGIOSETTINGS_H
#define DGIOSETTINGS_H

#include <QObject>
#include <QScopedPointer>

class DGioSettingsPrivate;
class DGioSettings : public QObject
{
    Q_OBJECT
public:
    explicit DGioSettings(const QString& schemaId, QObject* parent = nullptr);
    DGioSettings(const QString& schemaId, const QString& path, QObject* parent = nullptr);

    ~DGioSettings();

    void sync();

    bool setValue(const QString& key, const QVariant& value, bool sync = false);
    QVariant value(const QString& key) const;

    Q_DECL_DEPRECATED QStringList keys() const;

    void reset(const QString& key);

    static bool isSchemaInstalled(const QString& schemaId);

Q_SIGNALS:
    void valueChanged(const QString& key, const QVariant& value);

private:
    QScopedPointer<DGioSettingsPrivate> d_private;

    Q_DECLARE_PRIVATE_D(d_private, DGioSettings)
};

#endif // DGIOSETTINGS_H
