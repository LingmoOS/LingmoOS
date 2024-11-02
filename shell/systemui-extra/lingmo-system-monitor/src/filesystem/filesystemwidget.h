#ifndef FILESYSTEMWIDGET_H
#define FILESYSTEMWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMap>
#include <QMenu>
#include <QLabel>
#include <QPaintEvent>

#include "filesystemworker.h"
#include "filesystemdata.h"
#include "filesystemwatcher.h"

class FileSystemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FileSystemWidget(QWidget *parent = nullptr);
    virtual ~FileSystemWidget();

public slots:
    void onUpdateInfo();
    void onSearch(QString searchKey);
    void onItemDblClicked(QString strMountUrl);

protected:
    void mousePressEvent(QMouseEvent *event);
    void hideEvent(QHideEvent *event);
    void resizeEvent(QResizeEvent *) override;

private:
    void initUI();
    void initConnections();
    void addInfoItems(FileSystemData& fsData);
    void clearInfoItems();

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QFrame *m_listFrame = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QScrollArea *m_scrollFrame = nullptr;
    QLabel *m_notFoundTextLabel = nullptr;
    QLabel *m_notFoundPixLabel = nullptr;

    QMenu *m_contextMenu = nullptr;

    QMap<QString, QWidget*> m_mapInfoItems;
    QMap<QString, FileSystemData> m_mapFileSystemData; // system data list
    FileSystemWorker *m_fileSystemWorker;    // list provider
    FileSystemWatcher *m_fileSystemWatcher;  // monitor the filesystem mount or unmount

    QString m_strSearchKey;
};

#endif // FILESYSTEMWIDGET_H
