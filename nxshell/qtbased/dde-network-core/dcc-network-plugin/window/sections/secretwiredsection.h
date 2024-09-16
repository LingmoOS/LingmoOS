// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SECRETWIREDSECTION_H
#define SECRETWIREDSECTION_H

#include "secret8021xsection.h"

namespace DCC_NAMESPACE {
class SwitchWidget;
}

class SecretWiredSection : public Secret8021xSection
{
    Q_OBJECT

public:
    SecretWiredSection(NetworkManager::Security8021xSetting::Ptr sSetting, QFrame *parent = nullptr);
    virtual ~SecretWiredSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    DCC_NAMESPACE::SwitchWidget *m_secretEnable;
    Secret8021xEnableWatcher *m_enableWatcher;
};

#endif /* SECRETWIREDSECTION_H */
