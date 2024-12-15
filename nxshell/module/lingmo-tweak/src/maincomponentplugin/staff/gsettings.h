// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef GSETTINGS_H
#define GSETTINGS_H

#include <memory>
#include <QGSettings/QGSettings>
#include <QJSValue>
#include <QObject>

class GSettingsBackend : public QObject
{
    Q_OBJECT
public:
    explicit GSettingsBackend(const QByteArray &schema_id,
                              const QByteArray &path = QByteArray(),
                              QObject *parent = nullptr);

    Q_INVOKABLE void changed(QJSValue callback);
    Q_INVOKABLE bool contains(const QString &key) const;
    Q_INVOKABLE QVariant get(const QString &key) const;
    Q_INVOKABLE bool set(const QString &key, const QVariant &value);
    Q_INVOKABLE QStringList keys() const;
    Q_INVOKABLE QVariantList choices(const QString &key) const;
    Q_INVOKABLE void reset(const QString &key);

private:
    std::unique_ptr<QGSettings> m_settings;
    QJSValue m_callback;
};

#endif // GSETTINGS_H
