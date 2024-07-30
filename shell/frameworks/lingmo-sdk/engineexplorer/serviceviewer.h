/*
 *   SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SERVICEVIEWER_H
#define SERVICEVIEWER_H

#include "ui_serviceviewer.h"
#include <QDialog>

class KJob;
class QPushButton;

namespace Lingmo5Support
{
class DataEngine;
class Service;
} // namespace Lingmo5Support

class ServiceViewer : public QDialog, public Ui::ServiceViewer
{
    Q_OBJECT

public:
    ServiceViewer(Lingmo5Support::DataEngine *engine, const QString &m_source, QWidget *parent = nullptr);
    ~ServiceViewer() override;

private:
    void updateJobCount(int numberOfJobs);

private Q_SLOTS:
    void updateOperations();
    void startOperation();
    void operationSelected(const QString &operation);
    void operationResult(KJob *job);
    void engineDestroyed();

private:
    Lingmo5Support::DataEngine *m_engine;
    Lingmo5Support::Service *m_service;
    QString m_source;
    int m_operationCount;
    QPushButton *m_operationButton;
};

#endif
