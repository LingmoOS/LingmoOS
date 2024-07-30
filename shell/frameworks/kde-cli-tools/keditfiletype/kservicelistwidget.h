/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
   SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2002 Daniel Molkentin <molkentin@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#ifndef _KSERVICELISTWIDGET_H
#define _KSERVICELISTWIDGET_H

#include <QGroupBox>
#include <QListWidget>

#include <KPluginMetaData>
#include <kpluginmetadata.h>
#include <kservice.h>

class QPushButton;

class MimeTypeData;
class KService;

class KServiceListItem : public QListWidgetItem
{
public:
    explicit KServiceListItem(const KService::Ptr &pService);
    QString storageId;
    QString desktopPath;
    QString localPath;
};

class PluginListItem : public QListWidgetItem
{
public:
    PluginListItem(const KPluginMetaData &data);
    KPluginMetaData metaData;
};

/**
 * This widget holds a list of services, with 6 buttons to manage it.
 * It's a separate class so that it can be used by both tabs of the
 * module, once for applications and once for services.
 * The "kind" is determined by the argument given to the constructor.
 */
class KServiceListWidget : public QGroupBox
{
    Q_OBJECT
public:
    enum Kind {
        SERVICELIST_APPLICATIONS,
        SERVICELIST_SERVICES,
    };
    explicit KServiceListWidget(Kind kind, QWidget *parent = nullptr);

    void setMimeTypeData(MimeTypeData *item);

    void allowMultiApply(bool allow);

Q_SIGNALS:
    void changed(bool);
    void multiApply(int kind);

protected Q_SLOTS:
    void promoteService();
    void demoteService();
    void addService();
    void editService();
    void removeService();
    void applyTo();
    void enableMoveButtons();

protected:
    void updatePreferredServices();

private:
    Kind m_kind;
    QListWidget *servicesLB;
    QPushButton *servUpButton, *servDownButton;
    QPushButton *servNewButton, *servEditButton, *servRemoveButton;
    QPushButton *servApplyToButton;
    MimeTypeData *m_mimeTypeData;
    bool m_allowMultiApply;
};

#endif
