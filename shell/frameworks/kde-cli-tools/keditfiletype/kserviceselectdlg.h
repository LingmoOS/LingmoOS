/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef __kserviceselectdlg_h
#define __kserviceselectdlg_h

#include <QDialog>
#include <QListWidget>

#include <KPluginMetaData>

class QDialogButtonBox;

class KPartSelectDlg : public QDialog
{
    Q_OBJECT
public:
    /**
     * Create a dialog to select a service (not application) for a given service type.
     *
     * @param serviceType the service type we want to choose a service for.
     * @param value is the initial service to select (not implemented currently)
     * @param parent parent widget
     */
    explicit KPartSelectDlg(QWidget *parent = nullptr);

    ~KPartSelectDlg() override;

    /**
     * @return the chosen service
     */
    KPluginMetaData chosenPart();

private:
    QListWidget *m_listbox;
    QDialogButtonBox *m_buttonBox;
};

#endif
