// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PPPSECTION_H
#define PPPSECTION_H

#include "abstractsection.h"

#include <networkmanagerqt/pppsetting.h>

namespace DCC_NAMESPACE {
class SwitchWidget;
}

class PPPSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit PPPSection(NetworkManager::PppSetting::Ptr pppSetting, QFrame *parent = nullptr);
    virtual ~PPPSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initStrMaps();
    void initUI();
    void initConnection();
    void onMppeEnableChanged(const bool checked);

private:
    QMap<QString, QString> OptionsStrMap;

    NetworkManager::PppSetting::Ptr m_pppSetting;

    DCC_NAMESPACE::SwitchWidget *m_mppeEnable;
    DCC_NAMESPACE::SwitchWidget *m_mppe128;
    DCC_NAMESPACE::SwitchWidget *m_mppeStateful;
    DCC_NAMESPACE::SwitchWidget *m_refuseEAP;
    DCC_NAMESPACE::SwitchWidget *m_refusePAP;
    DCC_NAMESPACE::SwitchWidget *m_refuseCHAP;
    DCC_NAMESPACE::SwitchWidget *m_refuseMSCHAP;
    DCC_NAMESPACE::SwitchWidget *m_refuseMSCHAP2;
    DCC_NAMESPACE::SwitchWidget *m_noBSDComp;
    DCC_NAMESPACE::SwitchWidget *m_noDeflate;
    DCC_NAMESPACE::SwitchWidget *m_noVJComp;
    DCC_NAMESPACE::SwitchWidget *m_lcpEchoInterval;
};

#endif /* PPPSECTION_H */
