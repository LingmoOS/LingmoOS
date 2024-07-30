/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PROJECTTEST_H
#define PROJECTTEST_H

#include "ui_editproject.h"

#include <QLabel>
#include <QMainWindow>

#include <project.h>
#include <providermanager.h>

namespace Attica
{
class Project;
class BaseJob;
class Provider;
}

class ProjectTest : public QMainWindow
{
    Q_OBJECT
public:
    ProjectTest();
    ~ProjectTest() override;

private Q_SLOTS:
    void providerAdded(const Attica::Provider &provider);

    void projectListResult(Attica::BaseJob *);
    void projectResult(Attica::BaseJob *);
    void createProjectResult(Attica::BaseJob *j);
    void saveProjectResult(Attica::BaseJob *j);
    void deleteProjectResult(Attica::BaseJob *j);

    void buildServiceListResult(Attica::BaseJob *);
    void buildServiceJobListResult(Attica::BaseJob *);

    void save();
    void create();
    void deleteProject(); // don't clashs with keyword

    void selectedBuildServiceChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void createBuildServiceJob();
    void buildServiceJobCreated(Attica::BaseJob *j);
    void cancelBuildServiceJob();
    void buildServiceJobCanceled(Attica::BaseJob *j);

    void updateCurrentProject();

private:
    void initOcs();
    void getProject(QString id);
    void deleteProject(QString id);
    void listProjects();
    void listBuildServices();
    void listBuildServiceJobs(const Attica::Project &p);

    void setStatus(QString status);
    Attica::Project uiToProject();
    void projectToUi(const Attica::Project &p);
    QString currentProject();

    Attica::ProviderManager m_pm;
    Attica::Provider m_provider;
    QHash<QString, QSharedPointer<Attica::Provider>> m_providers;

    QString m_currentProjectId;

    QWidget *m_mainWidget;

    Ui::EditProject *m_editor;

    QHash<QString, Attica::BuildService> m_buildServices;
    QHash<QString, Attica::BuildServiceJob> m_buildServiceJobs;
    QHash<QString, Attica::Project> m_projects;
};

#endif // PROJECTTEST_H
