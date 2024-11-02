#ifndef SCOREINFOMODEL_H
#define SCOREINFOMODEL_H

#include <QObject>
#include <QStandardItem>
#include <QTableView>
#include <QList>
#include <QDebug>
#include "UIControl/base/musicDataBase.h"

class MusicListModel : public QObject
{
    Q_OBJECT
public:
    explicit MusicListModel(QObject *parent = 0);
    bool add(musicDataStruct info);
    bool add(QList<musicDataStruct> list);
    bool remove(int i);
    int count();
    void clear();
    void setView(QTableView &view);
    musicDataStruct getItem(int i);
    QStringList getPathList(QString listName);
    void getMusicDateList(QList<musicDataStruct> list);
    QStandardItemModel m_model; // 数据模型，不能单独显示出来
    QStringList titleList;
    QStandardItem* item[4];
private:
    QList<musicDataStruct> resList;

};

#endif // SCOREINFOMODEL_H
