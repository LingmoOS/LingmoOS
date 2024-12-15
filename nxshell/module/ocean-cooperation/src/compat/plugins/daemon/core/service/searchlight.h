// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QReadWriteLock>
#include <QMap>
#include <QSharedPointer>
#include <QMutex>

#include <co/time.h>
#include <co/stl.h>
#include <memory>
#include <functional>

namespace searchlight {

#ifdef _WIN32
  #define __disapi __declspec(dllexport)
#else
  #define __disapi __attribute__((visibility("default")))
#endif

class __disapi Discoverer {

public:
    /*!
    * Represents a discovered service
    * */
    struct service
    {
        fastring service_name; // the name of the service
        fastring endpoint; //sockaddr 地址转换成 "ip:port" 形式的字符串
        fastring info; //json 格式的服务节点信息
        qint8 flags; // 0是服务器上线，1服务器下线，2信息改变
        int64_t last_seen; //last see time

        bool operator<(const service& o) const
        {
            // last_seen is ignored
            return std::tie(service_name, endpoint) <
                std::tie(o.service_name, o.endpoint);
        }

        bool operator==(const service& o) const
        {
            // again, last_seen is ignored
            return std::tie(service_name, endpoint) ==
                std::tie(o.service_name, o.endpoint);
        }

        // this uses "name injection"
        friend fastream& operator<<(fastream& os, const Discoverer::service& service)
        {
            os << service.service_name << " on " << "(" << service.endpoint << "): " <<
                service.info << " == flags = " << int(service.flags) <<
                  " , time = " << service.last_seen;
            return os;
        }
    };

    /// a set of discovered services
    typedef QMap<QString, QSharedPointer<searchlight::Discoverer::service>> services;

    /// this callback gets called, when ever the set of available services changes
    typedef std::function<void(const QList<service>& services)> on_services_changed_t;

    Discoverer(const fastring& listen_for_service, // the service to watch out for
               const on_services_changed_t on_services_changed // callback discovered services changes
              );
    ~Discoverer();

    void start();

    bool started();

    void exit();

    void setSearchIp(const QString &ip);

    void handle_message(const fastring& message, const fastring& sender_endpoint,
                        const bool isFilter = true);

private:
    bool remove_idle_services();
    void handleChanges(const QString &endpoint, const fastring &info, const qint64 time);

    bool _stop = true;

    co::Timer _timer;
    const fastring _listen_for_service;
    const on_services_changed_t _on_services_changed;

    QReadWriteLock _discovered_lock;
    services _discovered_services;
    QList<service> _change_sevices;

    DISALLOW_COPY_AND_ASSIGN(Discoverer);
};


class __disapi Announcer {
public:
    Announcer(const fastring& service_name, // the name of the announced service
              const uint16_t service_port, // the port where the service listens on
              const fastring& info // the extend info will be announced
              );
    ~Announcer();

    // update the announce info
    void updateBase(const fastring &info);

    fastring baseInfo() const;

    // append app's info
    void appendApp(const fastring &info);

    // remove app's info
    void removeApp(const fastring &info);

    // remove app's info by name
    void removeAppbyName(const fastring &name);

    typedef std::function<void(const QString &ip)> handleTcpDiscover;
    // start announce
    void start(handleTcpDiscover handle);

    bool started();

    // exit announce
    void exit();

    bool finished() const { return _finished; }

    fastring udpSendPackage();

    fastring nodeInfoStr();

private:
    int sameApp(const fastring &info);

private:
    bool _stop = true;
    bool _finished = false;

    const fastring _service_name;
    const uint16_t _service_port;

    fastring _base_info;
    co::vector<fastring> _app_infos;

    DISALLOW_COPY_AND_ASSIGN(Announcer);
};

} // searchlight
