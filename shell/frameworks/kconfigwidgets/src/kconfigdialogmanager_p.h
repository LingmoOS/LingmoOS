/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Benjamin C Meyer <ben+kdelibs at meyerhome dot net>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGDIALOGMANAGER_P_H
#define KCONFIGDIALOGMANAGER_P_H

#include <QHash>
#include <QSet>
#include <QString>

class QWidget;
class KConfigDialogManager;
class KCoreConfigSkeleton;

class KConfigDialogManagerPrivate
{
public:
    KConfigDialogManagerPrivate(KConfigDialogManager *qq);

    void setDefaultsIndicatorsVisible(bool enabled);

    void onWidgetModified();
    void updateWidgetIndicator(const QString &configId, QWidget *widget);
    void updateAllWidgetIndicators();

public:
    KConfigDialogManager *const q;

    /**
     * KConfigSkeleton object used to store settings
     */
    KCoreConfigSkeleton *m_conf = nullptr;

    /**
     * Dialog being managed
     */
    QWidget *m_dialog = nullptr;

    QHash<QString, QWidget *> knownWidget;
    QHash<QString, QWidget *> buddyWidget;
    QSet<QWidget *> allExclusiveGroupBoxes;
    bool insideGroupBox : 1;
    bool trackChanges : 1;
    bool defaultsIndicatorsVisible : 1;
};

#endif // KCONFIGDIALOGMANAGER_P_H
