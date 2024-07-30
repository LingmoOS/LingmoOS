/*
    SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FAKEHELPERPROXY_H
#define FAKEHELPERPROXY_H

#include "HelperProxy.h"

namespace KAuth
{
class FakeHelperProxy : public HelperProxy
{
    Q_OBJECT
    Q_INTERFACES(KAuth::HelperProxy)

public:
    FakeHelperProxy();
    ~FakeHelperProxy() override;

    void sendProgressStepData(const QVariantMap &step) override;
    void sendProgressStep(int step) override;
    void sendDebugMessage(int level, const char *msg) override;
    bool hasToStopAction() override;
    void setHelperResponder(QObject *o) override;
    bool initHelper(const QString &name) override;
    void stopAction(const QString &action, const QString &helperID) override;
    void executeAction(const QString &action, const QString &helperID, const DetailsMap &details, const QVariantMap &arguments, int timeout = -1) override;
    int callerUid() const override;
};

}

#endif // FAKEHELPERPROXY_H
