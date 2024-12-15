// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "oceanuiinsider.h"

#include "ocean-control-center/oceanuifactory.h"
#include "insiderworker.h"

namespace ocean {
namespace insider {
OceanUIInsider::OceanUIInsider(QObject *parent)
    : QObject(parent)
    , m_insider(new InsiderWorker(this))
{
    m_currentItems.append({ m_insider->displayManager() });
    connect(m_insider, &InsiderWorker::displayManagerChanged, this, &OceanUIInsider::updateCurrentItem);
}

OceanUIInsider::~OceanUIInsider() { }

QStringList OceanUIInsider::currentItems() const
{
    return m_currentItems;
}

void OceanUIInsider::setCurrentItem(const QString &item)
{
    if (item == "lightdm" || item == "treeland") {
        m_insider->setDisplayManager(item);
    }
}

void OceanUIInsider::updateCurrentItem(const QString &item)
{
    if (item == "lightdm") {
        m_currentItems.removeOne("treeland");
        m_currentItems.append("lightdm");
    } else if (item == "treeland") {
        m_currentItems.removeOne("lightdm");
        m_currentItems.append("treeland");
    }
    Q_EMIT currentItemsChanged(m_currentItems);
}
DCC_FACTORY_CLASS(OceanUIInsider)
} // namespace insider
} // namespace ocean

#include "oceanuiinsider.moc"
