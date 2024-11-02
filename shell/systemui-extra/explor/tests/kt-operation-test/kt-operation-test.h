#ifndef KT_OPERATION_TEST_H
#define KT_OPERATION_TEST_H

#include <QMainWindow>
#include <QSignalSpy>
#include "explor-qt/file-move-operation.h"
#include "explor-qt/file-copy-operation.h"
#include "explor-qt/file-delete-operation.h"
#include "explor-qt/file-rename-operation.h"
#include "explor-qt/file-trash-operation.h"
#include "explor-qt/file-link-operation.h"
#include "explor-qt/file-untrash-operation.h"
#include "explor-qt/file-operation-error-handler.h"

class KTOperationTest : public QMainWindow
{
    Q_OBJECT

private Q_SLOTS:
    void testLibrary();
private:
    void testLibraryCopy();
    void testLibraryMove();
    void testLibraryDelete();
    void testLibraryLink();
    void testLibraryTrash();
    void testLibraryRename();
    void testLibraryUntrash();
    void handleError(Peony::FileOperationError& error);
};
#endif // KT_OPERATION_TEST_H
