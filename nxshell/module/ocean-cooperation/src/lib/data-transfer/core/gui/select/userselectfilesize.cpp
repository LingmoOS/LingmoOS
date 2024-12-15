#include "userselectfilesize.h"
#include "calculatefilesize.h"
#include <QString>
#include <QDebug>
#include <QModelIndex>
#include <QListView>
#include <QStandardItemModel>

UserSelectFileSize::UserSelectFileSize()
{
    // Update the size of the selected files to be computed.
    QObject::connect(CalculateFileSizeThreadPool::instance(),
                     &CalculateFileSizeThreadPool::sendFileSizeSignal, this,
                     &UserSelectFileSize::updatependingFileSize);
}

void UserSelectFileSize::sendFileSize()
{
    if (pendingFiles.isEmpty()) {
        emit updateUserFileSelectSize(fromByteToQstring(userSelectFileSize));
    } else {
        emit updateUserFileSelectSize(QString(tr("Calculating")));
    }
}

UserSelectFileSize::~UserSelectFileSize() { }

UserSelectFileSize *UserSelectFileSize::instance()
{
    static UserSelectFileSize ins;
    return &ins;
}

bool UserSelectFileSize::done()
{
    return pendingFiles.empty();
}

bool UserSelectFileSize::isPendingFile(const QString &path)
{
    return pendingFiles.contains(path);
}

void UserSelectFileSize::addPendingFiles(const QString &path)
{
    pendingFiles.push_back(path);
}

void UserSelectFileSize::delPendingFiles(const QString &path)
{
    if (pendingFiles.contains(path))
        pendingFiles.removeOne(path);
}

void UserSelectFileSize::addSelectFiles(const QString &path)
{
    selectFiles.push_back(path);
    emit updateUserFileSelectNum(path, true);
}

void UserSelectFileSize::delSelectFiles(const QString &path)
{
    if (selectFiles.contains(path))
        selectFiles.removeOne(path);
    emit updateUserFileSelectNum(path, false);
}

void UserSelectFileSize::addUserSelectFileSize(quint64 filesize)
{
    userSelectFileSize += filesize;
    sendFileSize();
}

void UserSelectFileSize::delUserSelectFileSize(quint64 filesize)
{
    userSelectFileSize -= filesize;
    sendFileSize();
}

quint64 UserSelectFileSize::getAllSelectSize()
{
    return userSelectFileSize;
}

QStringList UserSelectFileSize::getSelectFilesList()
{
    return selectFiles;
}

void UserSelectFileSize::updatependingFileSize(const quint64 &size, const QString &path)
{
    if (pendingFiles.contains(path)) {
        userSelectFileSize += size;
        pendingFiles.removeOne(path);
        sendFileSize();
    }
}

void UserSelectFileSize::delDevice(QStandardItem *siderbarItem)
{
    QMap<QString, FileInfo> *filemap = CalculateFileSizeThreadPool::instance()->getFileMap();
    QStringList::iterator it = selectFiles.begin();
    while (it != selectFiles.end()) {
        if (filemap->value(*it).siderbarItem == siderbarItem) {
            if (filemap->value(*it).isCalculate) {
                userSelectFileSize -= filemap->value(*it).size;
            }
            it = selectFiles.erase(it);
        } else {
            ++it;
        }
    }
    sendFileSize();
}

void UserSelectFileSize::updateFileSelectList(QStandardItem *item)
{
    QString path = item->data(Qt::UserRole).toString();
    QMap<QString, FileInfo> *filemap = CalculateFileSizeThreadPool::instance()->getFileMap();
    if (item->data(Qt::CheckStateRole) == Qt::Unchecked) {
        if ((*filemap)[path].isSelect == false) {
            return;
        }
        // do not select the file
        (*filemap)[path].isSelect = false;
        delSelectFiles(path);
        if ((*filemap)[path].isCalculate) {
            quint64 size = (*filemap)[path].size;
            delUserSelectFileSize(size);
        } else {
            delPendingFiles(path);
        }
    } else if (item->data(Qt::CheckStateRole) == Qt::Checked) {
        if ((*filemap)[path].isSelect == true) {
            return;
        }
        (*filemap)[path].isSelect = true;
        addSelectFiles(path);
        if ((*filemap)[path].isCalculate) {
            quint64 size = (*filemap)[path].size;
            addUserSelectFileSize(size);
        } else {
            addPendingFiles(path);
        }
    }
}
