#include "comboxlistmodel.h"
#include <QFile>
#include <QDebug>

namespace KServer {

ItemLanguageList ReadLanguageFile()
{
    ItemLanguageList list;
    QFile file(":/res/file/language");
    if(file.open(QFile::ReadOnly)) {
        QTextStream filetext(&file);
        QStringList flist = filetext.readAll().split('\n');
        for(int j = 0; j < flist.length() - 1; j++) {
            QStringList str = flist.at(j).split(':');
            ItemLanguage item;
            item.itemname = str.at(1);
            item.systemlocal = str.at(0);
            item.country = str.at(2);
            list.append(item);
        }
    } else {
        qCritical() << "fail to open the file";
    }
    return (ItemLanguageList)list;
}

ComboxListModel::ComboxListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_languageList = ReadLanguageFile();
}

ComboxListModel::~ComboxListModel()
{

}

int ComboxListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (!parent.isValid())
        return m_languageList.length();

}

QVariant ComboxListModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole) {
        return QVariant();
    }

    if (!index.isValid()) {
        return QVariant();
    } else {
        return m_languageList.at(index.row()).systemlocal;
    }
}

ItemLanguage ComboxListModel::getItemLanguage(const QModelIndex &index)
{
    if(index.isValid()) {
        return m_languageList.at(index.row());
    } else {
        return ItemLanguage();
    }
}

QModelIndex ComboxListModel::getIndex(const QString &itemname)
{
    for(int i = 0; i < m_languageList.length(); i++) {
        if(m_languageList.at(i).itemname == itemname) {
            return index(i);
        }
    }
    return QModelIndex();
}
}
