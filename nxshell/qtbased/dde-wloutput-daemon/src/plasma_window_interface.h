// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLASMA_WINDOW_INTERFACE_H
#define PLASMA_WINDOW_INTERFACE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusConnection>

#include <DWayland/Client/plasmawindowmanagement.h>

using namespace KWayland::Client;

const QString WINDOW_PATH = "/org/deepin/dde/KWayland1/PlasmaWindow";

class PlasmaWindowInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.KWayland1.PlasmaWindow")

public:
    explicit PlasmaWindowInterface(PlasmaWindow* plasma_window);
    virtual ~PlasmaWindowInterface();
    bool InitDBus();
    void InitConnect();

signals:
    void ActiveChanged();
    void AppIdChanged();
    void CloseableChanged();
    void DemandsAttentionChanged();
    void FullscreenableChanged();
    void FullscreenChanged();
    void GeometryChanged();
    void IconChanged();
    void KeepAboveChanged();
    void KeepBelowChanged();
    void MaximizeableChanged();
    void MaximizedChanged();
    void MinimizeableChanged();
    void MinimizedChanged();
    void MovableChanged();
    void OnAllDesktopsChanged();
    void ParentWindowChanged();
    void PlasmaVirtualDesktopEntered(const QString &id);
    void PlasmaVirtualDesktopLeft(const QString &id);
    void ResizableChanged();
    void ShadeableChanged();
    void ShadedChanged();
    void SkipSwitcherChanged();
    void SkipTaskbarChanged();
    void TitleChanged();
    void Unmapped();
    void VirtualDesktopChangeableChanged();
    void VirtualDesktopChanged();

public Q_SLOTS:
    QString AppId() const;
    void Destroy();
    QRect Geometry() const;
    QString Icon () const;
    quint32 InternalId () const;
    bool IsActive () const;
    bool IsCloseable () const;
    bool IsDemandingAttention () const;
    bool IsFullscreen () const;
    bool IsFullscreenable () const;
    bool IsKeepAbove () const;
    bool IsKeepBelow () const;
    bool IsMaximizeable () const;
    bool IsMaximized () const;
    bool IsMinimizeable () const;
    bool IsMinimized () const;
    bool IsMovable () const;
    bool IsOnAllDesktops () const;
    bool IsResizable () const;
    bool IsShadeable () const;
    bool IsShaded () const;
    bool IsValid () const;
    bool IsVirtualDesktopChangeable () const;
    quint32 Pid () const;
    QStringList PlasmaVirtualDesktops () const;
    void Release ();
    void RequestActivate ();
    void RequestClose ();
    void RequestEnterNewVirtualDesktop ();
    void RequestEnterVirtualDesktop (const QString &id);
    void RequestLeaveVirtualDesktop (const QString &id);
    void RequestMove ();
    void RequestResize ();
    void RequestToggleKeepAbove ();
    void RequestToggleKeepBelow ();
    void RequestToggleMaximized ();
    void RequestToggleMinimized ();
    void RequestToggleShaded ();
    void RequestVirtualDesktop (quint32 desktop);
    void SetMinimizedGeometry (Surface *panel, const QRect &geom);
    bool SkipSwitcher () const;
    bool SkipTaskbar () const;
    QString Title () const;
    void UnsetMinimizedGeometry (Surface *panel);
    quint32 VirtualDesktop () const;
    quint32 WindowId() const;
    QByteArray uuid() const;

private:
    PlasmaWindow* m_plasmaWindow{nullptr};
};

#endif // PLASMA_WINDOW_INTERFACE_H
