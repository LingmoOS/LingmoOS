// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCONFIGHIOCEANNMENUSCENE_H
#define DCONFIGHIOCEANNMENUSCENE_H

#include "dfmplugin_menu_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

DPMENU_BEGIN_NAMESPACE

class DConfigHioceannMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "DConfigMenuFilter";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class DConfigHioceannMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit DConfigHioceannMenuScene(QObject *parent = nullptr);

    virtual QString name() const override;
    bool initialize(const QVariantHash &params) override;
    void updateState(QMenu *parent) override;

private:
    void disableScene();
    void updateActionHioceann(QMenu *parent);
};

DPMENU_END_NAMESPACE

#endif   // DCONFIGHIOCEANNMENUSCENE_H
