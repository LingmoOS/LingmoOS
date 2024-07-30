/*
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <Lingmo5Support/DataEngine>

class PackagekitEngine : public Lingmo5Support::DataEngine
{
    Q_OBJECT

public:
    PackagekitEngine(QObject *parent);
    void init();

protected:
    Lingmo5Support::Service *serviceForSource(const QString &source) override;

private:
    bool m_pk_available;
};
