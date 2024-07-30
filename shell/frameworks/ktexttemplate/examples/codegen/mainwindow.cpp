/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "mainwindow.h"

#include <QLabel>
#include <QSplitter>
#include <QTextBrowser>

#include <QDebug>

#include <KTextTemplate/Context>
#include <KTextTemplate/Engine>
#include <KTextTemplate/FileSystemTemplateLoader>
#include <KTextTemplate/Util>

#include "ktexttemplate_paths.h"

#include "designwidget.h"

/**
  Don't escape the code generation output.

  'const QString &' should not become 'const QString &amp;'
*/
class NoEscapeOutputStream : public KTextTemplate::OutputStream
{
public:
    NoEscapeOutputStream()
        : KTextTemplate::OutputStream()
    {
    }

    NoEscapeOutputStream(QTextStream *stream)
        : OutputStream(stream)
    {
    }

    virtual QSharedPointer<KTextTemplate::OutputStream> clone() const
    {
        return QSharedPointer<KTextTemplate::OutputStream>(new NoEscapeOutputStream);
    }

    virtual QString escape(const QString &input) const
    {
        return input;
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QSplitter *splitter = new QSplitter(this);
    splitter->setStretchFactor(1, 5);

    m_designWidget = new DesignWidget(splitter);

    m_tabWidget = new QTabWidget(splitter);
    m_tabWidget->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);

    createOutputTab("Output", "Click Generate to create output");

    setCentralWidget(splitter);

    connect(m_designWidget, SIGNAL(generateClicked(bool)), SLOT(generateOutput()));

    m_engine = new KTextTemplate::Engine(this);
    m_engine->setPluginPaths(QStringList() << KTEXTTEMPLATE_PLUGIN_PATH << ":/plugins");
    m_engine->addDefaultLibrary("ktexttemplate_scriptabletags");
    m_engine->setSmartTrimEnabled(true);

    m_loader = QSharedPointer<KTextTemplate::FileSystemTemplateLoader>(new KTextTemplate::FileSystemTemplateLoader);
    m_loader->setTemplateDirs(QStringList() << ":/templates");
    m_engine->addTemplateLoader(m_loader);
    m_engine->addDefaultLibrary("custom_tags");
}

MainWindow::~MainWindow()
{
}

void MainWindow::generateOutput()
{
    m_tabWidget->clear();

    QString outputType = m_designWidget->outputType();
    m_loader->setTheme(outputType);

    if (outputType == "cpp")
        return generateCpp();

    KTextTemplate::Template classTemplate = m_engine->loadByName("class." + outputType);

    if (classTemplate->error()) {
        createOutputTab("Class", classTemplate->errorString());
        return;
    }

    KTextTemplate::Context c = m_designWidget->getContext();

    QString output;
    QTextStream textStream(&output);
    NoEscapeOutputStream stream(&textStream);

    classTemplate->render(&stream, &c);
    createOutputTab("Class", classTemplate->error() ? classTemplate->errorString() : output);
}

void MainWindow::generateCpp()
{
    KTextTemplate::Template headerTemplate = m_engine->loadByName("header.h");

    if (headerTemplate->error()) {
        createOutputTab("Header", headerTemplate->errorString());
        return;
    }

    KTextTemplate::Context c = m_designWidget->getContext();

    QString output;
    QTextStream textStream(&output);
    NoEscapeOutputStream stream(&textStream);

    headerTemplate->render(&stream, &c);
    createOutputTab("Header", headerTemplate->error() ? headerTemplate->errorString() : output);
    if (headerTemplate->error())
        return;

    output.clear();

    KTextTemplate::Template implementationTemplate = m_engine->loadByName("implementation.cpp");

    if (implementationTemplate->error()) {
        createOutputTab("Implementation", implementationTemplate->errorString());
        return;
    }

    implementationTemplate->render(&stream, &c);
    createOutputTab("Implementation", implementationTemplate->error() ? implementationTemplate->errorString() : output);
    if (implementationTemplate->error())
        return;
    output.clear();

    if (c.lookup("pimpl").toBool()) {
        KTextTemplate::Template privateHeaderTemplate = m_engine->loadByName("private_header.h");

        if (privateHeaderTemplate->error()) {
            createOutputTab("Private Header", privateHeaderTemplate->errorString());
            return;
        }
        c.insert("className", KTextTemplate::getSafeString(c.lookup("className")) + QString("Private"));
        c.insert("baseClass", QVariant());
        privateHeaderTemplate->render(&stream, &c);
        createOutputTab("Private Header", privateHeaderTemplate->error() ? privateHeaderTemplate->errorString() : output);
        if (privateHeaderTemplate->error())
            return;
    }
}

void MainWindow::createOutputTab(const QString &label, const QString &content)
{
    QTextBrowser *browser = new QTextBrowser(m_tabWidget);
    QFont f;
    f.setFamily("monospace");
    browser->setCurrentFont(f);

    m_tabWidget->addTab(browser, label);

    browser->setPlainText(content);
}

#include "moc_mainwindow.cpp"
