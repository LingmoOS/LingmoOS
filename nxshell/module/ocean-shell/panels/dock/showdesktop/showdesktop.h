// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"
#include "dsglobal.h"
#include "treelandwindowmanager.h"

namespace dock {

class ShowDesktop : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
public:
    explicit ShowDesktop(QObject *parent = nullptr);
    virtual bool init() override;
    virtual bool load() override;

    Q_INVOKABLE void toggleShowDesktop();
    Q_INVOKABLE bool checkNeedShowDesktop();

private:
    TreelandWindowManager *m_windowManager;
};

}
