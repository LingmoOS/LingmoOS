/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <KTextTemplate/TemplateLoader>

class DesignWidget;

namespace KTextTemplate
{
class Engine;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private Q_SLOTS:
    void generateOutput();

private:
    void createOutputTab(const QString &label, const QString &content);

    void generateCpp();

private:
    DesignWidget *m_designWidget;
    KTextTemplate::Engine *m_engine;
    QSharedPointer<KTextTemplate::FileSystemTemplateLoader> m_loader;
    QTabWidget *m_tabWidget;
};

#endif
