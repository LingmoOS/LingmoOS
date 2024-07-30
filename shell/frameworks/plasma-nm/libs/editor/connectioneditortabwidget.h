/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_CONNECTION_EDITOR_TAB_WIDGET_H
#define PLASMA_NM_CONNECTION_EDITOR_TAB_WIDGET_H

#include "plasmanm_editor_export.h"

#include "connectioneditorbase.h"

#include <QDBusPendingCallWatcher>
#include <QDialog>

namespace Ui
{
class ConnectionEditorTabWidget;
}

class PLASMANM_EDITOR_EXPORT ConnectionEditorTabWidget : public ConnectionEditorBase
{
    Q_OBJECT
public:
    explicit ConnectionEditorTabWidget(const NetworkManager::ConnectionSettings::Ptr &connection,
                                       QWidget *parent = nullptr,
                                       Qt::WindowFlags f = Qt::WindowFlags());
    ~ConnectionEditorTabWidget() override;

    void setConnection(const NetworkManager::ConnectionSettings::Ptr &connection) override;

protected:
    void addWidget(QWidget *widget, const QString &text) override;
    QString connectionName() const override;

private:
    Ui::ConnectionEditorTabWidget *const m_ui;

    void initializeTabWidget(const NetworkManager::ConnectionSettings::Ptr &connection);
};

#endif // PLASMA_NM_CONNECTION_EDITOR_TAB_WIDGET_H
