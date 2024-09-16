// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDE_DISPLAY_DISPLAY_H
#define DDE_DISPLAY_DISPLAY_H

#include "../dbus/screenrect.h"
#include "../dbus/resolution.h"
#include "../dbus/resolutionlist.h"
#include "../dbus/brightnessmap.h"
#include "../dbus/touchscreeninfolist.h"
#include "../dbus/touchscreeninfolist_v2.h"
#include "../dbus/touchscreenmap.h"

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusContext>

namespace dde {
namespace display {
class DisplayManager;
}
} // namespace dde

class Display1 : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(uchar DisplayMode READ displayMode NOTIFY displayModeChanged)
    Q_PROPERTY(QString Primary READ primary NOTIFY primaryChanged)
    Q_PROPERTY(ScreenRect PrimaryRect READ primaryRect NOTIFY primaryRectChanged)
    Q_PROPERTY(quint16 ScreenHeight READ screenHeight NOTIFY screenHeightChanged)
    Q_PROPERTY(quint16 ScreenWidth READ screenWidth NOTIFY screenWidthChanged)
    Q_PROPERTY(BrightnessMap Brightness READ brightness)
    Q_PROPERTY(bool HasChanged READ hasChanged)
    Q_PROPERTY(quint32 MaxBacklightBrightness READ maxBacklightBrightness)
    Q_PROPERTY(QList<QDBusObjectPath> Monitors READ monitors)
    Q_PROPERTY(QString CurrentCustomId READ currentCustomId)
    Q_PROPERTY(QStringList CustomIdList READ customIdList)
    Q_PROPERTY(TouchscreenInfoList Touchscreens READ touchscreens)
    Q_PROPERTY(TouchscreenInfoList_V2 TouchscreensV2 READ touchscreensV2)
    Q_PROPERTY(TouchscreenMap TouchMap READ touchMap)
    Q_PROPERTY(quint32 ColorTemperatureMode READ colorTemperatureMode)
    Q_PROPERTY(quint32 ColorTemperatureManual READ colorTemperatureManual)

public :
    inline uchar displayMode() const { return 1; }
    inline bool hasChanged() const { return false; }
    inline quint32 maxBacklightBrightness() const { return 0; }
    inline QString currentCustomId() const { return QString{}; }
    inline QStringList customIdList() const { return QStringList{}; }
    inline TouchscreenInfoList touchscreens() const { return TouchscreenInfoList{}; }
    inline TouchscreenInfoList_V2 touchscreensV2() const { return TouchscreenInfoList_V2{}; }
    inline TouchscreenMap touchMap() const { return TouchscreenMap{}; }
    inline quint32 colorTemperatureMode() const { return 0; }
    inline quint32 colorTemperatureManual() const { return 0; }

    BrightnessMap brightness() const;
    QString primary() const;
    quint16 screenHeight() const;
    quint16 screenWidth() const;
    ScreenRect primaryRect() const;
    QList<QDBusObjectPath> monitors() const;

    void init();

public Q_SLOTS:
    void ApplyChanges();
    bool CanRotate();
    bool GetRealDisplayMode();
    QStringList ListOutputNames();
    ResolutionList ListOutputsCommonModes();
    void Reset();
    void ResetChanges();
    void Save();
    void SetPrimary(const QString &name);
    void SwitchMode(const uchar &mode, const QString &name);
    void AssociateTouch(const QString &in0, const QString &in1);
    void AssociateTouchByUUID(const QString &in0, const QString &in1);
    void ChangeBrightness(bool in0);
    void DeleteCustomMode(const QString &in0);
    void ModifyConfigName(const QString &in0, const QString &in1);
    void RefreshBrightness();
    void SetAndSaveBrightness(const QString &in0, double in1);
    void SetBrightness(const QString &in0, double in1);
    void SetColorTemperature(int in0);
    void SetMethodAdjustCCT(int in0);

public:
    Display1(QObject *parent = nullptr);
    ~Display1() override=default;

    void initConnections();

Q_SIGNALS:
    // signals on dbus
    void monitorsChanged(QList<QDBusObjectPath>);
    void displayModeChanged(uchar);
    void primaryChanged(QString);
    void primaryRectChanged(ScreenRect);
    void screenHeightChanged(quint16);
    void screenWidthChanged(quint16);

private:
    uchar m_displayMode;
    QString m_primary;
    ScreenRect m_primaryRect;
    quint16 m_screenHeight;
    quint16 m_screenWidth;

    dde::display::DisplayManager *m_manager;
};

#endif // DDE_DISPLAY_DISPLAY_H
