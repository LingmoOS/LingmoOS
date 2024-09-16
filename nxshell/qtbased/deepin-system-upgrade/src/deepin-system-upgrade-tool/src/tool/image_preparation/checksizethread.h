// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QThread>
#include <QString>

class CheckSizeThread : public QThread
{
    Q_OBJECT

signals:
    void done(long long size);
    void error(QString errMsg);

public:
    CheckSizeThread(const QString &urlString);
    void run();

private:
    const QString m_url;
};
