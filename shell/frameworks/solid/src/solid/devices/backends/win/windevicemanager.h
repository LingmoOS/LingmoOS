/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WINDEVICEMANAGER_H
#define WINDEVICEMANAGER_H

#include <solid/devices/ifaces/devicemanager.h>

#include <QDebug>
#include <QSet>

#include <qt_windows.h>
#include <winioctl.h>

inline QString qGetLastError(ulong errorNummber = GetLastError())
{
    LPVOID error = NULL;
    size_t len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL,
                               errorNummber,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (LPWSTR)&error,
                               0,
                               NULL);
    QString out = QString::fromWCharArray((wchar_t *)error, (int)len).trimmed().append(" %1").arg(errorNummber);
    LocalFree(error);
    return out;
}

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinDeviceManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT
public:
    WinDeviceManager(QObject *parent = 0);
    ~WinDeviceManager();

    virtual QString udiPrefix() const;

    virtual QSet<Solid::DeviceInterface::Type> supportedInterfaces() const;

    virtual QStringList allDevices();

    virtual QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type = Solid::DeviceInterface::Unknown);

    virtual QObject *createDevice(const QString &udi);

    static const WinDeviceManager *instance();

    template<class INFO>
    static INFO getDeviceInfo(const QString &devName, int code)
    {
        return getDeviceInfo<INFO, void *>(devName, code);
    }

    template<class INFO, class QUERY>
    static INFO getDeviceInfo(const QString &devName, int code, QUERY *query = NULL)
    {
        INFO info;
        ZeroMemory(&info, sizeof(INFO));
        getDeviceInfoPrivate(devName, code, &info, sizeof(INFO), query);
        return info;
    }

    template<class BUFFER_TYPE, class QUERY>
    static void getDeviceInfo(const QString &devName, int code, BUFFER_TYPE *out, DWORD outSize, QUERY *query = NULL)
    {
        ZeroMemory(out, sizeof(BUFFER_TYPE) * outSize);
        getDeviceInfoPrivate(devName, code, out, outSize, query);
    }

    static void deviceAction(const QString &devName, int code)
    {
        getDeviceInfoPrivate<void, void *>(devName, code, NULL, 0, NULL);
    }

Q_SIGNALS:
    void powerChanged();

private Q_SLOTS:
    void updateDeviceList();
    void slotDeviceAdded(const QSet<QString> &udi);
    void slotDeviceRemoved(const QSet<QString> &udi);

private:
    friend class SolidWinEventFilter;

    QSet<QString> m_devices;
    QStringList m_devicesList;
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;

    template<class INFO, class QUERY>
    static void getDeviceInfoPrivate(const QString &devName, int code, INFO *info, DWORD size, QUERY *query = NULL)
    {
        Q_ASSERT(!devName.isNull());
        wchar_t deviceNameBuffer[MAX_PATH];
        QString dev = devName;
        if (!dev.startsWith("\\")) {
            dev = QLatin1String("\\\\?\\") + dev;
        }
        deviceNameBuffer[dev.toWCharArray(deviceNameBuffer)] = 0;
        DWORD bytesReturned = 0;

        ulong err = NO_ERROR;
        HANDLE handle = ::CreateFileW(deviceNameBuffer, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (handle == INVALID_HANDLE_VALUE) {
            err = GetLastError();
            if (err == ERROR_ACCESS_DENIED) {
                // we would need admin rights for GENERIC_READ on systenm drives and volumes
                handle = ::CreateFileW(deviceNameBuffer, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
                err = GetLastError();
            }
            if (handle == INVALID_HANDLE_VALUE) {
                qWarning() << "Invalid Handle" << dev << "reason:" << qGetLastError(err) << "this should not happen.";
                return;
            }
        }
        if (::DeviceIoControl(handle, code, query, sizeof(QUERY), info, size, &bytesReturned, NULL)) {
            ::CloseHandle(handle);
            return;
        }

        if (handle == INVALID_HANDLE_VALUE) {
            qWarning() << "Invalid Handle" << devName << "reason:" << qGetLastError() << "is probaply a subst path or more seriously there is bug!";
            return;
        }

        err = GetLastError();
        switch (err) {
        case ERROR_NOT_READY:
            // the drive is a cd drive with no disk
            break;
        case ERROR_INVALID_FUNCTION:
            // in most cases this means that the device doesn't support this method, like temperature for some batteries
            break;
        default:
            qWarning() << "Failed to query" << dev << "reason:" << qGetLastError(err);
            // DebugBreak();
        }
        ::CloseHandle(handle);
    }
};

}
}
}
#endif // WINDEVICEMANAGER_H
