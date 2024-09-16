// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPMSCLIENT_H
#define DPMSCLIENT_H

#include <kscreen/config.h>
#include <QObject>
#include <QRect>

#include <KWayland/Client/dpms.h>
#include <KWayland/Client/registry.h>

class QThread;

namespace KWayland
{
namespace Client
{
class ConnectionThread;
}
}

namespace KScreen
{
class DpmsClient : public QObject
{
    Q_OBJECT

public:
    explicit DpmsClient(QObject *parent = nullptr);
    ~DpmsClient() override;

    void setExcludedOutputNames(const QStringList &excluded)
    {
        m_excludedOutputNames = excluded;
    }

    void connect();
    void off();
    void on();

Q_SIGNALS:
    void ready();
    void finished();

private Q_SLOTS:
    void connected();
    void modeChanged();

private:
    void changeMode(KWayland::Client::Dpms::Mode mode);
    KWayland::Client::ConnectionThread *m_connection = nullptr;
    KWayland::Client::DpmsManager *m_dpmsManager = nullptr;
    KWayland::Client::Registry *m_registry = nullptr;
    bool m_setOff = true;
    bool m_setOn = false;

    bool m_supportedOututCount = 0;
    int m_modeChanges = 0;
    QStringList m_excludedOutputNames;
};

} // namespace

#endif // DPMSCLIENT_H
