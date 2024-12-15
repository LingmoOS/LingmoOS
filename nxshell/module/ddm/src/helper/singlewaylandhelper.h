// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QObject>

class QProcess;

class SingleWaylandHelper : public QObject {
    Q_OBJECT
public:
    explicit SingleWaylandHelper(QObject *parent = nullptr);

    bool start(const QString& compositor, const QString& args);

private:
    QProcess *m_process;
};
