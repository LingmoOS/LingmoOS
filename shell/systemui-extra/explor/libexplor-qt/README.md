# explor-qt-core
Core part of Peony, providing a set of APIs for Peony and other projects for file manager development.

# description
Peony is the filemanager of lingmo3.0，which refactoring Peony(LINGMO 2.0) based on qt and glib/gio.

In simple terms, this project can be seem as the qt/c++ style wrapper of gio's api. It should provide all kinds of file operation that gio has supported, such as querying, enumerating, monitoring copying/moving, mounting, etc... And it will translate g_signal to qt's signals for using signal/slot between QObject based classes.

## designing concept
- 1. File and its info is shared in global scale.
- 2. Try to use asynchronous methods.
- 3. Integrate exception handling.
- 4. Detach front and backend as detached as possible.

## content
### FileInfo and FileInfoManager
In Peony, a file can be abstracted to a FileInfo class. It contains some informations about the file, such as uri, icon, size, etc. When the FileInfo was created, it should be refered by FileInfoManager. So that the file system will be mapped into a string-info map.
The FileInfoManager is desgined to a static single global instance, and the FileInfo instances are usually wrapped by a std::shared_ptr.
The FileInfo instance, getting from FileInfo::fromUri().
Actually, FileInfo in FileInfoManager only has a weak reference and it will be destoryed by the last third class instance deconstruction which hold this smart pointer.

The FileInfo instance could be 'incomplete' and 'virtual'. It means the real file might not exsited. When requiring a FileInfo instance, the instance aslo have not gotten all details of the file. You should use FileInfoJob for a querying at first.

### FileEnumerator
FileEnumerator is a 'holder' class of FileInfo instances. It provieds the file enumeration function of both local and virtual filesystem.
The enumeration is divided into two types, the synchronous one and the asynchronous one. asynchronous enueration need to connect the signal to monitor the enumeration states. Once enumeration finished, you can get the children of the target directoy, which is aslo abstract to the FileInfo instances. This instances are aslo incomplete and need be handled.

### FileWatcher
FileWatcher is desgined to monitor the directory changing. When there some changes ocurred, it will send the signal by the file change type.

### FileItem and models of FileItem
FileItem is a more advanced layer abstraction of FileInfo. This class is the interface of Qt's Model/View programming.
Through the FileItem, the file's info could be shown into a model-based view.

By this way, It could aslo used the Qt's ProxySortFilterModel for the sorting and filtering.

- see [Model](model/README.md)

### FileOperation
FileOperation is relatively independent from previous classes. All file operations should not run in ui thread. It dervied from QRunnable and should be executed by QThreadPool global instance.

- see [FileOperation](file-operation/README.md)

# License
- LGPLv3
