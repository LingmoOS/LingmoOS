// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNPPPSECTION_H
#define VPNPPPSECTION_H

#include "sections/abstractsection.h"

#include <networkmanagerqt/vpnsetting.h>

namespace DCC_NAMESPACE {
class SwitchWidget;
class ComboxWidget;
}

using namespace NetworkManager;

class VpnPPPSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnPPPSection(VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnPPPSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

    void setSupportOptions(const QStringList &supportOptions);

private:
    void initStrMaps();
    void initUI();
    void initConnection();
    void onMppeEnableChanged(const bool checked);
    void onMppeMethodChanged(const QString &methodKey);

private:
    QList<QPair<QString, QString>> MppeMethodStrMap;
    QMap<QString, QString> OptionsStrMap;

    VpnSetting::Ptr m_vpnSetting;
    NMStringMap m_dataMap;

    DCC_NAMESPACE::SwitchWidget *m_mppeEnable;
    DCC_NAMESPACE::ComboxWidget *m_mppeChooser;
    DCC_NAMESPACE::SwitchWidget *m_mppeStateful;

    QString m_currentMppeMethod;
    QStringList m_supportOptions;
    QList<DCC_NAMESPACE::SwitchWidget *> m_optionsWidgets;
};

#endif /* VPNPPPSECTION_H */
