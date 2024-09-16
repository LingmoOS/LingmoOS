// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariant>
#include <memory>

class QSettings;
class SettingsBackend : public QObject
{
    Q_OBJECT
public:
    explicit SettingsBackend(const QString& file, QObject *parent = nullptr);
    ~SettingsBackend() override;

    Q_INVOKABLE QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    Q_INVOKABLE void setValue(const QString& key, const QVariant &value);
    Q_INVOKABLE void beginGroup(const QString& group);
    Q_INVOKABLE void endGroup();
    Q_INVOKABLE QStringList keys() const;

private:
   std::unique_ptr<QSettings> m_settings;
};

#endif // SETTINGS_H
