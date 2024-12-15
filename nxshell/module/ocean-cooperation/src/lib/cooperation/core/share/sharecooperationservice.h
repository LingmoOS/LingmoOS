// SPDX-FileCopyrightText: 2023-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHARECOOPERATIONSERVICE_H
#define SHARECOOPERATIONSERVICE_H

#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QTextStream>
#include "discover/deviceinfo.h"

typedef enum barrier_type_t {
    Server = 555,
    Client = 666
} BarrierType;

struct ShareServerConfig
{
    QString server_screen { "" };
    QString client_screen { "" };
    QString screen_left { "" };
    QString screen_right { "" };
    bool left_halfDuplexCapsLock { false };
    bool left_halfDuplexNumLock { false };
    bool left_halfDuplexScrollLock { false };
    bool left_xtestIsXineramaUnaware { false };
    bool left_preserveFocus { false };
    QString left_switchCorners { "none" };
    int left_switchCornerSize { 0 };
    bool right_halfDuplexCapsLock { false };
    bool right_halfDuplexNumLock { false };
    bool right_halfDuplexScrollLock { false };
    bool right_xtestIsXineramaUnaware { false };
    bool right_preserveFocus { false };
    QString right_switchCorners { "none" };
    int right_switchCornerSize { 0 };
    bool relativeMouseMoves { false };
    bool screenSaverSync { true };
    bool win32KeepForeground { false };
    bool clipboardSharing { false };
    QString switchCorners { "none" };
    int switchCornerSize { 0 };
};

class CooConfig;
class ShareCooperationService : public QObject
{
    Q_OBJECT
    friend class ShareCooperationServiceManager;

public:
    ~ShareCooperationService() override;

    void setBarrierType(BarrierType type);
    BarrierType barrierType() const;

    void setServerConfig(const DeviceInfoPointer selfDevice, const DeviceInfoPointer targetDevice);
    bool setServerConfig(const ShareServerConfig &config);

    void setClientTargetIp(const QString &ip);

    void setEnableCrypto(bool enable);
    void setBarrierProfile(const QString &dir);

    bool isRunning();
    void terminateAllBarriers();

public slots:
    bool restartBarrier();
    bool startBarrier();
    void stopBarrier();

protected slots:
    void barrierFinished(int exitCode, QProcess::ExitStatus);
    void appendLogRaw(const QString &text, bool error);
    void logOutput();
    void logError();

private:
    explicit ShareCooperationService(QObject *parent = nullptr);

protected:
    CooConfig &cooConfig() { return *_cooConfig; }
    QProcess *barrierProcess() { return _pBarrier; }
    void setBarrierProcess(QProcess *p) { _pBarrier = p; }

    QString configFilename();
    QString address();
    QString appPath(const QString &name);

    bool clientArgs(QStringList &args, QString &app);
    bool serverArgs(QStringList &args, QString &app);
    bool checkParam(const ShareServerConfig &config);
    void setScreen(const ShareServerConfig &config, QTextStream *stream);
    void setScreenLink(const ShareServerConfig &config, QTextStream *stream);
    void setScreenOptions(const ShareServerConfig &config, QTextStream *stream);
private:
    CooConfig *_cooConfig { nullptr };
    QProcess *_pBarrier { nullptr };
    BarrierType _brrierType;
    QString _barrierConfig;

    bool _expectedRunning = false;
};

#endif   // SHARECOOPERATIONSERVICE_H
