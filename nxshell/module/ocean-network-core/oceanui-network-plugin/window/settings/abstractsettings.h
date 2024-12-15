// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ABSTRACTSETTINGS_H
#define ABSTRACTSETTINGS_H

#include <QWidget>

#include <networkmanagerqt/connectionsettings.h>

class AbstractSection;
class QVBoxLayout;

class AbstractSettings : public QWidget
{
    Q_OBJECT

public:
    explicit AbstractSettings(NetworkManager::ConnectionSettings::Ptr connSettings, QWidget *parent = nullptr);
    virtual ~AbstractSettings();

    bool allInputValid();
    void saveSettings();

    bool isAutoConnect();

Q_SIGNALS:
    void requestNextPage(QWidget *const);
    void requestFrameAutoHide(const bool autoHide) const;
    void anyEditClicked();

protected:
    virtual void initSections() = 0;

    // 是否清除连接的 interface-name 字段
    // 如果不设置连接的 macAddress 则需要清空 interface-name 字段, 反之不清空, 而是设置为当前设备的 interfaceName
    virtual bool setInterfaceName() { return true; }

protected:
    virtual void resetConnectionInterfaceName();

protected:
    NetworkManager::ConnectionSettings::Ptr m_connSettings;

    QVBoxLayout *m_sectionsLayout;
    QList<AbstractSection *> m_settingSections;
};

#endif /* ABSTRACTSETTINGS_H */
