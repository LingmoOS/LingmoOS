/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SCRIPTABLETAGSTEST_H
#define SCRIPTABLETAGSTEST_H

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTest>

#include "context.h"
#include "engine.h"
#include "ktexttemplate_paths.h"
#include "template.h"

using Dict = QHash<QString, QVariant>;

Q_DECLARE_METATYPE(KTextTemplate::Error)

using namespace KTextTemplate;

class TestScriptableTagsSyntax : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testBasicSyntax_data();
    void testBasicSyntax()
    {
        doTest();
    }

    void testResolve_data();
    void testResolve()
    {
        doTest();
    }

    void cleanupTestCase();

private:
    void doTest();

    Engine *m_engine = nullptr;
};

void TestScriptableTagsSyntax::initTestCase()
{
    Q_INIT_RESOURCE(testresource);

    m_engine = new Engine(this);
    auto appDirPath = QFileInfo(QCoreApplication::applicationDirPath()).absoluteDir().path();
    m_engine->setPluginPaths({
        QStringLiteral(KTEXTTEMPLATE_PLUGIN_PATH),
        QStringLiteral(":/plugins/") // For scripteddefaults.qs
    });
    m_engine->addDefaultLibrary(QStringLiteral("ktexttemplate_scriptabletags"));
}

void TestScriptableTagsSyntax::cleanupTestCase()
{
    delete m_engine;
}

void TestScriptableTagsSyntax::doTest()
{
    QFETCH(QString, input);
    QFETCH(Dict, dict);
    QFETCH(QString, output);
    QFETCH(KTextTemplate::Error, error);

    auto t = m_engine->newTemplate(input, QLatin1String(QTest::currentDataTag()));

    if (t->error() != NoError) {
        if (t->error() != error)
            qDebug() << t->errorString();
        QCOMPARE(t->error(), error);
        return;
    }

    Context context(dict);

    auto result = t->render(&context);

    if (t->error() != NoError) {
        if (t->error() != error)
            qDebug() << t->errorString();
        QCOMPARE(t->error(), error);
        return;
    }

    QCOMPARE(t->error(), NoError);

    // Didn't catch any errors, so make sure I didn't expect any.
    QCOMPARE(NoError, error);

    QCOMPARE(t->error(), NoError);

    QCOMPARE(result, output);
}

void TestScriptableTagsSyntax::testBasicSyntax_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<Dict>("dict");
    QTest::addColumn<QString>("output");
    QTest::addColumn<KTextTemplate::Error>("error");

    Dict dict;

    dict.insert(QStringLiteral("boo"), QStringLiteral("Far"));
    dict.insert(QStringLiteral("booList"), QVariantList{QStringLiteral("Tom"), QStringLiteral("Dick"), QStringLiteral("Harry")});

    QTest::newRow("scriptable-tags01") << "{% load scripteddefaults %}{% if2 "
                                          "\"something\\\" stupid\" %}{{ boo "
                                          "}}{% endif2 %}"
                                       << dict << QStringLiteral("Far") << NoError;

    // Nest c++ tags inside scripted tags.
    QTest::newRow("scriptable-tags02") << "{% load scripteddefaults %}{% if2 \"something\\\" stupid\" %}{% "
                                          "for "
                                          "name in booList %}:{{ name }};{% endfor %}{% endif2 %}"
                                       << dict << QStringLiteral(":Tom;:Dick;:Harry;") << NoError;

    // Nest c++ tags inside scripted tags.
    QTest::newRow("scriptable-tags03") << QStringLiteral("{% load scripteddefaults %}{% if2 boo %}yes{% else %}no{% endif2 %}") << dict << QStringLiteral("yes")
                                       << NoError;
    QTest::newRow("scriptable-tags04") << QStringLiteral("{% load scripteddefaults %}{% if2 foo %}yes{% else %}no{% endif2 %}") << dict << QStringLiteral("no")
                                       << NoError;

    QTest::newRow("scriptable-tags05") << QStringLiteral("{% load scripteddefaults %}{{ boo|upper }}") << dict << QStringLiteral("FAR") << NoError;

    dict.insert(QStringLiteral("boo"), QStringLiteral("Far & away"));
    // Variables are escaped ...
    QTest::newRow("scriptable-tags06") << QStringLiteral("{% load scripteddefaults %}{{ boo }}") << dict << QStringLiteral("Far &amp; away") << NoError;
    // and scripted filters can mark output as 'safe'.
    QTest::newRow("scriptable-tags07") << QStringLiteral("{% load scripteddefaults %}{{ boo|safe2 }}") << dict << QStringLiteral("Far & away") << NoError;

    // Filters can take arguments
    QTest::newRow("scriptable-tags08") << "{% load scripteddefaults %}{{ booList|join2:\" \" }}" << dict << QStringLiteral("Tom Dick Harry") << NoError;

    // Literals in tags are compared un-escaped.
    QTest::newRow("scriptable-tags09") << "{% load scripteddefaults %}{% ifequal2 boo \"Far & away\" %}yes{% "
                                          "endifequal2 %}"
                                       << dict << QStringLiteral("yes") << NoError;

    // Literals in arguments to filters are not escaped.
    QTest::newRow("scriptable-tags10") << "{% load scripteddefaults %}{{ booList|join2:\" & \" }}" << dict << QStringLiteral("Tom & Dick & Harry") << NoError;

    // Nor are variables.
    dict.insert(QStringLiteral("amp"), QStringLiteral(" & "));
    QTest::newRow("scriptable-tags11") << QStringLiteral("{% load scripteddefaults %}{{ booList|join2:amp }}") << dict << QStringLiteral("Tom & Dick & Harry")
                                       << NoError;

    QTest::newRow("scriptable-load-error01") << QStringLiteral("{% load %}{{ booList|join2:amp }}") << dict << QString() << TagSyntaxError;

    dict.clear();
}

void TestScriptableTagsSyntax::testResolve_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<Dict>("dict");
    QTest::addColumn<QString>("output");
    QTest::addColumn<KTextTemplate::Error>("error");

    Dict dict;
    dict.insert(QStringLiteral("boo"), QStringLiteral("Far"));
    dict.insert(QStringLiteral("zing"), QStringLiteral("Bang"));

    QTest::newRow("resolve-01") << "{% load scripteddefaults %}{% resolver boo zing %}" << dict << QStringLiteral("Far - Bang") << NoError;
}

QTEST_MAIN(TestScriptableTagsSyntax)
#include "testscriptabletags.moc"

#endif
