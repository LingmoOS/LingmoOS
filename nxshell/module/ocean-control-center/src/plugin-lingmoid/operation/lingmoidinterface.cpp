// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "oceanuifactory.h"
#include "lingmoidinterface.h"
#include "utils.h"

LingmoIDInterface::LingmoIDInterface(QObject *parent)
    : QObject(parent)
    , m_model(new LingmoidModel(this))
    , m_worker(new LingmoWorker(m_model, this))
{
    m_worker->initData();
}

QString LingmoIDInterface::editionName() const
{
    if (IsCommunitySystem) {
        return tr("lingmo");
    } else {
        return tr("UOS");
    }
}

DCC_FACTORY_CLASS(LingmoIDInterface)

#include "lingmoidinterface.moc"
