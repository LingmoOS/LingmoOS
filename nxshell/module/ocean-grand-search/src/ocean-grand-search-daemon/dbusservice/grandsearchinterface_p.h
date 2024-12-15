// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHINTERFACE_P_H
#define GRANDSEARCHINTERFACE_P_H

#include "grandsearchinterface.h"
#include "maincontroller/maincontroller.h"

#include <QTimer>
#include <QDBusMessage>

namespace GrandSearch {

class GrandSearchInterfacePrivate : public QObject
{
    Q_OBJECT
    friend class GrandSearchInterface;
public:
    explicit GrandSearchInterfacePrivate(GrandSearchInterface *parent);
    ~GrandSearchInterfacePrivate();

    inline bool vaildSession(const QString &session) {
        if (session.isEmpty())
            return false;
        return m_session == session;
    }
    bool isAccessable(const QDBusMessage &msg) const;
private slots:
    void terminate();
    void onMatched();
    void onSearchCompleted();
private:
    GrandSearchInterface *q;
    MainController *m_main = nullptr;
    QString m_session;
    QTimer m_deadline;
    QHash<QString, bool> m_permit;
};

}

#endif // GRANDSEARCHINTERFACE_P_H
