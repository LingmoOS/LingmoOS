// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNOTIFYSENDER_H
#define DNOTIFYSENDER_H

#include "dtkcore_global.h"

#include <QDBusPendingCall>
#include <memory>

DCORE_BEGIN_NAMESPACE

namespace DUtil {
struct DNotifyData;
class LIBDTKCORESHARED_EXPORT DNotifySender {
public:
    DNotifySender(const QString &summary);
    DNotifySender    appName(const QString &appName = QString());
    DNotifySender    appIcon(const QString &appIcon = QString());
    DNotifySender    appBody(const QString &appBody = QString());
    DNotifySender    replaceId(const uint replaceId = 0);
    DNotifySender    timeOut(const int timeOut = -1);
    DNotifySender    actions(const QStringList &actions = QStringList());
    DNotifySender    hints(const QVariantMap &hints = QVariantMap());
    QDBusPendingCall call();

private:
    std::shared_ptr<DNotifyData> m_dbusData;
};
}  // namespace DUtil

DCORE_END_NAMESPACE

#endif  // DNOTIFYSENDER_H
