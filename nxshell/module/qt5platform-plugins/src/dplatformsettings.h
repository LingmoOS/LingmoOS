// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMSETTINGS_H
#define DPLATFORMSETTINGS_H

#include "global.h"

#include <QVariant>

QT_BEGIN_NAMESPACE
class QByteArray;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DPlatformSettings
{
public:
    virtual ~DPlatformSettings() {}

    virtual bool initialized() const { return true; }
    virtual bool isEmpty() const { return false; }

    virtual bool contains(const QByteArray &property) const = 0;
    virtual QVariant setting(const QByteArray &property) const = 0;
    virtual void setSetting(const QByteArray &property, const QVariant &value) = 0;
    virtual QByteArrayList settingKeys() const = 0;

    virtual void emitSignal(const QByteArray &signal, qint32 data1, qint32 data2) = 0;

    typedef void (*PropertyChangeFunc)(const QByteArray &name, const QVariant &property, void *handle);
    void registerCallback(PropertyChangeFunc func, void *handle);
    void removeCallbackForHandle(void *handle);
    typedef void (*SignalFunc)(const QByteArray &signal, qint32 data1, qint32 data2, void *handle);
    void registerSignalCallback(SignalFunc func, void *handle);
    void removeSignalCallback(void *handle);

protected:
    void handlePropertyChanged(const QByteArray &property, const QVariant &value);
    void handleNotify(const QByteArray &signal, qint32 data1, qint32 data2);

    struct Q_DECL_HIDDEN Callback
    {
        PropertyChangeFunc func;
        void *handle;
    };

    struct Q_DECL_HIDDEN SignalCallback
    {
        SignalFunc func;
        void *handle;
    };

    std::vector<Callback> callback_links;
    std::vector<SignalCallback> signal_callback_links;
};

DPP_END_NAMESPACE

#endif // DPLATFORMSETTINGS_H
