/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pppwidget.h"
#include "ui_ppp.h"

#include <NetworkManagerQt/PppSetting>

PPPWidget::PPPWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::PPPWidget)
{
    m_ui->setupUi(this);

    // Connect for setting check
    watchChangedSetting();

    KAcceleratorManager::manage(this);

    if (setting) {
        loadConfig(setting);
    }
}

PPPWidget::~PPPWidget()
{
    delete m_ui;
}

void PPPWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::PppSetting::Ptr pppSetting = setting.staticCast<NetworkManager::PppSetting>();

    m_ui->eap->setChecked(!pppSetting->refuseEap());
    m_ui->pap->setChecked(!pppSetting->refusePap());
    m_ui->chap->setChecked(!pppSetting->refuseChap());
    m_ui->mschap->setChecked(!pppSetting->refuseMschap());
    m_ui->mschapv2->setChecked(!pppSetting->refuseMschapv2());

    m_ui->mppe->setChecked(pppSetting->requireMppe());
    m_ui->mppe128->setChecked(pppSetting->requireMppe128());
    m_ui->mppeStateful->setChecked(pppSetting->mppeStateful());

    m_ui->bsdComp->setChecked(!pppSetting->noBsdComp());
    m_ui->deflateComp->setChecked(!pppSetting->noDeflate());
    m_ui->tcpComp->setChecked(!pppSetting->noVjComp());

    if (pppSetting->lcpEchoInterval() > 0) {
        m_ui->senddEcho->setChecked(true);
    } else {
        m_ui->senddEcho->setChecked(false);
    }
}

QVariantMap PPPWidget::setting() const
{
    NetworkManager::PppSetting pppSetting;

    pppSetting.setRefuseEap(!m_ui->eap->isChecked());
    pppSetting.setRefusePap(!m_ui->pap->isChecked());
    pppSetting.setRefuseChap(!m_ui->chap->isChecked());
    pppSetting.setRefuseMschap(!m_ui->mschap->isChecked());
    pppSetting.setRefuseMschapv2(!m_ui->mschapv2->isChecked());

    pppSetting.setRequireMppe(m_ui->mppe->isChecked());
    pppSetting.setRequireMppe128(m_ui->mppe128->isChecked());
    pppSetting.setMppeStateful(m_ui->mppeStateful->isChecked());

    pppSetting.setNoBsdComp(!m_ui->bsdComp->isChecked());
    pppSetting.setNoDeflate(!m_ui->deflateComp->isChecked());
    pppSetting.setNoVjComp(!m_ui->tcpComp->isChecked());

    if (m_ui->senddEcho->isChecked()) {
        pppSetting.setLcpEchoFailure(5);
        pppSetting.setLcpEchoInterval(30);
    }

    return pppSetting.toMap();
}

#include "moc_pppwidget.cpp"
