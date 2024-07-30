/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULEQML_H
#define KCMODULEQML_H

#include "kcmodule.h"

#include <memory>

class QQuickItem;
class QQmlEngine;
class KCModuleQmlPrivate;
class KQuickConfigModule;

class KCModuleQml : public KCModule
{
    Q_OBJECT

public:
    KCModuleQml(KQuickConfigModule *configModule, QWidget *parent);
    ~KCModuleQml() override;

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;
    QWidget *widget() override;

private:
    friend class QmlConfigModuleWidget;
    const std::unique_ptr<KCModuleQmlPrivate> d;

    Q_PRIVATE_SLOT(d, void syncCurrentIndex())
};

#endif
