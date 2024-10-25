/*
    SPDX-FileCopyrightText: 2019 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class KateSessionManagerTest : public QObject
{
    Q_OBJECT

public:
    ~KateSessionManagerTest() override;
    KateSessionManagerTest();

private Q_SLOTS:

    void basic();
    void activateNewNamedSession();
    void anonymousSessionFile();
    void urlizeSessionFile();
    void renameSession();

private:
    class QTemporaryDir *m_tempdir;
    class KateSessionManager *m_manager;
    class KateApp *m_app; // dependency, sigh...
};
