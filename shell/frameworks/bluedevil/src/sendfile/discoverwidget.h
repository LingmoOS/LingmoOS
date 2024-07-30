/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "ui_discover.h"

#include <QWidget>

#include <BluezQt/Types>

class KMessageWidget;

class DevicesProxyModel;

class DiscoverWidget : public QWidget, public Ui::Discover
{
    Q_OBJECT

public:
    explicit DiscoverWidget(BluezQt::Manager *manager, QWidget *parent = nullptr);

Q_SIGNALS:
    void deviceSelected(BluezQt::DevicePtr device);

private Q_SLOTS:
    void indexSelected(const QModelIndex index);

    void checkAdapters();
    void fixAdaptersError();

private:
    BluezQt::Manager *m_manager;
    DevicesProxyModel *m_model;
    KMessageWidget *m_warningWidget;
};
