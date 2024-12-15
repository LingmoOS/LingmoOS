// SPDX-FileCopyrightText: 2010 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HOMEPAGECHECKMODULE_H
#define HOMEPAGECHECKMODULE_H

#include "window/interface/moduleinterface.h"
#include "window/interface/frameproxyinterface.h"
#include "src/window/modules/common/common.h"
#include "window/modules/homepagecheck/homepagecheckwidget.h"


#include <DGuiApplicationHelper>

class FrameProxyInterface;
class ModuleInterface;

class HomePageCheckModule : public QObject, public ModuleInterface
{
    Q_OBJECT
public:
    explicit HomePageCheckModule(FrameProxyInterface *frame, QObject *parent = nullptr);
    ~HomePageCheckModule() override {}

public:
    void initialize() override;
    void preInitialize() override;
    const QString name() const override;
    void active(int index) override;
    void deactive() override;

Q_SIGNALS:

public Q_SLOTS:

private:
    HomePageCheckWidget  *m_homePagecheckWidget = nullptr;
};

#endif // HOMEPAGECHECKMODULE_H
