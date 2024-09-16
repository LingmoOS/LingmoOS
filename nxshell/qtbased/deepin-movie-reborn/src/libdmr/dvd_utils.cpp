// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dvd_utils.h"

#include <dvdnav/dvdnav.h>

namespace dmr {
//add by xxj
#ifdef heyi
namespace dvd {

/*QString RetrieveDVDTitle(const QString &device)
{
    qInfo() << "device" << device;
    const char *title = nullptr;

    dvdnav_t *handle = nullptr;
    auto res = dvdnav_open(&handle, device.toUtf8().constData());
    if (res == DVDNAV_STATUS_ERR) {
        qWarning() << "dvdnav open " << device << "failed";
        return "";
    }

    int32_t nr_titles = 0;
    res = dvdnav_get_number_of_titles(handle, &nr_titles);
    if (res == DVDNAV_STATUS_ERR) {
        goto on_error;
    }

    res = dvdnav_get_title_string(handle, &title);
    if (res == DVDNAV_STATUS_ERR) {
        goto on_error;
    }

#if 0
    uint64_t max_duration = -1;
    QString title = "";
    //uint32_t dvdnav_describe_title_chapters(dvdnav_t *self, int32_t title, uint64_t **times, uint64_t *duration);
    for (int i = 0; i < nr_titles; i++) {
        uint64_t duration = 0;
        auto n = dvdnav_describe_title_chapters(handle, i, NULL, &duration);
        if (max_duration < duration) {
            max_duration = duration;
            //title
        }
    }
#endif
    if (handle) dvdnav_close(handle);
    return QString::fromUtf8(title);

on_error:
    qWarning() << dvdnav_err_to_string(handle);
    if (handle) dvdnav_close(handle);
    return "";
}*/

static std::atomic<RetrieveDvdThread *> _instance { nullptr };
static QMutex _instLock;
static QMutex _startLock;
static QMutex _runLock;
static QWaitCondition cond;

RetrieveDvdThread::RetrieveDvdThread()
{

}

RetrieveDvdThread::~RetrieveDvdThread()
{
    this->requestInterruption();
    this->quit();
    this->wait();
    delete &_instance;
    _instance = nullptr;
}

RetrieveDvdThread *RetrieveDvdThread::get()
{
    if (_instance == nullptr) {
        QMutexLocker lock(&_instLock);
//        if (_instance == nullptr) {
            _instance = new RetrieveDvdThread;
//        }
    }
    return _instance;
}

void RetrieveDvdThread::startDvd(const QString &dev)
{
    m_dev = dev;
    QMutexLocker lock(&_runLock);
    start();
}

void RetrieveDvdThread::run()
{
    setPriority(QThread::IdlePriority);

    do {
        QMutexLocker lock(&_runLock);
        while (m_dev.isEmpty() && !_quit.load()) {
            cond.wait(lock.mutex(), 40);
        }

        if (_quit.load()) break;

        auto title = getDvdMsg(m_dev);
        qInfo() << "-----" << title;
        emit sigData(title);
    } while (false);
}

QString RetrieveDvdThread::getDvdMsg(const QString &device)
{
    qInfo() << "device" << device;
    const char *title = nullptr;

    dvdnav_t *handle = nullptr;
    int32_t res = 0;
#ifndef __mips__
    res = dvdnav_open(&handle, device.toUtf8().constData());
    if (res == DVDNAV_STATUS_ERR) {
        qCritical() << "dvdnav open " << device << "failed";
        qCritical() << dvdnav_err_to_string(handle);
        if (handle) dvdnav_close(handle);
        return "dvd open failed";
    }
#endif
    int32_t nr_titles = 0;
    res = dvdnav_get_number_of_titles(handle, &nr_titles);
    if (res == DVDNAV_STATUS_ERR) {
        goto on_error;
    }

    res = dvdnav_get_title_string(handle, &title);
    if (res == DVDNAV_STATUS_ERR) {
        goto on_error;
    }

#if 0
    uint64_t max_duration = -1;
    QString title = "";
    //uint32_t dvdnav_describe_title_chapters(dvdnav_t *self, int32_t title, uint64_t **times, uint64_t *duration);
    for (int i = 0; i < nr_titles; i++) {
        uint64_t duration = 0;
        auto n = dvdnav_describe_title_chapters(handle, i, NULL, &duration);
        if (max_duration < duration) {
            max_duration = duration;
            //title
        }
    }
#endif
    if (handle) dvdnav_close(handle);
    return QString::fromUtf8(title);

on_error:
    qWarning() << dvdnav_err_to_string(handle);
    if (handle) dvdnav_close(handle);
    return "";
}

}
#endif
}


