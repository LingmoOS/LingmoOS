// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOPORT_DAEMON_H
#define DAUDIOPORT_DAEMON_H

#include "daudioport_p.h"

#include <QObject>
#include <DDBusInterface>

DAUDIOMANAGER_BEGIN_NAMESPACE
class DDaemonAudioPort : public DPlatformAudioPort
{
    Q_OBJECT

public:
    explicit DDaemonAudioPort(DPlatformAudioCard *card);
    virtual ~DDaemonAudioPort() override;

    void setEnabled(const bool enabled) override;
    bool isEnabled() const override;

    virtual void setActive(const int active) override;
};
DAUDIOMANAGER_END_NAMESPACE

#endif
