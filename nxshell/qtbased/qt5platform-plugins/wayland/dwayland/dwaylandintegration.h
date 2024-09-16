// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWAYLANDINTEGRATION_H
#define DWAYLANDINTEGRATION_H

#include "global.h"
#include "QtWaylandClient/private/qwaylandintegration_p.h"

DPP_BEGIN_NAMESPACE

class DWaylandIntegration : public QtWaylandClient::QWaylandIntegration
{
    DWaylandIntegration();
public:
    static DWaylandIntegration *instance() {
        static DWaylandIntegration *integration = new DWaylandIntegration;
        return integration;
    }

    void initialize() override;
    QStringList themeNames() const override;
    QVariant styleHint(StyleHint hint) const override;

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    void setPrimaryScreen(QPlatformScreen *newPrimary);
#endif
};

DPP_END_NAMESPACE

#endif // DWAYLANDINTEGRATION_H
