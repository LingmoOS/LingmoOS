// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSETTINGS_H
#define DFMSETTINGS_H

#include <QObject>
#include <QVariantHash>

class SettingsPrivate;
class Settings : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Settings)

    Q_PROPERTY(bool autoSync READ autoSync WRITE setAutoSync)
    Q_PROPERTY(bool watchChanges READ watchChanges WRITE setWatchChanges)

public:
    enum ConfigType {
        AppConfig,
        GenericConfig
    };

    explicit Settings(const QString &defaultFile, const QString &fallbackFile, const QString &settingFile, QObject *parent = nullptr);
    explicit Settings(const QString &name, ConfigType type = AppConfig, QObject *parent = nullptr);
    ~Settings();

    bool contains(const QString &group, const QString &key) const;

    QSet<QString> groups() const;
    QSet<QString> keys(const QString &group) const;
    QStringList keyList(const QString &group) const;

    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue = QVariant()) const;
    void setValue(const QString &group, const QString &key, const QVariant &value);
    // if changed return true
    bool setValueNoNotify(const QString &group, const QString &key, const QVariant &value);

    void removeGroup(const QString &group);
    bool isRemovable(const QString &group, const QString &key) const;
    void remove(const QString &group, const QString &key);
    void clear();
    void reload();

    bool sync();

    bool autoSync() const;
    bool watchChanges() const;

public Q_SLOTS:
    void setAutoSync(bool autoSync);
    void setWatchChanges(bool watchChanges);
    void onFileChanged(const QString &filePath);

Q_SIGNALS:
    void valueChanged(const QString &group, const QString &key, const QVariant &value);
    void valueEdited(const QString &group, const QString &key, const QVariant &value);

private:
    QScopedPointer<SettingsPrivate> d_ptr;
};

#endif // DFMSETTINGS_H
