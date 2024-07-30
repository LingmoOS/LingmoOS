/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SETTING_WIDGET_H
#define SETTING_WIDGET_H

#include "plasmanm_editor_export.h"

#include <NetworkManagerQt/Setting>
#include <QWidget>

#include <KAcceleratorManager>

class PLASMANM_EDITOR_EXPORT SettingWidget : public QWidget
{
    Q_OBJECT
public:
    class EnumPasswordStorageType
    {
    public:
        enum PasswordStorageType { Store = 0, AlwaysAsk, NotRequired };
    };

    explicit SettingWidget(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(), QWidget *parent = nullptr, Qt::WindowFlags f = {});
    explicit SettingWidget(const NetworkManager::Setting::Ptr &setting, const QStringList &hints, QWidget *parent = nullptr, Qt::WindowFlags f = {});

    ~SettingWidget() override;

    virtual void loadConfig(const NetworkManager::Setting::Ptr &setting);
    virtual void loadSecrets(const NetworkManager::Setting::Ptr &setting);

    virtual QVariantMap setting() const = 0;

    // Do not forget to call this function in the inherited class once initialized
    void watchChangedSetting();

    QString type() const;

    virtual bool isValid() const
    {
        return true;
    }

protected Q_SLOTS:
    void slotWidgetChanged();

Q_SIGNALS:
    void validChanged(bool isValid);
    void settingChanged();

protected:
    QStringList m_hints;

private:
    QString m_type;
};

#endif // SETTING_WIDGET_H
