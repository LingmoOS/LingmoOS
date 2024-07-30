/*
    SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "ui_discover.h"

#include <QWizardPage>

#include <BluezQt/Types>

class KMessageWidget;

class BlueWizard;
class DevicesProxyModel;

class DiscoverPage : public QWizardPage, public Ui::Discover
{
    Q_OBJECT

public:
    explicit DiscoverPage(BlueWizard *parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;
    int nextId() const override;

protected:
    void showEvent(QShowEvent *event) override;

private Q_SLOTS:
    void indexSelected(const QModelIndex &index);
    void itemDoubleClicked(const QModelIndex &index);

    void usableAdapterChanged(BluezQt::AdapterPtr adapter);
    void checkAdapters();
    void fixAdaptersError();

private:
    BlueWizard *const m_wizard;
    DevicesProxyModel *const m_model;
    BluezQt::Manager *m_manager = nullptr;
    BluezQt::AdapterPtr m_adapter;
    KMessageWidget *m_warningWidget = nullptr;
};
