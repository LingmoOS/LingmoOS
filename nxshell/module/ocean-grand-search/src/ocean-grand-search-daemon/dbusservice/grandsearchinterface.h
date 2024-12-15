// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHINTERFACE_H
#define GRANDSEARCHINTERFACE_H

#include <qdbusservice.h>

#include <QObject>
#include <QDBusContext>
#include <QMap>

//后端程序DBus服务
#define GrandSearchServiceName          "org.lingmo.ocean.GrandSearchDaemon"
#define GrandSearchServicePath          "/org/lingmo/ocean/GrandSearchDaemon"
#define GrandSearchServiceInterface     "org.lingmo.ocean.GrandSearchDaemon"

namespace GrandSearch {

class GrandSearchInterfacePrivate;
class GrandSearchInterface : public QDBusService, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", GrandSearchServiceInterface)
    friend class GrandSearchInterfacePrivate;
public:
    explicit GrandSearchInterface(QObject *parent = nullptr);
    ~GrandSearchInterface();
    bool init();
public slots:
    Q_SCRIPTABLE bool Search(const QString &session, const QString &key);
    Q_SCRIPTABLE void Terminate();
    Q_SCRIPTABLE QByteArray MatchedResults(const QString &session);
    Q_SCRIPTABLE QByteArray MatchedBuffer(const QString &session);
    Q_SCRIPTABLE bool OpenWithPlugin(const QString &searcher, const QString &item);
//    Q_SCRIPTABLE bool Configure(const QVariantMap &);
//    Q_SCRIPTABLE QVariantMap Configuration() const;
//    Q_SCRIPTABLE bool SetFeedBackStrategy(const QVariantMap &);
//    Q_SCRIPTABLE QVariantMap FeedBackStrategy() const;
    Q_SCRIPTABLE bool KeepAlive(const QString &session);
signals:
    Q_SCRIPTABLE void Matched(const QString &session);
    Q_SCRIPTABLE void SearchCompleted(const QString &session);
private:
    GrandSearchInterfacePrivate *d;
};

}

#endif // GRANDSEARCHINTERFACE_H
