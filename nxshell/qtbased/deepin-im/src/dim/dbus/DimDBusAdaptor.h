// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIM_DBUS_ADAPTOR_
#define DIM_DBUS_ADAPTOR_

#include "DimDBusType.h"
#include "dimcore/Dim.h"

#include <QDBusContext>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QObject>

using namespace org::deepin::dim;

class DimDBusAdaptor : public QObject, public QDBusContext
{
    Q_OBJECT

public:
    DimDBusAdaptor(Dim *parent);
    DimDBusAdaptor(const DimDBusAdaptor &) = delete;
    DimDBusAdaptor(DimDBusAdaptor &&) = delete;
    DimDBusAdaptor &operator=(const DimDBusAdaptor &) = delete;
    DimDBusAdaptor &operator=(DimDBusAdaptor &&) = delete;
    ~DimDBusAdaptor() override;

    inline Dim *parent() const { return dynamic_cast<Dim *>(QObject::parent()); }

public Q_SLOTS:
    // DBus Methods
    void AddInputMethod(const QString &addon, const QString &name);
    void RemoveInputMethod(const QString &addon, const QString &name);
    bool SetCurrentInputMethod(const QString &addon, const QString &name);
    InputMethodData GetCurrentInputMethod();
    InputMethodDataList GetCurrentInputMethods();
    InputMethodEntryList GetAvailableInputMethods();
    void Configure(const QString &addon, const QString &name);
    bool SetInputMethodShortcut(const QString &keyStroke);
    bool SetInputMethodOrderShortcut(const QString &keyStroke);
    QDBusVariant GetConfig(const QString &addon, const QString &name);
    void SetConfig(const QString &addon, const QString &name, const QDBusVariant &config);
    void Toggle();

signals:
    // signals on dbus
    void InputMethodsChanged();
    void CurrentInputMethodChanged(InputMethodData inputMethodEntry);
};

#endif // DIM_DBUS_ADAPTOR_
