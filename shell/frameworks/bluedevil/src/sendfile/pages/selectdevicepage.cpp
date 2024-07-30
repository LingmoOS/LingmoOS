/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "selectdevicepage.h"
#include "../discoverwidget.h"
#include "../sendfilewizard.h"

#include <QDesktopServices>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

#include <BluezQt/Device>

SelectDevicePage::SelectDevicePage(SendFileWizard *wizard)
    : QWizardPage(wizard)
    , m_wizard(wizard)
{
    setupUi(this);

    DiscoverWidget *widget = new DiscoverWidget(m_wizard->manager(), this);
    widget->setContentsMargins(0, 0, 0, 0);
    discoverLayout->addWidget(widget);

    selectBtn->setHidden(true);
    selectLbl->setHidden(true);
    connect(widget, &DiscoverWidget::deviceSelected, this, &SelectDevicePage::deviceSelected);
}

void SelectDevicePage::deviceSelected(BluezQt::DevicePtr device)
{
    m_wizard->setDevice(device);

    Q_EMIT completeChanged();
}

bool SelectDevicePage::isComplete() const
{
    if (!m_wizard->device()) {
        return false;
    }
    return true;
}

#include "moc_selectdevicepage.cpp"
