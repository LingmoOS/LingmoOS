#ifndef KT_OPERATION_TEST_H
#define KT_OPERATION_TEST_H

#include <QMainWindow>
#include <QSignalSpy>
#include "explorer-qt/file-move-operation.h"
#include "explorer-qt/file-copy-operation.h"
#include "explorer-qt/file-delete-operation.h"
#include "explorer-qt/file-rename-operation.h"
#include "explorer-qt/file-trash-operation.h"
#include "explorer-qt/file-link-operation.h"
#include "explorer-qt/file-untrash-operation.h"
#include "explorer-qt/file-operation-error-handler.h"

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
