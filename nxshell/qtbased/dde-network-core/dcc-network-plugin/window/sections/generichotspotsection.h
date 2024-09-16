// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GENERICHOTSPOTSECTION_H
#define GENERICHOTSPOTSECTION_H

#include "abstractsection.h"
#include "interface/namespace.h"

#include <networkmanagerqt/connectionsettings.h>

namespace DCC_NAMESPACE {
class LineEditWidget;
}

class GenericHotspotSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit GenericHotspotSection(NetworkManager::ConnectionSettings::Ptr connSettings, QFrame *parent = nullptr);
    virtual ~GenericHotspotSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initUI();
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    DCC_NAMESPACE::LineEditWidget *m_connIdItem;
    NetworkManager::ConnectionSettings::Ptr m_connSettings;
};

#endif /* GENERICHOTSPOTSECTION_H */
