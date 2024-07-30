/*
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PARTVIEWER_H
#define PARTVIEWER_H

#include <kparts/mainwindow.h>
#include <kparts/readonlypart.h>

class KPluginMetaData;

class PartViewer : public KParts::MainWindow
{
    Q_OBJECT
public:
    PartViewer();
    ~PartViewer() override;

    void openUrl(const QUrl &url);

    void switchToPart(const QUrl &url);

public Q_SLOTS:
    void slotFileOpen();

private:
    void loadPlugin(const KPluginMetaData &md, const QUrl &url);

    KParts::ReadOnlyPart *m_part;
    QList<QAction *> m_openWithActions;
};

#endif
