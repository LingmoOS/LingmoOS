/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KDirOperator>
#include <KFileWidget>
#include <QDir>
#include <QFileDialog>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTimer>

Q_DECLARE_METATYPE(QFileDialog::ViewMode)
Q_DECLARE_METATYPE(QFileDialog::FileMode)
Q_DECLARE_METATYPE(KFile::FileView)
Q_DECLARE_METATYPE(KFile::Modes)

class KFileDialog_UnitTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
    }

    void init()
    {
    }

    void cleanupTestCase()
    {
    }

    void testSetNameFilters()
    {
        QFileDialog dialog;

        QStringList nameFilterList = QStringList() << QStringLiteral("c (*.cpp)") << QStringLiteral("h (*.h)");
        dialog.setNameFilters(nameFilterList);
        QCOMPARE(dialog.nameFilters(), nameFilterList);
    }

    void testSelectNameFilter()
    {
        QFileDialog dialog;

        QStringList nameFilterList = QStringList() << QStringLiteral("c (*.cpp)") << QStringLiteral("h (*.h)");
        dialog.setNameFilters(nameFilterList);
        QCOMPARE(dialog.nameFilters(), nameFilterList);

        QString selectNameFilter(QStringLiteral("h (*.h)"));
        dialog.selectNameFilter(selectNameFilter);
        QEXPECT_FAIL("", "Does currently not work. Works, once the dialog gets shown, though.", Continue);
        QCOMPARE(dialog.selectedNameFilter(), selectNameFilter);

        dialog.show();
        QCOMPARE(dialog.selectedNameFilter(), selectNameFilter);
    }

    void testSelectNameFilterMultipleMatching()
    {
        QFileDialog dialog;

        QStringList nameFilterList = QStringList() << QStringLiteral("c (*.cpp)") << QStringLiteral("h1 (*.h)") << QStringLiteral("h2 (*.h)");
        dialog.setNameFilters(nameFilterList);
        QCOMPARE(dialog.nameFilters(), nameFilterList);

        QString selectNameFilter(QStringLiteral("h2 (*.h)"));
        dialog.selectNameFilter(selectNameFilter);
        QEXPECT_FAIL("", "Does currently not work. Works, once the dialog gets shown, though.", Continue);
        QCOMPARE(dialog.selectedNameFilter(), selectNameFilter);

        dialog.show();
        QCOMPARE(dialog.selectedNameFilter(), selectNameFilter);
    }

    void testSelectedMimeTypeFilter_data()
    {
        QTest::addColumn<QStringList>("mimeTypeFilters");
        QTest::addColumn<QString>("targetMimeTypeFilter");

        const auto headerMime = QStringLiteral("text/x-chdr");
        const auto jsonMime = QStringLiteral("application/json");
        const auto zipMime = QStringLiteral("application/zip");

        QTest::newRow("single mime filter (C header file)") << QStringList{headerMime} << headerMime;

        QTest::newRow("single mime filter (JSON file)") << QStringList{jsonMime} << jsonMime;

        QTest::newRow("multiple mime filters") << QStringList{jsonMime, zipMime} << jsonMime;
    }

    void testSelectedMimeTypeFilter()
    {
        QFileDialog dialog;

        QFETCH(QStringList, mimeTypeFilters);
        dialog.setMimeTypeFilters(mimeTypeFilters);

        QFETCH(QString, targetMimeTypeFilter);
        dialog.selectMimeTypeFilter(targetMimeTypeFilter);

        dialog.show();
        QCOMPARE(dialog.selectedMimeTypeFilter(), targetMimeTypeFilter);
    }

    void testFallbackOnFirstFilterInSaveMode()
    {
        QFileDialog dialog;
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setMimeTypeFilters({QStringLiteral("application/json"), QStringLiteral("application/zip")});
        dialog.show();
        QCOMPARE(dialog.selectedMimeTypeFilter(), QStringLiteral("application/json"));
    }

    void testSetDirectory()
    {
        QFileDialog dialog;
        dialog.setDirectory(QDir::rootPath());
        QCOMPARE(dialog.directory().absolutePath(), QDir::rootPath());
    }

    void testSelectUrl()
    {
        QTemporaryFile tempFile(m_tempDir.path() + "/kfiledialogtest_XXXXXX");
        tempFile.setAutoRemove(true);
        tempFile.open();
        QString tempName = tempFile.fileName();
        QUrl url = QUrl::fromLocalFile(tempName);
        int idx = tempName.lastIndexOf('/');
        QUrl directoryUrl = QUrl::fromLocalFile(tempName.left(idx + 1));

        QFileDialog dialog;
        dialog.selectUrl(url);
        dialog.show();

        // check if dialog was set to base directory url of the passed file url
        QCOMPARE(dialog.directoryUrl(), directoryUrl);
    }

    void testGetSaveFileUrl()
    {
        QObject lambdaGuard;
        QTemporaryFile tempFile(m_tempDir.path() + "/kfiledialogtest_XXXXXX");
        tempFile.open();
        const QString tempName = tempFile.fileName();
        const QUrl url = QUrl::fromLocalFile(tempName);

        // Need to use a lambda and not just QTest::qWaitForWindowExposed();
        // because with the static getSaveFileUrl we do not have access
        // to the QFileDialog object, so instead we hook to a signal
        KFileWidget::OperationMode saveFileOperationMode = KFileWidget::Other;
        connect(qApp, &QGuiApplication::focusWindowChanged, &lambdaGuard, [&saveFileOperationMode] {
            KFileWidget *fileWidget = findFileWidget();
            saveFileOperationMode = fileWidget->operationMode();
            qApp->activeWindow()->close();
        });

        QFileDialog::getSaveFileUrl(nullptr, QString(), url);

        QCOMPARE(saveFileOperationMode, KFileWidget::Saving);
    }

    void testViewMode()
    {
        // Open a file dialog, and change view mode to tree
        {
            QFileDialog dialog;
            dialog.show();

            KFileWidget *fw = findFileWidget();
            QVERIFY(fw);
            fw->setViewMode(KFile::Tree);
            fw->slotCancel(); // the saving happens there
        }
        // Open another one, and check that the view mode is now tree, change it to simple
        {
            QFileDialog dialog;
            dialog.show();

            KFileWidget *fw = findFileWidget();
            QVERIFY(fw);
            KDirOperator *op = fw->dirOperator();
            QCOMPARE(fileViewToString(op->viewMode()), fileViewToString(KFile::Tree));
            fw->setViewMode(KFile::Simple);
            fw->slotCancel();
        }
        // Open another one, and check that the view mode is now simple
        {
            QFileDialog dialog;
            dialog.show();

            KFileWidget *fw = findFileWidget();
            QVERIFY(fw);
            KDirOperator *op = fw->dirOperator();
            QCOMPARE(fileViewToString(op->viewMode()), fileViewToString(KFile::Simple));
            fw->setViewMode(KFile::Detail);
            fw->slotCancel();
        }
    }

    void testOpenDialog()
    {
        // Open parentless
        {
            QFileDialog dialog;
            dialog.open();

            KFileWidget *fw = findFileWidget();
            QVERIFY(fw);
            QCOMPARE(fw->isVisible(), true);
            fw->slotCancel();
        }
        // Open with parent
        {
            QWidget w;
            w.show();

            QFileDialog dialog(&w);
            dialog.open();

            KFileWidget *fw = findFileWidget();
            QVERIFY(fw);
            QCOMPARE(fw->isVisible(), true);
            fw->slotCancel();
        }
    }

    void testShowDialog()
    {
        // Show parentless
        {
            QFileDialog dialog;
            dialog.show();

            KFileWidget *fw = findFileWidget();
            QVERIFY(fw);
            QCOMPARE(fw->isVisible(), true);
            fw->slotCancel();
        }
        // Show with parent
        {
            QWidget w;
            w.show();

            QFileDialog dialog(&w);
            dialog.show();

            KFileWidget *fw = findFileWidget();
            QVERIFY(fw);
            QCOMPARE(fw->isVisible(), true);
            fw->slotCancel();
        }
    }

    void testSetFileMode_data()
    {
        QTest::addColumn<QFileDialog::FileMode>("qtFileMode");
        QTest::addColumn<KFile::Modes>("kdeFileMode");
        QTest::newRow("anyfile") << QFileDialog::AnyFile << KFile::Modes(KFile::File);
        QTest::newRow("existingfile") << QFileDialog::ExistingFile << KFile::Modes(KFile::File | KFile::ExistingOnly);
        QTest::newRow("directory") << QFileDialog::Directory << KFile::Modes(KFile::Directory | KFile::ExistingOnly);
        QTest::newRow("existingfiles") << QFileDialog::ExistingFiles << KFile::Modes(KFile::Files | KFile::ExistingOnly);
    }

    void testSetFileMode()
    {
        QFETCH(QFileDialog::FileMode, qtFileMode);
        QFETCH(KFile::Modes, kdeFileMode);
        QFileDialog dialog;
        dialog.setFileMode(qtFileMode);
        dialog.show();

        KFileWidget *fw = findFileWidget();
        QVERIFY(fw);
        QCOMPARE(fw->mode(), kdeFileMode);

        QCOMPARE(dialog.fileMode(), qtFileMode);
    }

    void testSaveOverwrite_data()
    {
        QTest::addColumn<bool>("qtOverwriteOption");
        QTest::addColumn<bool>("messageBoxExpected");
        QTest::newRow("checkoverwrite") << false << true;
        QTest::newRow("allowoverwrite") << true << false;
    }

    void testSaveOverwrite()
    {
        QFETCH(bool, qtOverwriteOption);
        QFETCH(bool, messageBoxExpected);

        QTemporaryFile tempFile(m_tempDir.path() + "/kfiledialogtest_XXXXXX");
        tempFile.setAutoRemove(true);
        tempFile.open();
        QString tempName = tempFile.fileName();
        tempFile.close();
        int idx = tempName.lastIndexOf('/');

        QFileDialog dialog;
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        if (qtOverwriteOption)
            dialog.setOption(QFileDialog::DontConfirmOverwrite);
        dialog.setDirectory(tempName.left(idx + 1));
        dialog.selectFile(tempName.mid(idx + 1));
        dialog.open();

        KFileWidget *fw = findFileWidget();
        QVERIFY(fw);
        QVERIFY(QTest::qWaitForWindowExposed(fw->window()));
        QCOMPARE(fw->isVisible(), true);

        bool timerRun = false;

        QTimer::singleShot(3500, this, [&] {
            timerRun = true;
            QDialog *msgbox = findMessageBox();
            if (msgbox) {
                QVERIFY(QTest::qWaitForWindowExposed(msgbox));
                QCOMPARE(msgbox->isVisible(), true);
                msgbox->close();
                QVERIFY(messageBoxExpected);
            } else {
                QVERIFY(!messageBoxExpected);
            }
        });
        fw->slotOk();

        QTRY_VERIFY(timerRun);
    }

    void testRememberLastDirectory()
    {
        const QUrl dir = QUrl::fromLocalFile(QDir::tempPath()).adjusted(QUrl::StripTrailingSlash);
        // Open and navigate
        {
            QFileDialog dialog;
            dialog.open();

            KFileWidget *fw = findFileWidget();
            QVERIFY(fw);
            QCOMPARE(fw->isVisible(), true);
            fw->setUrl(dir);
            fw->slotCancel();
        }
        // Open another filedialog, check that the default directory is the one from above
        {
            QFileDialog dialog;
            dialog.open();

            KFileWidget *fw = findFileWidget();
            QVERIFY(fw);
            QCOMPARE(fw->isVisible(), true);
            QCOMPARE(dialog.directoryUrl().adjusted(QUrl::StripTrailingSlash), dir);
            fw->slotCancel();
        }
    }

private:
    QTemporaryDir m_tempDir;

    static QString fileViewToString(KFile::FileView fv)
    {
        switch (fv) {
        case KFile::Detail:
            return QStringLiteral("Detail");
        case KFile::Simple:
            return QStringLiteral("Simple");
        case KFile::Tree:
            return QStringLiteral("Tree");
        case KFile::DetailTree:
            return QStringLiteral("DetailTree");
        default:
            break;
        }
        return QStringLiteral("ERROR");
    }

    static KFileWidget *findFileWidget()
    {
        QList<KFileWidget *> widgets;
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            KFileWidget *fw = widget->findChild<KFileWidget *>();
            if (fw) {
                widgets.append(fw);
            }
        }
        Q_ASSERT(widgets.count() == 1);
        return (widgets.count() == 1) ? widgets.first() : nullptr;
    }

    static QDialog *findMessageBox()
    {
        QList<QDialog *> widgets;
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            QDialog *dlg = widget->findChild<QDialog *>();
            if (dlg) {
                widgets.append(dlg);
            }
        }
        return (widgets.count() == 1) ? widgets.first() : nullptr;
    }
};

QTEST_MAIN(KFileDialog_UnitTest)

#include "kfiledialog_unittest.moc"
