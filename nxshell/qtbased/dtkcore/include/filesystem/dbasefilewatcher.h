// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBASEFILEWATCHER_H
#define DBASEFILEWATCHER_H

#include "dtkcore_global.h"
#include "dobject.h"

#include <QObject>

DCORE_BEGIN_NAMESPACE

class DBaseFileWatcherPrivate;
class LIBDTKCORESHARED_EXPORT DBaseFileWatcher : public QObject, public DObject
{
    Q_OBJECT

public:
    ~DBaseFileWatcher();

    QUrl fileUrl() const;

    bool startWatcher();
    bool stopWatcher();
    bool restartWatcher();

    virtual void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true);

    using SignalType1 = void(DBaseFileWatcher::*)(const QUrl &);
    using SignalType2 = void(DBaseFileWatcher::*)(const QUrl &, const QUrl &);
    static bool ghostSignal(const QUrl &targetUrl, SignalType1 signal, const QUrl &arg1);
    static bool ghostSignal(const QUrl &targetUrl, SignalType2 signal, const QUrl &arg1, const QUrl &arg2);

Q_SIGNALS:
    void fileDeleted(const QUrl &url);
    void fileAttributeChanged(const QUrl &url);
    void fileMoved(const QUrl &fromUrl, const QUrl &toUrl);
    void subfileCreated(const QUrl &url);
    void fileModified(const QUrl &url);
    void fileClosed(const QUrl &url);

protected:
    explicit DBaseFileWatcher(DBaseFileWatcherPrivate &dd, const QUrl &url, QObject *parent = 0);

private:
    Q_DISABLE_COPY(DBaseFileWatcher)
    D_DECLARE_PRIVATE(DBaseFileWatcher)
};

DCORE_END_NAMESPACE

#endif // DBASEFILEWATCHER_H
