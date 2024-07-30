/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef EXTERNALAPPMODULE_H
#define EXTERNALAPPMODULE_H

#include <KService>
#include <QWidget>

#include "ui_externalModule.h"

class QShowEvent;

class ExternalAppModule : public QWidget
{
    Q_OBJECT

public:
    explicit ExternalAppModule(const KService::Ptr &service);
    ~ExternalAppModule() override;

protected:
    void showEvent(QShowEvent *event) override;

private Q_SLOTS:
    void runExternal();

private:
    const KService::Ptr moduleService;
    Ui::ExternalModule externalModule;
    bool firstShow;
};

#endif
