// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATWRAPPER_P_H
#define COMPATWRAPPER_P_H

#include <QObject>
#include <QTimer>

class CuteIPCInterface;
namespace cooperation_core {

class CompatWrapper;
class CompatWrapperPrivate : public QObject
{
    friend class CompatWrapper;
    Q_OBJECT
public:
    explicit CompatWrapperPrivate(CompatWrapper *qq);
    ~CompatWrapperPrivate();

    QString sessionId;
public Q_SLOTS:
    void onTimeConnectBackend();
    void ipcCompatSlot(int type, const QString& msg);

private:
    CompatWrapper *q { nullptr };
    CuteIPCInterface *ipcInterface { nullptr };

    QTimer *ipcTimer { nullptr };
    bool backendOk { false };
};

}

#endif // COMPATWRAPPER_P_H
