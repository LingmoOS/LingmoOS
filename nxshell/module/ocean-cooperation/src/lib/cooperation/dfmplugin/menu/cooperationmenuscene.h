// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONMENUSCENE_H
#define COOPERATIONMENUSCENE_H

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_cooperation {

class CooperationMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "CooperationMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class CooperationMenuScenePrivate;
class CooperationMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit CooperationMenuScene(QObject *parent = nullptr);
    ~CooperationMenuScene() override;

    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    QScopedPointer<CooperationMenuScenePrivate> d;
};

}   // namespace dfmplugin_cooperation

#endif   // COOPERATIONMENUSCENE_H
