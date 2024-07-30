/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

#include <KTextTemplate/TemplateLoader>
#include <QListWidget>
#include <QWebEngineView>

namespace KTextTemplate
{
class Engine;
}

class QComboBox;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &templateDir, QWidget *parent = nullptr);

protected:
    virtual void initLocalizer();

private Q_SLOTS:
    void render() const;
    void delayedInit();

protected:
    QSharedPointer<KTextTemplate::AbstractLocalizer> m_localizer;

private:
    QListWidget *m_list;
    QWebEngineView *m_webView;
    QComboBox *m_combo;
    QString m_templateDir;

    KTextTemplate::Engine *m_engine;
    QSharedPointer<KTextTemplate::FileSystemTemplateLoader> m_templateLoader;
};

template<typename T>
class AppMainWindow : public MainWindow
{
public:
    explicit AppMainWindow(const QString &templateDir, QWidget *parent = nullptr)
        : MainWindow(templateDir, parent)
    {
    }

protected:
    virtual void initLocalizer()
    {
        m_localizer = QSharedPointer<KTextTemplate::AbstractLocalizer>(new T);
    }
};

#endif
