// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDE_DISPLAY_MONITOR_H
#define DDE_DISPLAY_MONITOR_H

#include "../dbus/resolution.h"
#include "../dbus/resolutionlist.h"
#include "../dbus/reflectlist.h"
#include "../dbus/rotationlist.h"

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusContext>
#include <sys/types.h>

#include <kscreen/output.h>

class Monitor : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(QStringList AvailableFillModes READ availableFillModes)
    Q_PROPERTY(Resolution BestMode READ bestMode)
    Q_PROPERTY(bool Connected READ connected)
    Q_PROPERTY(QString CurrentFillMode READ currentFillMode)
    Q_PROPERTY(Resolution CurrentMode READ currentMode)
    Q_PROPERTY(uchar CurrentRotateMode READ currentRotateMode)
    Q_PROPERTY(bool Enabled READ enabled)
    Q_PROPERTY(ushort Height READ height)
    Q_PROPERTY(QString Manufacturer READ manufacturer)
    Q_PROPERTY(quint32 MmHeight READ mmHeight)
    Q_PROPERTY(quint32 MmWidth READ mmWidth)
    Q_PROPERTY(QString Model READ model)
    Q_PROPERTY(ResolutionList Modes READ modes)
    Q_PROPERTY(QString Name READ name)
    Q_PROPERTY(ushort Reflect READ reflect)
    Q_PROPERTY(ReflectList Reflects READ reflects)
    Q_PROPERTY(double RefreshRate READ refreshRate)
    Q_PROPERTY(ushort Rotation READ rotation)
    Q_PROPERTY(RotationList Rotations READ rotations)
    Q_PROPERTY(ushort Width READ width)
    Q_PROPERTY(short X READ x)
    Q_PROPERTY(short Y READ y)
    Q_PROPERTY(quint32 ID READ id)
    Q_PROPERTY(double Brightness READ brightness)

public :
    inline QStringList availableFillModes() const { return QStringList{}; }
    inline Resolution bestMode() const { return Resolution{}; }
    inline bool connected() const { return true; }
    inline QString currentFillMode() const { return QString{}; }
    inline uchar currentRotateMode() const { return 0; }
    inline bool enabled() const { return true; }
    inline QString manufacturer() const { return QString{}; }
    inline QString model() const { return QString{}; }
    inline ResolutionList modes() const { return ResolutionList{}; }
    inline ushort reflect() const { return 0; }
    inline ReflectList reflects() const { return ReflectList{}; }
    inline ushort rotation() const { return 0; }
    inline RotationList rotations() const { return RotationList{}; }
    inline double brightness() const { return 1.0; }

    QString name() const;
    quint32 id() const;
    short x() const;
    short y() const;
    ushort width() const;
    ushort height() const;
    quint32 mmHeight() const;
    quint32 mmWidth() const;
    double refreshRate() const;
    Resolution currentMode() const;

    void init();

public Q_SLOTS:
    void Enable(bool in0);
    void SetMode(uint in0);
    void SetModeBySize(ushort in0, ushort in1);
    void SetPosition(short in0, short in1);
    void SetReflect(ushort in0);
    void SetRotation(ushort in0);

public:
    Monitor(const KScreen::OutputPtr &output, QObject *parent = nullptr);
    ~Monitor() override=default;

private:
    KScreen::OutputPtr m_monitor;
};

#endif // DDE_DISPLAY_MONITOR_H
