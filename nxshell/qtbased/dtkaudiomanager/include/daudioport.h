// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOPORT_H
#define DAUDIOPORT_H

#include <dtkaudiomanager_global.h>

#include <QObject>
#include <QPointer>

DAUDIOMANAGER_BEGIN_NAMESPACE
class DPlatformAudioPort;
class LIBDTKAUDIOMANAGERSHARED_EXPORT DAudioPort : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int direction READ direction NOTIFY directionChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(bool isEnabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)

public:
    explicit DAudioPort(DPlatformAudioPort *d);
    virtual ~DAudioPort() override;

    int direction() const;
    QString name() const;
    QString description() const;
    bool isEnabled() const;
    void setEnabled(const bool enabled);
    bool available() const;

Q_SIGNALS:

    void directionChanged(int direction);
    void nameChanged(QString name);
    void descriptionChanged(QString description);
    void enabledChanged(bool isEnabled);
    void availableChanged(bool available);

public:
    Q_DISABLE_COPY(DAudioPort)
    friend class DPlatformAudioPort;
    QExplicitlySharedDataPointer<DPlatformAudioPort> d;
};
DAUDIOMANAGER_END_NAMESPACE

#endif
