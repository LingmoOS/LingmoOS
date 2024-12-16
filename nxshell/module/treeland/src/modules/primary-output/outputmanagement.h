// Copyright (C) 2023 rewine <luhongxu@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <woutput.h>
#include <wserver.h>

#include <qwsignalconnector.h>

#include <QList>
#include <QQmlEngine>

struct treeland_output_manager_v1;
WAYLIB_SERVER_USE_NAMESPACE

class PrimaryOutputV1 : public QObject, public Waylib::Server::WServerInterface
{
    Q_OBJECT
    Q_PROPERTY(const char *primaryOutput READ primaryOutput WRITE setPrimaryOutput NOTIFY primaryOutputChanged)

public:
    explicit PrimaryOutputV1(QObject *parent = nullptr);

    const char *primaryOutput();

    void newOutput(WAYLIB_SERVER_NAMESPACE::WOutput *output);
    void removeOutput(WAYLIB_SERVER_NAMESPACE::WOutput *output);

protected:
    void create(WServer *server) override;
    void destroy(WServer *server) override;
    wl_global *global() const override;

Q_SIGNALS:
    void requestSetPrimaryOutput(const char *);
    void primaryOutputChanged();

public Q_SLOTS:
    bool setPrimaryOutput(const char *name);

private:
    treeland_output_manager_v1 *m_handle{ nullptr };
    QList<WAYLIB_SERVER_NAMESPACE::WOutput *> m_outputs;
};
