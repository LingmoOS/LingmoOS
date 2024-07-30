/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2005-2007 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KNOTIFYCONFIGACTIONSWIDGET_H
#define KNOTIFYCONFIGACTIONSWIDGET_H

#include "knotify-config.h"
#include "ui_knotifyconfigactionswidgetbase.h"
#include <QWidget>

#if HAVE_CANBERRA
struct ca_context;
#endif

class KNotifyConfigElement;

/**
 * Represent the config for an event
 *  @internal
 *  @author Olivier Goffart <ogoffart @ kde.org>
 */
class KNotifyConfigActionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KNotifyConfigActionsWidget(QWidget *parent);
    ~KNotifyConfigActionsWidget() override;

    void setConfigElement(KNotifyConfigElement *config);
    void save(KNotifyConfigElement *config);
Q_SIGNALS:
    void changed();
private Q_SLOTS:
    void slotPlay();

private:
    Ui::KNotifyConfigActionsWidgetBase m_ui;
#if HAVE_CANBERRA
    ca_context *m_context = nullptr;
#endif
};

#endif // KNOTIFYCONFIGACTIONSWIDGET_H
