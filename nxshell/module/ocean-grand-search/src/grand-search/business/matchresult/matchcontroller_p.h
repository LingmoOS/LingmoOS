// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MATCHCONTROLLER_P_H
#define MATCHCONTROLLER_P_H

#include "matchcontroller.h"

#include <QAtomicInteger>

namespace GrandSearch {

class DaemonGrandSearchInterface;
class MatchControllerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit MatchControllerPrivate(MatchController *parent = nullptr);

public slots:
    void onMatched(const QString &missionId);
    void onSearchCompleted(const QString &missionId);
    void sendCacheItems();

public:
    void initConnect();

public:
    MatchController *q_p = nullptr;

    QString m_missionId;
    QString m_missionContent;

    QAtomicInteger<bool> m_missionIdChanged = true;
    MatchedItemMap m_cacheItems;
    bool m_enableBestMatch = true;
    int m_firstItemLimit = 30;
    int m_firstWaitTime = 500;
    int m_bestItemMaxCount = 4;
    QSharedPointer<QTimer> m_waitTimer = nullptr;

    DaemonGrandSearchInterface *m_daemonDbus = nullptr;
};

}

#endif // MATCHCONTROLLER_P_H
