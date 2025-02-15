/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KRECENTFILESACTIONTEST_H
#define KRECENTFILESACTIONTEST_H

#include <QObject>
class QMenu;
class KRecentFilesActionTest : public QObject
{
    Q_OBJECT
public:
    explicit KRecentFilesActionTest(QObject *parent = nullptr);
    ~KRecentFilesActionTest() override;

private:
    static QStringList extractActionNames(QMenu *menu);
    static QList<bool> extractActionEnableVisibleState(QMenu *menu);

private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAddActionInTop();
    void shouldClearMenu();
    void testUrlsOrder();
    void addUrlAlreadyInList();
    void removeExecessItems();
};

#endif // KRECENTFILESACTIONTEST_H
