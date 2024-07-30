/*
    This file is part of the KDE KFileMetaData project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2017 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QObject>
#include <QTest>
#include <QDirIterator>
#include <QMimeDatabase>
#include <QMultiMap>

#include "mimeutils.h"

#include "indexerextractortestsconfig.h"

namespace KFileMetaData {

class ExtractorCoverageTest : public QObject
{
    Q_OBJECT

private:
    static QString filePath() {
        return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH);
    }

    QStringList m_testFiles;
    QMultiMap<QString, QString> m_knownFiles;

private Q_SLOTS:

    void initTestCase() {
        // Expected mimetypes
        m_knownFiles = {
            { "test_plain_text_newlines.txt",  "text/plain"},
            { "test_plain_text_file.txt",      "text/plain"},
            { "test_plain_text_file_win1251.txt",        "text/plain"},
            { "test_plain_text_file_GB18030.txt",        "text/plain"},
            { "test_plain_text_file_euc-jp.txt",         "text/plain"},
            { "test_plain_text_html_file_utf16.html",    "text/html"},
            { "test_plain_text_html_file_win1251.html",  "text/html"},
            { "test.avi",                      "video/vnd.avi"},
            { "test.aif",                      "audio/x-aifc"},
            { "test.ape",                      "audio/x-ape"},
            { "test.avif",                     "image/avif"},
            { "test.AppImage",                 "application/vnd.appimage"},
            { "test_apple_systemprofiler.spx", "application/x-apple-systemprofiler+xml"},  // s-m-i < 2.0 would give "application/xml"
            { "test.dot",                      "text/vnd.graphviz"},
            { "test.eps",                      "image/x-eps"},
            { "test.epub",                     "application/epub+zip"},
            { "test.fb2",                      "application/x-fictionbook+xml"},
            { "test.fb2.zip",                  "application/x-zip-compressed-fb2"},
            { "test.flac",                     "audio/flac"},
            { "test.heif",                     "image/heif"}, // alias for image/heic
            { "test.jpg",                      "image/jpeg"},
            { "test.jxl",                      "image/jxl"},
            { "test_libreoffice.docx",         "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
            { "test_libreoffice.pptx",         "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
            { "test_libreoffice.xlsx",         "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
            { "test.m4a",                      "audio/mp4"},
            { "test_missing_content.odt",      "application/vnd.oasis.opendocument.text"},
            { "test_missing_meta.odt",         "application/vnd.oasis.opendocument.text"},
            { "test.mkv",                      "video/x-matroska"},
            { "test.mp3",                      "audio/mpeg"},
            { "test.id3v1.mp3",                "audio/mpeg"},
            { "test.aax",                      "audio/vnd.audible.aax"},
            { "test.aaxc",                     "audio/vnd.audible.aaxc"},
            { "test.mpc",                      "audio/x-musepack"},
            { "test_no_gps.jpg",               "image/jpeg"},
            { "test.odg",                      "application/vnd.oasis.opendocument.graphics"},
            { "test.odp",                      "application/vnd.oasis.opendocument.presentation"},
            { "test.odt",                      "application/vnd.oasis.opendocument.text"},
            { "test_multicolumn.odt",          "application/vnd.oasis.opendocument.text"},
            { "test.ods",                      "application/vnd.oasis.opendocument.spreadsheet"},
            { "test.fodg",                     "application/vnd.oasis.opendocument.graphics-flat-xml"},
            { "test.fodp",                     "application/vnd.oasis.opendocument.presentation-flat-xml"},
            { "test.fodt",                     "application/vnd.oasis.opendocument.text-flat-xml"},
            { "test.ogg",                      "audio/x-vorbis+ogg"},
            { "test.flac.ogg",                 "audio/x-flac+ogg"},
            { "test.mml",                      "application/mathml+xml"},
            { "test_multivalue.ogg",           "audio/x-vorbis+ogg"},
            { "test.ogv",                      "video/x-theora+ogg"},
            { "test.opus",                     "audio/x-opus+ogg"},
            { "test.pdf",                      "application/pdf"},
            { "test_multicolumn.pdf",          "application/pdf"},
            { "test.pl",                       "application/x-perl"},
            { "test.ps",                       "application/postscript"},
            { "test_public_key.gpg",           "application/pgp-encrypted"},
            { "test_repeated.epub",            "application/epub+zip"},
            { "test.spx",                      "audio/x-speex+ogg"},
            { "test.ts",                       "video/mp2t"},
            // Check both the actual name and its alias for wav
            { "test.wav",                      "audio/vnd.wave"},
            { "test.wav",                      "audio/x-wav"},
            { "test.webm",                     "video/webm"},
            { "test.webp",                     "image/webp"},
            { "test_dcterms.svg",              "image/svg+xml"},
            { "test_with_container.svg",       "image/svg+xml"},
            { "test_with_metadata.svg",        "image/svg+xml"},
            { "test_with_metadata.svgz",       "image/svg+xml-compressed"},
            { "test.wma",                      "audio/x-ms-wma"},
            { "test.wv",                       "audio/x-wavpack"},
            { "test_zero_gps.jpg",             "image/jpeg"},
            { "test.mobi",                     "application/x-mobipocket-ebook"},
            { "test.png",                      "image/png"},
            { "test.kra",                      "application/x-krita"},
            { "test.mod",                      "audio/x-mod"},
            { "test.s3m",                      "audio/x-s3m"},
            { "test.it",                       "audio/x-it"},
            { "test.xm",                       "audio/x-xm"},
        };

        // Collect all test files from the samplefiles directory
        QDirIterator it(filePath(), {QStringLiteral("test*")}, QDir::Files);
        while (it.hasNext()) {
            it.next();
            m_testFiles.append(it.fileName());
        }
    }

    void testMimetype_data()
    {
        /*
         * Check if we get the correct mimetype for
         * each available test sample
         */
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<QString>("mimeType");

        auto it = m_knownFiles.cbegin();
        while (it != m_knownFiles.cend()) {
            QTest::addRow("%s_%s", it.key().toUtf8().constData(), it.value().toUtf8().constData())
                << it.key() << it.value();
            ++it;
        }
    }

    void testMimetype()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, mimeType);
        QString url = filePath() + QChar('/') + fileName;

        QMimeDatabase db;
        auto fileMime = MimeUtils::strictMimeType(url, db);

        QVERIFY(fileMime.isValid());
        if (fileMime.name() == "application/xml" && mimeType == "application/x-apple-systemprofiler+xml") {
            // s-m-i < 2.0 didn't have application/x-apple-systemprofiler+xml yet, it's all fine
            return;
        }
        if (!db.mimeTypeForName(mimeType).isValid()) {
            /* Examples when test could be skipped:
             * image/avif is available since s-m-i 2.0
             * image/jxl will be registered in s-m-i 2.2 or when libjxl is installed
             */
            QSKIP("Expected mimetype is not registered");
        }
        if (fileMime.name() != mimeType) {
            const auto aliases = fileMime.aliases();
	    if (!aliases.contains(mimeType))
		QCOMPARE(fileMime.name(), mimeType);
            QVERIFY(aliases.contains(mimeType));
        }
    }

    void testFileCoverage_data()
    {
        /*
         * Check if we get the correct mimetype for
         * each available test sample
         */
        QTest::addColumn<QString>("fileName");

        auto it = m_testFiles.cbegin();
        while (it != m_testFiles.cend()) {
            QTest::addRow("%s", it->toUtf8().constData()) << (*it);
            ++it;
        }
    }

    void testFileCoverage()
    {
        QFETCH(QString, fileName);

        QVERIFY2(m_knownFiles.contains(fileName), "test file omitted from test suite");
    }

};

}

QTEST_GUILESS_MAIN(KFileMetaData::ExtractorCoverageTest)

#include "extractorcoveragetest.moc"
