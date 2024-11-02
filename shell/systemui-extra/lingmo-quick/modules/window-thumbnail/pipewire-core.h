/*
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileContributor: Jan Grulich <jgrulich@redhat.com>
    SPDX-FileContributor: iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PIPEWIRECORE_H
#define PIPEWIRECORE_H


#include <QObject>
#include <pipewire/pipewire.h>

class PipeWireCore : public QObject
{
Q_OBJECT
public:
    PipeWireCore();

    static void onCoreError(void *data, uint32_t id, int seq, int res, const char *message);

    ~PipeWireCore();

    bool init();

    static QSharedPointer<PipeWireCore> self();

    struct pw_core *pwCore = nullptr;
    struct pw_context *pwContext = nullptr;
    struct pw_loop *pwMainLoop = nullptr;
    spa_hook coreListener;
    QString m_error;

    pw_core_events pwCoreEvents = {};

Q_SIGNALS:
    void pipewireFailed(const QString &message);
};

#endif // PIPEWIRECORE_H

