#ifndef USERSELECTFILESIZE_H
#define USERSELECTFILESIZE_H

#include <QObject>
#include <QMultiMap>
class QString;
class QListView;
class QModelIndex;
class QStandardItem;
class UserSelectFileSize : public QObject
{
    Q_OBJECT
public:
    ~UserSelectFileSize();
    static UserSelectFileSize *instance();
    bool done();
    bool isPendingFile(const QString &path);
    void addPendingFiles(const QString &path);
    void delPendingFiles(const QString &path);
    void addSelectFiles(const QString &path);
    void delSelectFiles(const QString &path);

    void addUserSelectFileSize(quint64 filesize);
    void delUserSelectFileSize(quint64 filesize);

    void clearAllFileSelect();
    quint64 getAllSelectSize();
    QStringList getSelectFilesList();

    void delDevice(QStandardItem *siderbarItem);
signals:
    void updateUserFileSelectSize(const QString &size);
    void updateUserFileSelectNum(const QString &path, const bool &isAdd);
public slots:
    void updatependingFileSize(const quint64 &size, const QString &path);
    void updateFileSelectList(QStandardItem *item);

private:
    UserSelectFileSize();
    void sendFileSize();

private:
    QStringList pendingFiles;

    QStringList selectFiles;

    quint64 userSelectFileSize{ 0 };
};

#endif // USERSELECTFILESIZE_H
