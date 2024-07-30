/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2015 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include <QDebug>
#include <QFileInfo>
#include <QTest>

#include "cachingloaderdecorator.h"
#include "context.h"
#include "engine.h"
#include "ktexttemplate_paths.h"
#include "template.h"
#include <metaenumvariable_p.h>

using Dict = QHash<QString, QVariant>;

Q_DECLARE_METATYPE(KTextTemplate::Error)

using namespace KTextTemplate;

class TestCachingLoader : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRenderAfterError();
};

void TestCachingLoader::testRenderAfterError()
{
    Engine engine;
    engine.setPluginPaths({QStringLiteral(KTEXTTEMPLATE_PLUGIN_PATH)});

    QSharedPointer<InMemoryTemplateLoader> loader(new InMemoryTemplateLoader);
    loader->setTemplate(QStringLiteral("template1"), QStringLiteral("This template has an error {{ va>r }}"));
    loader->setTemplate(QStringLiteral("template2"), QStringLiteral("Ok"));
    loader->setTemplate(QStringLiteral("main"), QStringLiteral("{% include template_var %}"));

    QSharedPointer<KTextTemplate::CachingLoaderDecorator> cache(new KTextTemplate::CachingLoaderDecorator(loader));

    engine.addTemplateLoader(cache);

    Context c;
    Template t;
    t = engine.loadByName(QStringLiteral("main"));

    c.insert(QStringLiteral("template_var"), QLatin1String("template1"));
    QCOMPARE(t->render(&c), QString());
    QCOMPARE(t->error(), TagSyntaxError);

    c.insert(QStringLiteral("template_var"), QLatin1String("template2"));
    QCOMPARE(t->render(&c), QLatin1String("Ok"));
    QCOMPARE(t->error(), NoError);
}

QTEST_MAIN(TestCachingLoader)
#include "testcachingloader.moc"
