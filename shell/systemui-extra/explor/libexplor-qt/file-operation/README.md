# FileOperation

## Brief
FileOperation is an QRunnable based interface of several file operation. Such as move, copy, delete, rename, and so on.

## Desgin Concept
- Every operation is atomic. That means if you cancelled the operation, it should rollback to the previous states when you didn't execute the operation.
- The operation can not handle error itself. When a error ocurred, it will send the signal for an error handler instance, and block itself until the handler's slot return the response type. The simplest handler should ignore all the error, even though the operation might not success.
-  The error handler is variable. It could have a UI, or just running in backend.
-  There should be a operation manager to manager the operations' stack for operation undo or redo.

## Implement example -- FileMoveOperation
FileMoveOperatoin is dervied from FileOperation. It is an synchronous and cancellable operation. There are two types internal move operation provieded by the class, the native move and the fallback move.

The native move is very fast, but it might not be support in many cases, such as move from different computer. The fallback move is actually a copy and delete operation of files. It might spend a lot of times for a large file directory's movement.

## FileOperationProgressWizard
FileOperationProgressWizard is used to indicate the progress of a FileOperation instance. Cause the operation running in thread, wizard should connect the providing signal of FileOperation and then it will handle the sending signal internally.