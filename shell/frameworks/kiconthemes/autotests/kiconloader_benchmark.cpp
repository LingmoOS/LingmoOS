/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kiconengine.h>
#include <kiconloader.h>

#include <QStandardPaths>
#include <QTest>

class KIconLoader_Benchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void init()
    {
#if 0 // Enable this code to benchmark very first startup.
      // Starting the application again uses the on-disk cache, so actually benchmarking -with- a cache is more relevant.
      // Remove icon cache
        const QString cacheFile = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/icon-cache.kcache";
        QFile::remove(cacheFile);

        // Clear SHM cache
        KIconLoader::global()->reconfigure(QString());
#endif
    }

    void benchmarkExistingIcons()
    {
        // icon list I get to load kwrite
        static QStringList icons = {QStringLiteral("accessories-text-editor"),
                                    QStringLiteral("bookmarks"),
                                    QStringLiteral("dialog-close"),
                                    QStringLiteral("edit-cut"),
                                    QStringLiteral("edit-paste"),
                                    QStringLiteral("edit-copy"),
                                    QStringLiteral("document-save"),
                                    QStringLiteral("edit-undo"),
                                    QStringLiteral("edit-redo"),
                                    QStringLiteral("code-context"),
                                    QStringLiteral("document-print"),
                                    QStringLiteral("document-print-preview"),
                                    QStringLiteral("view-refresh"),
                                    QStringLiteral("document-save-as"),
                                    QStringLiteral("preferences-other"),
                                    QStringLiteral("edit-select-all"),
                                    QStringLiteral("zoom-in"),
                                    QStringLiteral("zoom-out"),
                                    QStringLiteral("edit-find"),
                                    QStringLiteral("go-down-search"),
                                    QStringLiteral("go-up-search"),
                                    QStringLiteral("tools-check-spelling"),
                                    QStringLiteral("bookmark-new"),
                                    QStringLiteral("format-indent-more"),
                                    QStringLiteral("format-indent-less"),
                                    QStringLiteral("text-plain"),
                                    QStringLiteral("go-up"),
                                    QStringLiteral("go-down"),
                                    QStringLiteral("dialog-ok"),
                                    QStringLiteral("dialog-cancel"),
                                    QStringLiteral("window-close"),
                                    QStringLiteral("document-new"),
                                    QStringLiteral("document-open"),
                                    QStringLiteral("document-open-recent"),
                                    QStringLiteral("window-new"),
                                    QStringLiteral("application-exit"),
                                    QStringLiteral("show-menu"),
                                    QStringLiteral("configure-shortcuts"),
                                    QStringLiteral("configure-toolbars"),
                                    QStringLiteral("help-contents"),
                                    QStringLiteral("help-contextual"),
                                    QStringLiteral("tools-report-bug"),
                                    QStringLiteral("preferences-desktop-locale"),
                                    QStringLiteral("kde")};

        QBENCHMARK {
            for (const QString &iconName : std::as_const(icons)) {
                const QIcon icon = QIcon::fromTheme(iconName);
                if (icon.isNull()) {
                    QSKIP("missing icons");
                }
                QVERIFY(!icon.pixmap(24, 24).isNull());
                // QVERIFY(!icon.pixmap(512, 512).isNull());
            }
        }
    }

    void benchmarkNonExistingIcon_notCached()
    {
        QBENCHMARK {
            // Remove icon cache
            const QString cacheFile = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/icon-cache.kcache");
            QFile::remove(cacheFile);
            // Clear SHM cache
            KIconLoader::global()->reconfigure(QString());

            QIcon icon(new KIconEngine(QStringLiteral("invalid-icon-name"), KIconLoader::global()));
            QVERIFY(icon.isNull());
            QVERIFY2(icon.name().isEmpty(), qPrintable(icon.name()));
            QVERIFY(!icon.pixmap(QSize(16, 16), QIcon::Normal).isNull());
        }
    }

    void benchmarkNonExistingIcon_cached()
    {
        QBENCHMARK {
            QIcon icon(new KIconEngine(QStringLiteral("invalid-icon-name"), KIconLoader::global()));
            QVERIFY(icon.isNull());
            QVERIFY2(icon.name().isEmpty(), qPrintable(icon.name()));
            QVERIFY(!icon.pixmap(QSize(16, 16), QIcon::Normal).isNull());
        }
    }
};

QTEST_MAIN(KIconLoader_Benchmark)

#include "kiconloader_benchmark.moc"
