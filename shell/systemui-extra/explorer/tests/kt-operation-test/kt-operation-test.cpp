#include "kt-operation-test.h"
#include "explorer-qt/file-node.h"
#include "explorer-qt/file-node-reporter.h"
#include "explorer-qt/file-operation-error-dialog.h"
#include "explorer-qt/file-operation-error-handler.h"
#include "explorer-qt/file-operation-progress-bar.h"
#include "explorer-qt/file-enumerator.h"
#include <QDebug>
#include <QMessageBox>
#include <QtTest/QTest>

#include <QThread>
#include <QThreadPool>

#include <QDialog>
#include <QFileDialog>
#include <QUrl>
#include <QStandardPaths>

void KTOperationTest::testLibrary(){

    testLibraryMove();

//    testLibraryCopy();

//    testLibraryDelete();

//    testLibraryTrash();

//    testLibraryLink();

//    testLibraryRename();

//    testLibraryUntrash();
}

void KTOperationTest::testLibraryMove()
{
    auto button = QMessageBox::question(nullptr, tr("move source files"), tr("choose move source files，"
                          "use cancel to finish the choices."));
    if (button != QMessageBox::Yes) {
      return;
    }
    QFileDialog srcdlg;
    srcdlg.setFileMode(QFileDialog::ExistingFiles);
    srcdlg.setAcceptMode(QFileDialog::AcceptSave);
    srcdlg.setViewMode(QFileDialog::List);

    QStringList srcUris;
    srcdlg.exec();

    for (auto uri : srcdlg.selectedUrls()) {
        srcUris<<uri.toString();
    }
    auto button1 = QMessageBox::question(nullptr, tr("move dest dir"), tr("choose move dest dir,"
                          "use cancel to finish the choices."));
    if (button1 != QMessageBox::Yes) {
        return;
    }
    QFileDialog destdlg;
    destdlg.setAcceptMode(QFileDialog::AcceptSave);
    destdlg.setFileMode(QFileDialog::Directory);

    QString destUri;
    QUrl targetDir = destdlg.getExistingDirectoryUrl();
    if (targetDir.isEmpty()) {
        return;
    }
    destUri = targetDir.url();
    Peony::FileMoveOperation *moveOp = new Peony::FileMoveOperation(srcUris, destUri);
    moveOp->setForceUseFallback();
    QSignalSpy spyStart(moveOp, &Peony::FileOperation::operationFinished);
    QSignalSpy spyError(moveOp, &Peony::FileOperation::errored);
    moveOp->connect(moveOp, &Peony::FileOperation::errored,
                    this, &KTOperationTest::handleError,
                    Qt::BlockingQueuedConnection);
    // precondition
    QVERIFY(spyStart.isValid());
    QVERIFY(spyStart.isEmpty());
    QVERIFY(spyError.isValid());
    QVERIFY(spyError.isEmpty());

    const unsigned long int Period = 5000;
    QVERIFY(QThreadPool::globalInstance()->tryStart(moveOp));

    QVERIFY(spyStart.wait(Period * 10));
    QVERIFY(!spyError.wait(Period * 10));
    // postcondition
    const int startResult = spyStart.count();
    QCOMPARE(startResult, 1);
    QList<QVariant> theCall1 = spyStart.takeFirst();
    QVERIFY(theCall1.isEmpty());
    const int errorResult = spyError.count();
    QCOMPARE(errorResult, 0);
}

void KTOperationTest::testLibraryCopy()
{
    auto button = QMessageBox::question(nullptr, tr("copy source files"), tr("choose copy source files，"
                          "use cancel to finish the choices."));
    if (button != QMessageBox::Yes) {
      return;
    }
    QFileDialog srcdlg;
    srcdlg.setFileMode(QFileDialog::ExistingFiles);
    srcdlg.setAcceptMode(QFileDialog::AcceptSave);
    srcdlg.setViewMode(QFileDialog::List);

    QStringList srcUris;
    srcdlg.exec();

    for (auto uri : srcdlg.selectedUrls()) {
        srcUris<<uri.toString();
    }
    auto button1 = QMessageBox::question(nullptr, tr("copy dest dir"), tr("choose copy dest dir,"
                          "use cancel to finish the choices."));
    if (button1 != QMessageBox::Yes) {
        return;
    }
    QFileDialog destdlg;
    destdlg.setAcceptMode(QFileDialog::AcceptSave);
    destdlg.setFileMode(QFileDialog::Directory);

    QString destUri;
    QUrl targetDir = destdlg.getExistingDirectoryUrl();
    if (targetDir.isEmpty()) {
        return;
    }
    destUri = targetDir.url();

    Peony::FileCopyOperation *copyOp = new Peony::FileCopyOperation(srcUris, destUri);
    QSignalSpy spyStart(copyOp, &Peony::FileOperation::operationFinished);
    QSignalSpy spyError(copyOp, &Peony::FileOperation::errored);
    copyOp->connect(copyOp, &Peony::FileOperation::errored,
                    this, &KTOperationTest::handleError,
                    Qt::BlockingQueuedConnection);
    // precondition
    QVERIFY(spyStart.isValid());
    QVERIFY(spyStart.isEmpty());
    QVERIFY(spyError.isValid());
    QVERIFY(spyError.isEmpty());

    const unsigned long int Period = 50000;
    QVERIFY(QThreadPool::globalInstance()->tryStart(copyOp));

    QVERIFY(spyStart.wait(Period * 10));
    QVERIFY(!spyError.wait(Period * 10));
    // postcondition
    const int startResult = spyStart.count();
    QCOMPARE(startResult, 1);
    QList<QVariant> theCall = spyStart.takeFirst();
    QVERIFY(theCall.isEmpty());
    const int errorResult = spyError.count();
        QCOMPARE(errorResult, 0);
}

void KTOperationTest::testLibraryDelete()
{
    auto button = QMessageBox::question(nullptr, tr("delete files"), tr("choose delete files，"
                          "use cancel to finish the choices."));
    if (button != QMessageBox::Yes) {
      return;
    }
    QFileDialog srcdlg;
    srcdlg.setFileMode(QFileDialog::ExistingFiles);
    srcdlg.setAcceptMode(QFileDialog::AcceptSave);
    srcdlg.setViewMode(QFileDialog::List);

    QStringList srcUris;
    srcdlg.exec();

    for (auto uri : srcdlg.selectedUrls()) {
        srcUris<<uri.toString();
    }
    Peony::FileDeleteOperation *deleteOp = new Peony::FileDeleteOperation(srcUris);
    QSignalSpy spyStart(deleteOp, &Peony::FileOperation::operationFinished);
    QSignalSpy spyError(deleteOp, &Peony::FileOperation::errored);
    deleteOp->connect(deleteOp, &Peony::FileOperation::errored,
                    this, &KTOperationTest::handleError,
                    Qt::BlockingQueuedConnection);
    // precondition
    QVERIFY(spyStart.isValid());
    QVERIFY(spyStart.isEmpty());
    QVERIFY(spyError.isValid());
    QVERIFY(spyError.isEmpty());

    const unsigned long int Period = 50;
    QVERIFY(QThreadPool::globalInstance()->tryStart(deleteOp));

    QVERIFY(spyStart.wait(Period * 10));
    QVERIFY(!spyError.wait(Period * 10));
    // postcondition
    const int startResult = spyStart.count();
    QCOMPARE(startResult, 1);
    QList<QVariant> theCall1 = spyStart.takeFirst();
    QVERIFY(theCall1.isEmpty());
    const int errorResult = spyError.count();
    QCOMPARE(errorResult, 0);
}

void KTOperationTest::testLibraryTrash()
{
    auto button = QMessageBox::question(nullptr, tr("trash files"), tr("choose trash files，"
                          "use cancel to finish the choices."));
    if (button != QMessageBox::Yes) {
      return;
    }
    QFileDialog srcdlg;
    srcdlg.setFileMode(QFileDialog::ExistingFiles);
    srcdlg.setAcceptMode(QFileDialog::AcceptSave);
    srcdlg.setViewMode(QFileDialog::List);

    QStringList srcUris;
    srcdlg.exec();

    for (auto uri : srcdlg.selectedUrls()) {
        srcUris<<uri.toString();
    }
    Peony::FileTrashOperation *trashOp = new Peony::FileTrashOperation(srcUris);
    QSignalSpy spyStart(trashOp, &Peony::FileOperation::operationFinished);
    QSignalSpy spyError(trashOp, &Peony::FileOperation::errored);
    trashOp->connect(trashOp, &Peony::FileOperation::errored,
                    this, &KTOperationTest::handleError,
                    Qt::BlockingQueuedConnection);
    // precondition
    QVERIFY(spyStart.isValid());
    QVERIFY(spyStart.isEmpty());
    QVERIFY(spyError.isValid());
    QVERIFY(spyError.isEmpty());

    const unsigned long int Period = 50;
    QVERIFY(QThreadPool::globalInstance()->tryStart(trashOp));

    QVERIFY(spyStart.wait(Period * 10));
    QVERIFY(!spyError.wait(Period * 10));
    // postcondition
    const int startResult = spyStart.count();
    QCOMPARE(startResult, 1);
    QList<QVariant> theCall1 = spyStart.takeFirst();
    QVERIFY(theCall1.isEmpty());
    const int errorResult = spyError.count();
    QCOMPARE(errorResult, 0);
}

void KTOperationTest::testLibraryLink()
{
    auto button = QMessageBox::question(nullptr, tr("link source file"), tr("choose link source file，"
                          "use cancel to finish the choices."));
    if (button != QMessageBox::Yes) {
      return;
    }
    QFileDialog srcdlg;
    srcdlg.setFileMode(QFileDialog::ExistingFiles);
    srcdlg.setAcceptMode(QFileDialog::AcceptSave);

    QString srcUri;
    QUrl srcgetDir = srcdlg.getSaveFileUrl();
    if (srcgetDir.isEmpty()) {
        return;
    }
    srcUri = srcgetDir.url();

    auto button1 = QMessageBox::question(nullptr, tr("link dest dir"), tr("choose link dest dir,"
                          "use cancel to finish the choices."));
    if (button1 != QMessageBox::Yes) {
        return;
    }
    QFileDialog destdlg;
    destdlg.setAcceptMode(QFileDialog::AcceptSave);
    destdlg.setFileMode(QFileDialog::Directory);

    QString destUri;
    QUrl targetDir = destdlg.getExistingDirectoryUrl();
    if (targetDir.isEmpty()) {
        return;
    }
    destUri = targetDir.url();

    Peony::FileLinkOperation *linkOp = new Peony::FileLinkOperation(srcUri, destUri);

    QSignalSpy spyStart(linkOp, &Peony::FileOperation::operationFinished);
    QSignalSpy spyError(linkOp, &Peony::FileOperation::errored);
    linkOp->connect(linkOp, &Peony::FileOperation::errored,
                    this, &KTOperationTest::handleError,
                    Qt::BlockingQueuedConnection);
    // precondition
    QVERIFY(spyStart.isValid());
    QVERIFY(spyStart.isEmpty());
    QVERIFY(spyError.isValid());
    QVERIFY(spyError.isEmpty());

    const unsigned long int Period = 50;
    QVERIFY(QThreadPool::globalInstance()->tryStart(linkOp));

    QVERIFY(spyStart.wait(Period * 10));
    QVERIFY(!spyError.wait(Period * 10));
    // postcondition
    const int startResult = spyStart.count();
    QCOMPARE(startResult, 1);
    QList<QVariant> theCall1 = spyStart.takeFirst();
    QVERIFY(theCall1.isEmpty());
    const int errorResult = spyError.count();
    QCOMPARE(errorResult, 0);
}

void KTOperationTest::testLibraryRename()
{
    auto button = QMessageBox::question(nullptr, tr("rename source file"), tr("choose rename source file，"
                          "use cancel to finish the choices."));
    if (button != QMessageBox::Yes) {
      return;
    }
    QFileDialog srcdlg;
    srcdlg.setFileMode(QFileDialog::ExistingFiles);
    srcdlg.setAcceptMode(QFileDialog::AcceptSave);

    QString srcUri;
    QUrl srcgetDir = srcdlg.getSaveFileUrl();
    if (srcgetDir.isEmpty()) {
        return;
    }
    srcUri = srcgetDir.url();
    Peony::FileRenameOperation *renameOp = new Peony::FileRenameOperation(srcUri, "TestName");

    QSignalSpy spyStart(renameOp, &Peony::FileOperation::operationFinished);
    QSignalSpy spyError(renameOp, &Peony::FileOperation::errored);
    renameOp->connect(renameOp, &Peony::FileOperation::errored,
                    this, &KTOperationTest::handleError,
                    Qt::BlockingQueuedConnection);
    // precondition
    QVERIFY(spyStart.isValid());
    QVERIFY(spyStart.isEmpty());
    QVERIFY(spyError.isValid());
    QVERIFY(spyError.isEmpty());

    const unsigned long int Period = 50;
    QVERIFY(QThreadPool::globalInstance()->tryStart(renameOp));

    QVERIFY(spyStart.wait(Period * 10));
    QVERIFY(!spyError.wait(Period * 10));
    // postcondition
    const int startResult = spyStart.count();
    QCOMPARE(startResult, 1);
    QList<QVariant> theCall1 = spyStart.takeFirst();
    QVERIFY(theCall1.isEmpty());
    const int errorResult = spyError.count();
    QByteArray sig = spyError.signal();
    Peony::FileOperationError *pError = (Peony::FileOperationError*)sig.data();
    QCOMPARE(errorResult, 0);
}

void KTOperationTest::testLibraryUntrash()
{
    Peony::FileEnumerator e;
    e.setEnumerateDirectory("trash:///");
    e.enumerateSync();
    auto infos = e.getChildren();
    QStringList uris;
    for (auto info : infos) {
        uris<<info->uri();
    }
    Peony::FileUntrashOperation *untrashOp = new Peony::FileUntrashOperation(uris);
    QSignalSpy spyStart(untrashOp, &Peony::FileOperation::operationFinished);
    QSignalSpy spyError(untrashOp, &Peony::FileOperation::errored);
    untrashOp->connect(untrashOp, &Peony::FileOperation::errored,
                    this, &KTOperationTest::handleError,
                    Qt::BlockingQueuedConnection);
    // precondition
    QVERIFY(spyStart.isValid());
    QVERIFY(spyStart.isEmpty());
    QVERIFY(spyError.isValid());
    QVERIFY(spyError.isEmpty());

    const unsigned long int Period = 50;
    QVERIFY(QThreadPool::globalInstance()->tryStart(untrashOp));

    QVERIFY(spyStart.wait(Period * 10));
    QVERIFY(!spyError.wait(Period * 10));
    // postcondition
    const int startResult = spyStart.count();
    QCOMPARE(startResult, 1);
    QList<QVariant> theCall1 = spyStart.takeFirst();
    QVERIFY(theCall1.isEmpty());
    const int errorResult = spyError.count();
    QCOMPARE(errorResult, 0);
}

void KTOperationTest::handleError(Peony::FileOperationError& error)
{
    if (error.srcUri.startsWith("trash://")
        && Peony::FileOpDelete == error.op)
    {
        error.respCode = Peony::IgnoreAll;
        return;
    }

    Peony::FileOperationErrorHandler* handle = Peony::FileOperationErrorDialogFactory::getDialog(error);
    handle->handle(error);
    qInfo() << error.respCode;
}
