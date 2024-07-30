/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "mainwindow.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QWebView>

#include "ktexttemplate_paths.h"
#include <KTextTemplate/Engine>
#include <KTextTemplate/FileSystemTemplateLoader>

#include "localrequestmanager.h"
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    QWebView *wv = new QWebView;

    KTextTemplate::Engine *engine = new KTextTemplate::Engine(this);
    engine->addDefaultLibrary("customplugin");

    QSharedPointer<KTextTemplate::FileSystemTemplateLoader> loader(new KTextTemplate::FileSystemTemplateLoader);
    loader->setTemplateDirs(QStringList() << KTEXTTEMPLATE_TEMPLATE_PATH);
    engine->addTemplateLoader(loader);

    wv->page()->setNetworkAccessManager(new LocalRequestManager(engine, this));

    wv->load(QUrl("template:///home/main.html"));

    layout->addWidget(wv);
}

#include "moc_mainwindow.cpp"
