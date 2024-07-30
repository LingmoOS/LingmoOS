/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_CONNECTION_EDITOR_DIALOG_H
#define PLASMA_NM_CONNECTION_EDITOR_DIALOG_H

#include "plasmanm_editor_export.h"

#include "connectioneditortabwidget.h"

#include <QDBusPendingCallWatcher>
#include <QDialog>
#include <QDialogButtonBox>

class PLASMANM_EDITOR_EXPORT ConnectionEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConnectionEditorDialog(const NetworkManager::ConnectionSettings::Ptr &connection,
                                    QWidget *parent = nullptr,
                                    Qt::WindowFlags f = Qt::WindowFlags());
    ~ConnectionEditorDialog() override;

    NMVariantMapMap setting() const;

private Q_SLOTS:
    void onValidityChanged(bool valid);

private:
    QDialogButtonBox *const m_buttonBox;
    ConnectionEditorTabWidget *const m_connectionEditorTabWidget;
};

#endif // PLASMA_NM_CONNECTION_EDITOR_BASE_H
