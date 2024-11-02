#ifndef FILEDIALOGINTERFACE_H
#define FILEDIALOGINTERFACE_H
#include "qpa/qplatformdialoghelper.h"
//定义接口
class FileDialogInterface
{
public:
    virtual ~FileDialogInterface() {}
    virtual QPlatformDialogHelper* create() = 0;
};

//一定是唯一的标识符
#define FileDialogInterface_iid "LINGMOPlatformTheme.Plugin.FileDialogInterface"

Q_DECLARE_INTERFACE(FileDialogInterface, FileDialogInterface_iid)

#endif // FILEDIALOGINTERFACE_H
