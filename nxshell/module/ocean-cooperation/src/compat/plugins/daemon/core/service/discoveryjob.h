// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISCOVERYJOB_H
#define DISCOVERYJOB_H

#include <QObject>
#include <QReadWriteLock>
#include <co/stl.h>
#include <QMutex>

class DiscoveryJob : public QObject
{
    Q_OBJECT
public:
    void discovererRun();
    void announcerRun(const fastring &info);
    void stopDiscoverer();
    void stopAnnouncer();

    void updateAnnouncBase(const fastring info);
    void updateAnnouncApp(bool remove, const fastring info);
    void updateAnnouncShare(const bool remove, const fastring connectIP = "");
    void removeAppbyName(const fastring name);
    fastring baseInfo() const;

    co::list<fastring> getNodes();

    static DiscoveryJob *instance()
    {
        static DiscoveryJob ins;
        return &ins;
    }

    void searchDeviceByIp(const QString &ip, const bool remove);

    fastring udpSendPackage();

    fastring nodeInfoStr();

    void handleUpdPackage(const QString &ip, const QString &msg);

signals:
    void sigNodeChanged(bool found, QString info);

private:
    void compareOldAndNew(const fastring &uid, const QString &cur,
                          const co::lru_map<fastring, std::pair<fastring, bool>>::iterator &it);

    void notifySearchResult(bool result, const fastring &info);

private:
    explicit DiscoveryJob(QObject *parent = nullptr);
    virtual ~DiscoveryJob();
    void *_discoverer_p = { nullptr };
    void *_announcer_p = { nullptr };

    //<uuid, <peerinfo, exist>>
    QReadWriteLock _dis_lock;
    co::lru_map<fastring, std::pair<fastring, bool>> _dis_node_maps;
    mutable QMutex _lock;
};

#endif // DISCOVERYJOB_H
