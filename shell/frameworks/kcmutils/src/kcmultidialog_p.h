/*
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMULTIDIALOG_P_H
#define KCMULTIDIALOG_P_H

#include "kcmodule.h"
#include "kcmultidialog.h"
#include <QList>
#include <QStringList>

class KCModuleProxy;
class KPageWidgetItem;

class KCMultiDialogPrivate
{
public:
    KCMultiDialogPrivate(KCMultiDialog *parent)
        : currentModule(nullptr)
        , q(parent)
    {
    }

    KCModule *currentModule;

    struct CreatedModule {
        KCModule *kcm;
        KPageWidgetItem *item;
        QStringList componentNames;
        bool firstShow = true;
    };

    typedef QList<CreatedModule> ModuleList;
    ModuleList modules;

    void slotCurrentPageChanged(KPageWidgetItem *current, KPageWidgetItem *previous);
    void clientChanged();
    void dialogClosed();
    void updateHeader(bool use, const QString &message);

    KCMultiDialog *q;

    void init();
    void apply();
    bool resolveChanges(KCModule *currentProxy);
    bool moduleSave(KCModule *module);
};

#endif // KCMULTIDIALOG_P_H
