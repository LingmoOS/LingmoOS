/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "InputDevice_interface.h"

#include <QObject>
#include <QString>

#include <optional>

class InputDevice : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sysName READ sysName CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QSizeF size READ size CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool supportsLeftHanded READ supportsLeftHanded CONSTANT)
    Q_PROPERTY(bool leftHanded READ isLeftHanded WRITE setLeftHanded NOTIFY leftHandedChanged)

    Q_PROPERTY(bool supportsOrientation READ supportsOrientation CONSTANT)
    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QString outputName READ outputName WRITE setOutputName NOTIFY outputNameChanged)
    Q_PROPERTY(QRectF outputArea READ outputArea WRITE setOutputArea NOTIFY outputAreaChanged)

public:
    InputDevice(const QString &dbusName, QObject *parent);

    void load();
    void save();
    void defaults();
    bool isSaveNeeded() const;
    bool isDefaults() const;

    QString name() const
    {
        return m_name.value();
    }

    QString sysName() const
    {
        return m_sysName.value();
    }
    bool isLeftHanded() const
    {
        return m_leftHanded.value();
    }
    bool supportsLeftHanded() const
    {
        return m_leftHanded.isSupported();
    }
    void setLeftHanded(bool set);

    bool supportsOrientation() const
    {
        return m_orientation.isSupported();
    }
    int orientation() const
    {
        return m_orientation.value();
    }
    void setOrientation(int ori);

    QString outputName() const
    {
        return m_outputName.value();
    }
    void setOutputName(const QString &outputName);
    QSizeF size() const
    {
        return m_size.value();
    }

    QRectF outputArea() const
    {
        return m_outputArea.value();
    }
    void setOutputArea(const QRectF &outputArea);

    bool isEnabled() const
    {
        return m_enabled.value();
    }

    void setEnabled(bool enabled);

Q_SIGNALS:
    void needsSaveChanged();

    void leftHandedChanged();
    void orientationChanged();
    void outputNameChanged();
    void outputAreaChanged();
    void enabledChanged();

private:
    template<typename T>
    struct Prop {
        typedef T (OrgKdeKWinInputDeviceInterface::*ValueFunction)() const;
        typedef bool (OrgKdeKWinInputDeviceInterface::*SupportedFunction)() const;
        typedef bool (OrgKdeKWinInputDeviceInterface::*SetterFunction)(const T &value);
        typedef void (InputDevice::*ChangedSignal)();

        explicit Prop(InputDevice *device, const char *propName, ValueFunction defaultValueFunction, SupportedFunction supported, ChangedSignal changedSignal)
            : m_defaultValueFunction(defaultValueFunction)
            , m_supportedFunction(supported)
            , m_changedSignalFunction(changedSignal)
            , m_device(device)
        {
            int idx = OrgKdeKWinInputDeviceInterface::staticMetaObject.indexOfProperty(propName);
            if (idx < 0) {
                qDebug() << "there is no" << propName;
            }
            Q_ASSERT(idx >= 0);
            m_prop = OrgKdeKWinInputDeviceInterface::staticMetaObject.property(idx);
        }

        explicit Prop(InputDevice *device, const char *propName)
            : m_defaultValueFunction(nullptr)
            , m_supportedFunction(nullptr)
            , m_changedSignalFunction(nullptr)
            , m_device(device)
        {
            Q_ASSERT(m_device);
            int idx = OrgKdeKWinInputDeviceInterface::staticMetaObject.indexOfProperty(propName);
            Q_ASSERT(idx >= 0);
            m_prop = OrgKdeKWinInputDeviceInterface::staticMetaObject.property(idx);
        }

        T value() const
        {
            if (!m_value.has_value()) {
                auto iface = m_device->m_iface.get();
                if (isSupported()) {
                    m_value = m_prop.read(iface).value<T>();
                }
            }
            return m_value ? m_value.value() : T();
        }
        void resetFromDefaults()
        {
            if (isSupported()) {
                set(defaultValue());
            }
        }
        void resetFromSaved()
        {
            m_value = {};
            value();
            m_configValue = m_value;
            if (m_changedSignalFunction) {
                (m_device->*m_changedSignalFunction)();
            }
        }

        void set(T newVal);
        T defaultValue() const
        {
            return m_defaultValueFunction ? (m_device->m_iface.get()->*m_defaultValueFunction)() : T();
        }
        bool changed() const;
        void set(const Prop<T> &p)
        {
            set(p.value);
        }

        bool isSupported() const
        {
            auto iface = m_device->m_iface.get();
            return !m_supportedFunction || (iface->*m_supportedFunction)();
        }

        bool save();
        bool isDefaults() const
        {
            return m_value == defaultValue();
        }

    private:
        QMetaProperty m_prop;
        const ValueFunction m_defaultValueFunction;
        const SupportedFunction m_supportedFunction;
        const ChangedSignal m_changedSignalFunction;
        InputDevice *const m_device;
        mutable std::optional<T> m_configValue;
        mutable std::optional<T> m_value;
    };

    //
    // general
    Prop<QString> m_name = Prop<QString>(this, "name");
    Prop<QSizeF> m_size = Prop<QSizeF>(this, "size");
    Prop<QString> m_sysName = Prop<QString>(this, "sysName");

    Prop<bool> m_leftHanded = Prop<bool>(this,
                                         "leftHanded",
                                         &OrgKdeKWinInputDeviceInterface::leftHandedEnabledByDefault,
                                         &OrgKdeKWinInputDeviceInterface::supportsLeftHanded,
                                         &InputDevice::leftHandedChanged);
    Prop<int> m_orientation =
        Prop<int>(this, "orientationDBus", nullptr, &OrgKdeKWinInputDeviceInterface::supportsCalibrationMatrix, &InputDevice::orientationChanged);
    Prop<bool> m_enabled = Prop<bool>(this, "enabled", &OrgKdeKWinInputDeviceInterface::enabledByDefault, nullptr, &InputDevice::enabledChanged);

    Prop<QString> m_outputName = Prop<QString>(this, "outputName", nullptr, nullptr, &InputDevice::outputNameChanged);
    Prop<QRectF> m_outputArea = Prop<QRectF>(this,
                                             "outputArea",
                                             &OrgKdeKWinInputDeviceInterface::defaultOutputArea,
                                             &OrgKdeKWinInputDeviceInterface::supportsOutputArea,
                                             &InputDevice::outputAreaChanged);

    std::unique_ptr<OrgKdeKWinInputDeviceInterface> m_iface;
};
