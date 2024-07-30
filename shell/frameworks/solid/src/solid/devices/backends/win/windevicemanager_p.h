/*
    SPDX-FileCopyrightText: 2014 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WINDEVICEMANAGER_P_H
#define WINDEVICEMANAGER_P_H

namespace Solid
{
namespace Backends
{
namespace Win
{
class SolidWinEventFilter : public QObject
{
    Q_OBJECT
public:
    SolidWinEventFilter();
    ~SolidWinEventFilter();
    void promoteAddedDevice(const QSet<QString> &udi);
    void promoteRemovedDevice(const QSet<QString> &udi);
    void promotePowerChange();

    static SolidWinEventFilter *instance();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

Q_SIGNALS:
    void powerChanged();
    void deviceAdded(const QSet<QString> &udi);
    void deviceRemoved(const QSet<QString> &udi);

private:
    HWND m_windowID;
};
}
}
}

#endif // WINDEVICEMANAGER_P_H
