/*
 *   SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KCMUSB_H
#define _KCMUSB_H

#include <QMap>

#include <KCModule>

class QTreeWidget;
class QTreeWidgetItem;
class QTextEdit;

class USBViewer : public KCModule
{
    Q_OBJECT

public:
    explicit USBViewer(QObject *parent, const KPluginMetaData &data);
    ~USBViewer() override;

    void load() override;

protected Q_SLOTS:

    void selectionChanged(QTreeWidgetItem *item);
    void refresh();

private:
    QMap<int, QTreeWidgetItem *> _items;
    QTreeWidget *_devices;
    QTextEdit *_details;
};

#endif
