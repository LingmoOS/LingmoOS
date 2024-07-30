/*

    This file is part of the KDE project, module kfile.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <chris@kde.org>
    SPDX-FileCopyrightText: 2002 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KICONDIALOG_P_H
#define KICONDIALOG_P_H

#include <QFileDialog>
#include <QPointer>
#include <QSortFilterProxyModel>

#include <kiconloader.h>

#include "kicondialogmodel_p.h"

#include "ui_kicondialog.h"

class QLabel;
class QPushButton;

class KIconDialog;
class KIconDialogSortFilterProxyModel;

class KIconDialogPrivate
{
public:
    KIconDialogPrivate(KIconDialog *qq);

    void init();
    void showIcons();
    bool selectIcon(const QString &iconName);
    void setContext(KIconLoader::Context context);
    void updatePlaceholderLabel();

    void browse();

    bool isSystemIconsContext() const;

    KIconDialog *q;

    KIconLoader *mpLoader;

    KIconDialogModel *model;
    KIconDialogSortFilterProxyModel *proxyModel;

    int mGroupOrSize;
    KIconLoader::Context mContext;

    QLabel *placeholderLabel;
    QPushButton *browseButton;

    QAction *filterSymbolicAction;
    QActionGroup *filterSymbolicGroup;

    bool m_bStrictIconSize = true;
    bool m_bLockUser = false;
    bool m_bLockCustomDir = false;
    QString custom;
    QString customLocation;
    QString pendingSelectedIcon;
    QPointer<QFileDialog> browseDialog;

    Ui::IconDialog ui;
};

#endif // KICONDIALOG_P_H
