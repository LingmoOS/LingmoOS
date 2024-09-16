// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DimDBusAdaptor.h"

#include "dbus/DimDBusType.h"

DimDBusAdaptor::DimDBusAdaptor(Dim *parent)
    : QObject(parent)
{
    registerDimQtDBusTypes();
}

DimDBusAdaptor::~DimDBusAdaptor() { }

void DimDBusAdaptor::AddInputMethod(const QString &addon, const QString &name)
{
    parent()->addInputMethod(addon.toStdString(), name.toStdString());

    Q_EMIT InputMethodsChanged();
}

void DimDBusAdaptor::RemoveInputMethod(const QString &addon, const QString &name)
{
    parent()->removeInputMethod(addon.toStdString(), name.toStdString());

    Q_EMIT InputMethodsChanged();
}

bool DimDBusAdaptor::SetCurrentInputMethod(const QString &addon, const QString &name)
{
    auto ret = parent()->requestSwitchIM(addon.toStdString(), name.toStdString());

    if (ret) {
        Q_EMIT CurrentInputMethodChanged(InputMethodData{ addon, name });
    }

    return ret;
}

InputMethodData DimDBusAdaptor::GetCurrentInputMethod()
{
    const auto imKey = parent()->getCurrentActiveInputMethod();
    return InputMethodData{ QString::fromStdString(imKey.first),
                            QString::fromStdString(imKey.second) };
}

InputMethodEntryList DimDBusAdaptor::GetAvailableInputMethods()
{
    InputMethodEntryList availableInputMethods = InputMethodEntryList{};
    for (const auto &entry : parent()->imEntries()) {
        availableInputMethods.append(
            ::InputMethodEntry{ QString::fromStdString(entry.addonKey()),
                                QString::fromStdString(entry.uniqueName()),
                                QString::fromStdString(entry.name()),
                                QString::fromStdString(entry.description()),
                                QString::fromStdString(entry.label()),
                                QString::fromStdString(entry.iconName()) });
    }

    return availableInputMethods;
}

InputMethodDataList DimDBusAdaptor::GetCurrentInputMethods()
{
    InputMethodDataList currentActiveIMs = InputMethodDataList{};
    for (const auto &im : parent()->activeInputMethodEntries()) {
        currentActiveIMs.append(
            InputMethodData{ QString::fromStdString(im.first), QString::fromStdString(im.second) });
    }

    return currentActiveIMs;
}

// TODO: implement
void DimDBusAdaptor::Configure(const QString &addon, const QString &name) { }

bool DimDBusAdaptor::SetInputMethodShortcut(const QString &keyStroke)
{
    return false;
}

bool DimDBusAdaptor::SetInputMethodOrderShortcut(const QString &keyStroke)
{
    return false;
}

QDBusVariant DimDBusAdaptor::GetConfig(const QString &addon, const QString &name)
{
    return QDBusVariant();
}

void DimDBusAdaptor::SetConfig(const QString &addon,
                               const QString &name,
                               const QDBusVariant &config)
{
}

void DimDBusAdaptor::Toggle()
{
    parent()->toggle();
}
