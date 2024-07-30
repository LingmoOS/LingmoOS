/*
    SPDX-FileCopyrightText: 2017 James D. Smith <smithjd15@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "usermetadatawritertest.h"
#include "indexerextractortestsconfig.h"
#include "usermetadata.h"

#include <QTest>
#include <QFile>

#define TEST_FILENAME "writertest.txt"
#define TEST_SYMLINK "dangling_symlink"

using namespace KFileMetaData;

QString UserMetaDataWriterTest::testFilePath(const QString& fileName) const
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + fileName;
}

void UserMetaDataWriterTest::initTestCase()
{
    m_writerTestFile.setFileName(testFilePath(TEST_FILENAME));
    QVERIFY(m_writerTestFile.open(QIODevice::WriteOnly | QIODevice::NewOnly));

    QFile::link(testFilePath("invalid_target"), testFilePath(TEST_SYMLINK));
}

void UserMetaDataWriterTest::testMissingPermision()
{
    m_writerTestFile.setPermissions(QFileDevice::ReadOwner);
    KFileMetaData::UserMetaData md(testFilePath(TEST_FILENAME));
    QVERIFY(md.isSupported());

    auto result = md.setAttribute("test", "my-value");
    QCOMPARE(result, KFileMetaData::UserMetaData::MissingPermission);

    QVERIFY(m_writerTestFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner));
}

void UserMetaDataWriterTest::testMetadataSize()
{
    KFileMetaData::UserMetaData md(testFilePath(TEST_FILENAME));
    QVERIFY(md.isSupported());

    // In the current ext2, ext3, and ext4 filesystem implementations,
    // the total bytes used by the names and values of all of a file's
    // extended attributes must fit in a single filesystem block (1/2/4 kB)

    // all implementations should support at least 512 B
    const auto smallSize = 512; // 512 B
    auto smallValue = QString(smallSize, 'a');
    auto result = md.setAttribute("test", smallValue);
    QCOMPARE(result, KFileMetaData::UserMetaData::NoError);
    QCOMPARE(md.attribute("test"), smallValue);

    // a big value, equal to the maximum value of an extended attribute according to Linux VFS
    // applies to XFS, btrfs...
    auto maxSize = 64 * 1024;
    const auto bigValue = QString(maxSize, 'a'); // 64 kB
    result = md.setAttribute("test", bigValue);
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    // BSD VFS has no such limit to 64 kB
    QCOMPARE(result, KFileMetaData::UserMetaData::NoError);
    QCOMPARE(md.attribute("test"), bigValue);
#else
    QCOMPARE(result, KFileMetaData::UserMetaData::NoSpace);
#endif

    // In Linux, The VFS-imposed limits on attribute names and
    // values are 255 bytes and 64 kB, respectively.
    auto excessiveValue = QString(maxSize + 1, 'a');
    result = md.setAttribute("test", excessiveValue);
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    QCOMPARE(result, KFileMetaData::UserMetaData::NoError);
    QCOMPARE(md.attribute("test"), excessiveValue);
#else
    // In Linux, we exceed the max value of an extended attribute, the error is different
    QCOMPARE(result, KFileMetaData::UserMetaData::ValueTooBig);
#endif

    // BSD and Linux have a limit of the attribute name of 255 bytes
    auto longName = QString(256, 'a');
    result = md.setAttribute(longName, "smallValue");
    QCOMPARE(result, KFileMetaData::UserMetaData::NameToolong);
}

void UserMetaDataWriterTest::test()
{

    KFileMetaData::UserMetaData md(testFilePath(TEST_FILENAME));
    QVERIFY(md.isSupported());

    // Tags
    md.setTags(QStringList() << QStringLiteral("this/is/a/test/tag"));
    QCOMPARE(md.tags().isEmpty() ? QStringLiteral("<no tags found>") : md.tags().at(0), QStringLiteral("this/is/a/test/tag"));
    QVERIFY(md.queryAttributes(UserMetaData::Attribute::Any) & UserMetaData::Attribute::Tags);
    QVERIFY(md.queryAttributes(UserMetaData::Attribute::All) & UserMetaData::Attribute::Tags);
    QVERIFY(md.queryAttributes(UserMetaData::Attribute::Tags) & UserMetaData::Attribute::Tags);
    QVERIFY(!(md.queryAttributes(UserMetaData::Attribute::Rating) & UserMetaData::Attribute::Tags));
    md.setTags(QStringList());
    QVERIFY(!md.hasAttribute(QStringLiteral("xdg.tags")));
    QVERIFY(!(md.queryAttributes(UserMetaData::Attribute::Tags) & UserMetaData::Attribute::Tags));

    // Rating
    md.setRating(3);
    QCOMPARE(md.rating(), 3);
    QVERIFY(md.queryAttributes(UserMetaData::Attribute::All) & UserMetaData::Attribute::Rating);
    QVERIFY(md.queryAttributes(UserMetaData::Attribute::Rating) & UserMetaData::Attribute::Rating);
    md.setRating(0);
    QVERIFY(!md.hasAttribute(QStringLiteral("baloo.rating")));
    QVERIFY(!(md.queryAttributes(UserMetaData::Attribute::All) & UserMetaData::Attribute::Rating));
    QVERIFY(!(md.queryAttributes(UserMetaData::Attribute::Rating) & UserMetaData::Attribute::Rating));

    // Comment
    md.setUserComment(QStringLiteral("this is a test comment"));
    QCOMPARE(md.userComment(), QStringLiteral("this is a test comment"));
    md.setUserComment(QString());
    QVERIFY(!md.hasAttribute(QStringLiteral("xdg.comment")));

    // Origin url
    md.setOriginUrl(QUrl("http://this.is.a.test.website.local"));
    QCOMPARE(md.originUrl(), QUrl("http://this.is.a.test.website.local"));
    md.setOriginUrl(QUrl());
    QVERIFY(!md.hasAttribute(QStringLiteral("xdg.origin.url")));

    // Origin e-mail subject
    md.setOriginEmailSubject(QStringLiteral("this is a test e-mail subject"));
    QCOMPARE(md.originEmailSubject(), QStringLiteral("this is a test e-mail subject"));
    md.setOriginEmailSubject(QString());
    QVERIFY(!md.hasAttribute(QStringLiteral("xdg.origin.email.subject")));

    // Origin e-mail sender
    md.setOriginEmailSender(QStringLiteral("Blue Bear"));
    QCOMPARE(md.originEmailSender(), QStringLiteral("Blue Bear"));
    md.setOriginEmailSender(QString());
    QVERIFY(!md.hasAttribute(QStringLiteral("xdg.origin.email.sender")));

    // Origin e-mail message id
    md.setOriginEmailMessageId(QStringLiteral("19991231235959.52234.24C26516HHBTF1C4"));
    QCOMPARE(md.originEmailMessageId(), QStringLiteral("19991231235959.52234.24C26516HHBTF1C4"));
    md.setOriginEmailMessageId(QString());
    QVERIFY(!md.hasAttribute(QStringLiteral("xdg.origin.email.message-id")));

    // Attribute
    md.setAttribute(QStringLiteral("test.attribute"), QStringLiteral("attribute"));
    QCOMPARE(md.attribute(QStringLiteral("test.attribute")), QStringLiteral("attribute"));
    md.setAttribute(QStringLiteral("test.attribute2"), QStringLiteral("attribute2"));
    QCOMPARE(md.attribute(QStringLiteral("test.attribute2")), QStringLiteral("attribute2"));
    QVERIFY(md.queryAttributes(UserMetaData::Attribute::All) & UserMetaData::Attribute::Other);
    QVERIFY(md.queryAttributes(UserMetaData::Attribute::Other) & UserMetaData::Attribute::Other);
    md.setAttribute(QStringLiteral("test.attribute"), QString());
    QVERIFY(!md.hasAttribute(QStringLiteral("test.attribute")));
    QVERIFY(md.queryAttributes(UserMetaData::Attribute::All) & UserMetaData::Attribute::Other);
    QVERIFY(md.queryAttributes(UserMetaData::Attribute::Other) & UserMetaData::Attribute::Other);
    md.setAttribute(QStringLiteral("test.attribute2"), QString());
    QVERIFY(!md.hasAttribute(QStringLiteral("test.attribute2")));

    // Check for side effects of calling sequence
    QVERIFY(!md.hasAttribute(QStringLiteral("test.check_contains")));
    md.setAttribute(QStringLiteral("test.check_contains"), QStringLiteral("dummy"));
    QVERIFY(md.hasAttribute(QStringLiteral("test.check_contains")));
    md.setAttribute(QStringLiteral("test.check_contains"), QString());
    QVERIFY(!md.hasAttribute(QStringLiteral("test.check_contains")));
}

void UserMetaDataWriterTest::testDanglingSymlink()
{
    KFileMetaData::UserMetaData md(testFilePath(TEST_SYMLINK));
    QVERIFY(md.queryAttributes(UserMetaData::Attribute::All) == UserMetaData::Attribute::None);
}

void UserMetaDataWriterTest::cleanupTestCase()
{
    m_writerTestFile.remove();
    QFile::remove(testFilePath(TEST_SYMLINK));
}

QTEST_GUILESS_MAIN(UserMetaDataWriterTest)

#include "moc_usermetadatawritertest.cpp"
