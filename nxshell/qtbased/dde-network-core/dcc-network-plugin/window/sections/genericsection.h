// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GENERICSECTION_H
#define GENERICSECTION_H

#include "abstractsection.h"

#include <networkmanagerqt/connectionsettings.h>

namespace DCC_NAMESPACE {
class LineEditWidget;
class SwitchWidget;
}

class GenericSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit GenericSection(NetworkManager::ConnectionSettings::Ptr connSettings, QFrame *parent = nullptr);
    virtual ~GenericSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;
    bool autoConnectChecked() const;
    void setConnectionNameEditable(const bool editable);
    void setConnectionType(NetworkManager::ConnectionSettings::ConnectionType connType);
    inline DCC_NAMESPACE::LineEditWidget *connIdItem() { return m_connIdItem; }
    inline DCC_NAMESPACE::SwitchWidget *autoConnItem() { return m_autoConnItem; }
    bool connectionNameIsEditable();
    const QString connectionName() const;
    void setConnectionName(const QString &name);

private:
    void initUI();
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    DCC_NAMESPACE::LineEditWidget *m_connIdItem;
    DCC_NAMESPACE::SwitchWidget *m_autoConnItem;
    NetworkManager::ConnectionSettings::Ptr m_connSettings;
    NetworkManager::ConnectionSettings::ConnectionType m_connType;
};

#endif /* GENERICSECTION_H */
