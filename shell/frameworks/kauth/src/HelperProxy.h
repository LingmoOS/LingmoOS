/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KAUTH_HELPER_PROXY_H
#define KAUTH_HELPER_PROXY_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

#include "action.h"
#include "actionreply.h"

namespace KAuth
{
typedef Action::DetailsMap DetailsMap;

class HelperProxy : public QObject
{
    Q_OBJECT

public:
    ~HelperProxy() override;

    // Application-side methods
    virtual void executeAction(const QString &action, const QString &helperID, const DetailsMap &details, const QVariantMap &arguments, int timeout) = 0;
    virtual void stopAction(const QString &action, const QString &helperID) = 0;

    // Helper-side methods
    virtual bool initHelper(const QString &name) = 0;
    virtual void setHelperResponder(QObject *o) = 0;
    virtual bool hasToStopAction() = 0;
    virtual void sendDebugMessage(int level, const char *msg) = 0;
    virtual void sendProgressStep(int step) = 0;
    virtual void sendProgressStepData(const QVariantMap &step) = 0;
    // Attempts to resolve the UID of the unprivileged remote process.
    virtual int callerUid() const = 0;

Q_SIGNALS:
    void actionStarted(const QString &action);
    void actionPerformed(const QString &action, const KAuth::ActionReply &reply);
    void progressStep(const QString &action, int progress);
    void progressStepData(const QString &action, const QVariantMap &data);
};

} // namespace KAuth

Q_DECLARE_INTERFACE(KAuth::HelperProxy, "org.kde.kf6auth.HelperProxy/0.1")

#endif
