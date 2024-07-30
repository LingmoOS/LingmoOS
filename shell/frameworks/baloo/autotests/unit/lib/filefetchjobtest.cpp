/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "propertydata.h"

#include "file.h"
#include "document.h"
#include "database.h"
#include "transaction.h"
#include "idutils.h"
#include "global.h"

#include <QJsonDocument>
#include <QTest>
#include <QTemporaryFile>
#include <QTemporaryDir>

using namespace Baloo;

class FileFetchJobTest : public QObject
{
    Q_OBJECT

    QTemporaryDir dir;

private Q_SLOTS:
    void test();
};

void FileFetchJobTest::test()
{
    using namespace KFileMetaData;

    setenv("BALOO_DB_PATH", dir.path().toStdString().c_str(), 1);

    KFileMetaData::PropertyMultiMap map;
    map.insert(Property::Album, QStringLiteral("value1"));
    map.insert(Property::Artist, QStringLiteral("value2"));

    const QJsonObject jo = propertyMapToJson(map);
    const QJsonDocument jdoc(jo);

    QByteArray json = jdoc.toJson();
    QVERIFY(!json.isEmpty());

    QTemporaryFile tempFile;
    tempFile.open();

    Document doc;
    doc.setData(json);
    doc.setUrl(QFile::encodeName(tempFile.fileName()));
    doc.setId(filePathToId(doc.url()));
    doc.addTerm("testterm");
    doc.addFileNameTerm("filename");
    doc.setMTime(1);
    doc.setCTime(1);

    auto lastSlash = doc.url().lastIndexOf('/');
    auto parentId = filePathToId(doc.url().left(lastSlash));

    {
        Database db(fileIndexDbPath());
        db.open(Database::CreateDatabase);

        Transaction tr(db, Transaction::ReadWrite);
        QVERIFY(parentId);
        doc.setParentId(parentId);
        tr.addDocument(doc);
        tr.commit();
    }

    File file(tempFile.fileName());
    QVERIFY(file.load());
    QCOMPARE(file.properties(), map);
}

QTEST_MAIN(FileFetchJobTest)

#include "filefetchjobtest.moc"
