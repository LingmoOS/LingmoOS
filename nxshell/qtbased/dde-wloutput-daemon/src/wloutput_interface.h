// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WLOUTPUT_INTERFACE_H
#define WLOUTPUT_INTERFACE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusConnection>
#include <QThread>
#include <QSize>
#include <QList>
#include <QTimer>
#include <QByteArray>

#include <DWayland/Client/outputdevice_v2.h>
#include <DWayland/Client/registry.h>
#include <DWayland/Client/fakeinput.h>
#include <DWayland/Client/connection_thread.h>
#include <DWayland/Client/outputmanagement_v2.h>
#include <DWayland/Client/outputconfiguration_v2.h>
#include <DWayland/Client/dpms.h>
#include <DWayland/Client/output.h>
#include <DWayland/Client/ddeseat.h>
#include <DWayland/Client/pointer.h>

#include "wlidle_interface.h"
#include "wldpms_interface.h"
#include "wldpms_manager_interface.h"

#include <linux/input.h>


namespace KWayland {
namespace  Client{
    class PlasmaWindowModel;
    class PrimaryOutputV1;
}
}

using namespace KWayland::Client;

const QString SERVER = "org.deepin.dde.KWayland1";
const QString PATH = "/org/deepin/dde/KWayland1/Output";
const QString INTERFACE = "org.deepin.dde.KWayland1.Output";

typedef struct _mode_info
{
    int id;
    int width;
    int height;
    int refresh_rate;
    int flags; // available values: [current, preferred]
}ModeInfo;


typedef  struct _output_info
{
    QString model;
    QString manufacturer;
    QString uuid;
    QString name;
    QByteArray edid;
    int enabled;
    int x;
    int y;
    int width;
    int height;
    int refresh_rate;
    int transform;
    int phys_width;
    int phys_height;
    double scale;
    QList<ModeInfo> lstModeInfos;
}OutputInfo;


class wloutput_interface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.KWayland1.Output")

public:
    explicit wloutput_interface(QObject *parent=nullptr);
    virtual ~wloutput_interface();
    bool InitDBus();
    void StartWork();
    QDBusAbstractAdaptor* idleObject() { return m_wlIdleInterface; }
    static OutputInfo GetOutputInfo(const OutputDeviceV2* dev);
    static QString OutputInfo2Json(QList<OutputInfo>& listOutputInfos);
    static QList<OutputInfo> json2OutputInfo(QString jsonString);

signals:
    void OutputAdded(QString output);
    void OutputRemoved(QString output);
    void OutputChanged(QString output);
    void PrimaryOutputChanged(const QString &outputName);
    void ButtonPress(quint32 button, quint32 x, quint32 y);
    void ButtonRelease(quint32 button, quint32 x, quint32 y);
    void CursorMove(quint32 x, quint32 y);
    void AxisChanged(int orientation, qreal delta);

public Q_SLOTS:
    QString ListOutput();
    QString GetOutput(QString uuid);
    void setPrimary(const QString &outputName);
    void Apply(QString outputs);
    void WlSimulateKey(int keycode);
    void setBrightness(QString uuid, const int brightness);

private:
    void onDeviceChanged(OutputDeviceV2 *dev);
    void onDeviceRemove(quint32 name, quint32 version) ;
    void onMangementAnnounced(quint32 name, quint32 version);
    void createPlasmaWindowManagement(quint32 name, quint32 version);
    void createDpmsManagement();
    void onPrimaryOutputV1Announced(quint32 name, quint32 version);
    void registerDpmsDbus(Output *output);

private:
    //QTimer m_Timer;
    ConnectionThread *m_pConnectThread{nullptr};
    QThread *m_pThread{nullptr};
    Registry *m_pRegisry{nullptr};
    OutputManagementV2 *m_pManager{nullptr};
    PrimaryOutputV1 *m_primaryOutput{nullptr};
    OutputConfigurationV2 *m_pConf{nullptr};
    EventQueue *m_eventQueue{nullptr};
    bool m_bConnected;
    PlasmaWindowManagement *m_pWindowManager{nullptr};
    WlIdleInterface *m_wlIdleInterface{nullptr};
    Idle *m_idle{nullptr};
    Seat *m_seat{nullptr};
    DDESeat *m_ddeSeat = nullptr;
    DDEPointer *m_ddePointer = nullptr;
    DDETouch *m_ddeTouch = nullptr;
    QMap<int32_t, QPointF> m_touchMap;  // wayland 窗管 touchUp 时没有位置信息，自行存储
    FakeInput *m_fakeInput{nullptr};
    QTimer *m_timer;
    WlDpmsManagerInterface *m_wldpms_Manager{nullptr};
    DpmsManager *m_dpmsManger{nullptr};
    QMap<OutputDeviceV2 *, QSize> devSizeMap;
};

#endif // WLOUTPUT_INTERFACE_H
